//===- ARMInstructionSelector.cpp ----------------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the InstructionSelector class for ARM.
/// \todo This should be generated by TableGen.
//===----------------------------------------------------------------------===//

#include "ARMInstructionSelector.h"
#include "ARMRegisterBankInfo.h"
#include "ARMSubtarget.h"
#include "ARMTargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "arm-isel"

using namespace llvm;

#ifndef LLVM_BUILD_GLOBAL_ISEL
#error "You shouldn't build this"
#endif

ARMInstructionSelector::ARMInstructionSelector(const ARMSubtarget &STI,
                                               const ARMRegisterBankInfo &RBI)
    : InstructionSelector(), TII(*STI.getInstrInfo()),
      TRI(*STI.getRegisterInfo()), RBI(RBI) {}

static bool selectCopy(MachineInstr &I, const TargetInstrInfo &TII,
                       MachineRegisterInfo &MRI, const TargetRegisterInfo &TRI,
                       const RegisterBankInfo &RBI) {
  unsigned DstReg = I.getOperand(0).getReg();
  if (TargetRegisterInfo::isPhysicalRegister(DstReg))
    return true;

  const RegisterBank *RegBank = RBI.getRegBank(DstReg, MRI, TRI);
  (void)RegBank;
  assert(RegBank && "Can't get reg bank for virtual register");

  const unsigned DstSize = MRI.getType(DstReg).getSizeInBits();
  (void)DstSize;
  unsigned SrcReg = I.getOperand(1).getReg();
  const unsigned SrcSize = RBI.getSizeInBits(SrcReg, MRI, TRI);
  (void)SrcSize;
  assert((DstSize == SrcSize ||
          // Copies are a means to setup initial types, the number of
          // bits may not exactly match.
          (TargetRegisterInfo::isPhysicalRegister(SrcReg) &&
           DstSize <= SrcSize)) &&
         "Copy with different width?!");

  assert((RegBank->getID() == ARM::GPRRegBankID ||
          RegBank->getID() == ARM::FPRRegBankID) &&
         "Unsupported reg bank");

  const TargetRegisterClass *RC = &ARM::GPRRegClass;

  if (RegBank->getID() == ARM::FPRRegBankID) {
    if (DstSize == 32)
      RC = &ARM::SPRRegClass;
    else if (DstSize == 64)
      RC = &ARM::DPRRegClass;
    else
      llvm_unreachable("Unsupported destination size");
  }

  // No need to constrain SrcReg. It will get constrained when
  // we hit another of its uses or its defs.
  // Copies do not have constraints.
  if (!RBI.constrainGenericRegister(DstReg, *RC, MRI)) {
    DEBUG(dbgs() << "Failed to constrain " << TII.getName(I.getOpcode())
                 << " operand\n");
    return false;
  }
  return true;
}

static bool selectFAdd(MachineInstrBuilder &MIB, const ARMBaseInstrInfo &TII,
                       MachineRegisterInfo &MRI) {
  assert(TII.getSubtarget().hasVFP2() && "Can't select fp add without vfp");

  LLT Ty = MRI.getType(MIB->getOperand(0).getReg());
  unsigned ValSize = Ty.getSizeInBits();

  if (ValSize == 32) {
    if (TII.getSubtarget().useNEONForSinglePrecisionFP())
      return false;
    MIB->setDesc(TII.get(ARM::VADDS));
  } else {
    assert(ValSize == 64 && "Unsupported size for floating point value");
    if (TII.getSubtarget().isFPOnlySP())
      return false;
    MIB->setDesc(TII.get(ARM::VADDD));
  }
  MIB.add(predOps(ARMCC::AL));

  return true;
}

static bool selectSequence(MachineInstrBuilder &MIB,
                           const ARMBaseInstrInfo &TII,
                           MachineRegisterInfo &MRI,
                           const TargetRegisterInfo &TRI,
                           const RegisterBankInfo &RBI) {
  assert(TII.getSubtarget().hasVFP2() && "Can't select sequence without VFP");

  // We only support G_SEQUENCE as a way to stick together two scalar GPRs
  // into one DPR.
  unsigned VReg0 = MIB->getOperand(0).getReg();
  (void)VReg0;
  assert(MRI.getType(VReg0).getSizeInBits() == 64 &&
         RBI.getRegBank(VReg0, MRI, TRI)->getID() == ARM::FPRRegBankID &&
         "Unsupported operand for G_SEQUENCE");
  unsigned VReg1 = MIB->getOperand(1).getReg();
  (void)VReg1;
  assert(MRI.getType(VReg1).getSizeInBits() == 32 &&
         RBI.getRegBank(VReg1, MRI, TRI)->getID() == ARM::GPRRegBankID &&
         "Unsupported operand for G_SEQUENCE");
  unsigned VReg2 = MIB->getOperand(3).getReg();
  (void)VReg2;
  assert(MRI.getType(VReg2).getSizeInBits() == 32 &&
         RBI.getRegBank(VReg2, MRI, TRI)->getID() == ARM::GPRRegBankID &&
         "Unsupported operand for G_SEQUENCE");

  // Remove the operands corresponding to the offsets.
  MIB->RemoveOperand(4);
  MIB->RemoveOperand(2);

  MIB->setDesc(TII.get(ARM::VMOVDRR));
  MIB.add(predOps(ARMCC::AL));

  return true;
}

static bool selectExtract(MachineInstrBuilder &MIB, const ARMBaseInstrInfo &TII,
                          MachineRegisterInfo &MRI,
                          const TargetRegisterInfo &TRI,
                          const RegisterBankInfo &RBI) {
  assert(TII.getSubtarget().hasVFP2() && "Can't select extract without VFP");

  // We only support G_EXTRACT as a way to break up one DPR into two GPRs.
  unsigned VReg0 = MIB->getOperand(0).getReg();
  (void)VReg0;
  assert(MRI.getType(VReg0).getSizeInBits() == 32 &&
         RBI.getRegBank(VReg0, MRI, TRI)->getID() == ARM::GPRRegBankID &&
         "Unsupported operand for G_EXTRACT");
  unsigned VReg1 = MIB->getOperand(1).getReg();
  (void)VReg1;
  assert(MRI.getType(VReg1).getSizeInBits() == 64 &&
         RBI.getRegBank(VReg1, MRI, TRI)->getID() == ARM::FPRRegBankID &&
         "Unsupported operand for G_EXTRACT");
  assert(MIB->getOperand(2).getImm() % 32 == 0 &&
         "Unsupported operand for G_EXTRACT");

  // Remove the operands corresponding to the offsets.
  MIB->getOperand(2).setImm(MIB->getOperand(2).getImm() / 32);

  MIB->setDesc(TII.get(ARM::VGETLNi32));
  MIB.add(predOps(ARMCC::AL));

  return true;
}

/// Select the opcode for simple extensions (that translate to a single SXT/UXT
/// instruction). Extension operations more complicated than that should not
/// invoke this. Returns the original opcode if it doesn't know how to select a
/// better one.
static unsigned selectSimpleExtOpc(unsigned Opc, unsigned Size) {
  using namespace TargetOpcode;

  if (Size != 8 && Size != 16)
    return Opc;

  if (Opc == G_SEXT)
    return Size == 8 ? ARM::SXTB : ARM::SXTH;

  if (Opc == G_ZEXT)
    return Size == 8 ? ARM::UXTB : ARM::UXTH;

  return Opc;
}

/// Select the opcode for simple loads and stores. For types smaller than 32
/// bits, the value will be zero extended. Returns the original opcode if it
/// doesn't know how to select a better one.
static unsigned selectLoadStoreOpCode(unsigned Opc, unsigned RegBank,
                                      unsigned Size) {
  bool isStore = Opc == TargetOpcode::G_STORE;

  if (RegBank == ARM::GPRRegBankID) {
    switch (Size) {
    case 1:
    case 8:
      return isStore ? ARM::STRBi12 : ARM::LDRBi12;
    case 16:
      return isStore ? ARM::STRH : ARM::LDRH;
    case 32:
      return isStore ? ARM::STRi12 : ARM::LDRi12;
    default:
      return Opc;
    }
  }

  if (RegBank == ARM::FPRRegBankID) {
    switch (Size) {
    case 32:
      return isStore ? ARM::VSTRS : ARM::VLDRS;
    case 64:
      return isStore ? ARM::VSTRD : ARM::VLDRD;
    default:
      return Opc;
    }
  }

  return Opc;
}

bool ARMInstructionSelector::select(MachineInstr &I) const {
  assert(I.getParent() && "Instruction should be in a basic block!");
  assert(I.getParent()->getParent() && "Instruction should be in a function!");

  auto &MBB = *I.getParent();
  auto &MF = *MBB.getParent();
  auto &MRI = MF.getRegInfo();

  if (!isPreISelGenericOpcode(I.getOpcode())) {
    if (I.isCopy())
      return selectCopy(I, TII, MRI, TRI, RBI);

    return true;
  }

  MachineInstrBuilder MIB{MF, I};
  bool isSExt = false;

  using namespace TargetOpcode;
  switch (I.getOpcode()) {
  case G_SEXT:
    isSExt = true;
    LLVM_FALLTHROUGH;
  case G_ZEXT: {
    LLT DstTy = MRI.getType(I.getOperand(0).getReg());
    // FIXME: Smaller destination sizes coming soon!
    if (DstTy.getSizeInBits() != 32) {
      DEBUG(dbgs() << "Unsupported destination size for extension");
      return false;
    }

    LLT SrcTy = MRI.getType(I.getOperand(1).getReg());
    unsigned SrcSize = SrcTy.getSizeInBits();
    switch (SrcSize) {
    case 1: {
      // ZExt boils down to & 0x1; for SExt we also subtract that from 0
      I.setDesc(TII.get(ARM::ANDri));
      MIB.addImm(1).add(predOps(ARMCC::AL)).add(condCodeOp());

      if (isSExt) {
        unsigned SExtResult = I.getOperand(0).getReg();

        // Use a new virtual register for the result of the AND
        unsigned AndResult = MRI.createVirtualRegister(&ARM::GPRRegClass);
        I.getOperand(0).setReg(AndResult);

        auto InsertBefore = std::next(I.getIterator());
        auto SubI =
            BuildMI(MBB, InsertBefore, I.getDebugLoc(), TII.get(ARM::RSBri))
                .addDef(SExtResult)
                .addUse(AndResult)
                .addImm(0)
                .add(predOps(ARMCC::AL))
                .add(condCodeOp());
        if (!constrainSelectedInstRegOperands(*SubI, TII, TRI, RBI))
          return false;
      }
      break;
    }
    case 8:
    case 16: {
      unsigned NewOpc = selectSimpleExtOpc(I.getOpcode(), SrcSize);
      if (NewOpc == I.getOpcode())
        return false;
      I.setDesc(TII.get(NewOpc));
      MIB.addImm(0).add(predOps(ARMCC::AL));
      break;
    }
    default:
      DEBUG(dbgs() << "Unsupported source size for extension");
      return false;
    }
    break;
  }
  case G_ADD:
  case G_GEP:
    I.setDesc(TII.get(ARM::ADDrr));
    MIB.add(predOps(ARMCC::AL)).add(condCodeOp());
    break;
  case G_SUB:
    I.setDesc(TII.get(ARM::SUBrr));
    MIB.add(predOps(ARMCC::AL)).add(condCodeOp());
    break;
  case G_MUL:
    if (TII.getSubtarget().hasV6Ops()) {
      I.setDesc(TII.get(ARM::MUL));
    } else {
      assert(TII.getSubtarget().useMulOps() && "Unsupported target");
      I.setDesc(TII.get(ARM::MULv5));
      MIB->getOperand(0).setIsEarlyClobber(true);
    }
    MIB.add(predOps(ARMCC::AL)).add(condCodeOp());
    break;
  case G_FADD:
    if (!selectFAdd(MIB, TII, MRI))
      return false;
    break;
  case G_FRAME_INDEX:
    // Add 0 to the given frame index and hope it will eventually be folded into
    // the user(s).
    I.setDesc(TII.get(ARM::ADDri));
    MIB.addImm(0).add(predOps(ARMCC::AL)).add(condCodeOp());
    break;
  case G_CONSTANT: {
    unsigned Reg = I.getOperand(0).getReg();
    if (MRI.getType(Reg).getSizeInBits() != 32)
      return false;

    assert(RBI.getRegBank(Reg, MRI, TRI)->getID() == ARM::GPRRegBankID &&
           "Expected constant to live in a GPR");
    I.setDesc(TII.get(ARM::MOVi));
    MIB.add(predOps(ARMCC::AL)).add(condCodeOp());
    break;
  }
  case G_STORE:
  case G_LOAD: {
    const auto &MemOp = **I.memoperands_begin();
    if (MemOp.getOrdering() != AtomicOrdering::NotAtomic) {
      DEBUG(dbgs() << "Atomic load/store not supported yet\n");
      return false;
    }

    unsigned Reg = I.getOperand(0).getReg();
    unsigned RegBank = RBI.getRegBank(Reg, MRI, TRI)->getID();

    LLT ValTy = MRI.getType(Reg);
    const auto ValSize = ValTy.getSizeInBits();

    assert((ValSize != 64 || TII.getSubtarget().hasVFP2()) &&
           "Don't know how to load/store 64-bit value without VFP");

    const auto NewOpc = selectLoadStoreOpCode(I.getOpcode(), RegBank, ValSize);
    if (NewOpc == G_LOAD || NewOpc == G_STORE)
      return false;

    I.setDesc(TII.get(NewOpc));

    if (NewOpc == ARM::LDRH || NewOpc == ARM::STRH)
      // LDRH has a funny addressing mode (there's already a FIXME for it).
      MIB.addReg(0);
    MIB.addImm(0).add(predOps(ARMCC::AL));
    break;
  }
  case G_SEQUENCE: {
    if (!selectSequence(MIB, TII, MRI, TRI, RBI))
      return false;
    break;
  }
  case G_EXTRACT: {
    if (!selectExtract(MIB, TII, MRI, TRI, RBI))
      return false;
    break;
  }
  default:
    return false;
  }

  return constrainSelectedInstRegOperands(I, TII, TRI, RBI);
}
