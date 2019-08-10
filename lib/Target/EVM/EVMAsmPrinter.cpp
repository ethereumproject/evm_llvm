//===-- EVMAsmPrinter.cpp - EVM LLVM assembly writer ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the EVM assembly language.
//
//===----------------------------------------------------------------------===//

#include "EVM.h"
#include "EVMMCInstLower.h"
#include "InstPrinter/EVMInstPrinter.h"
#include "EVMTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

#include "llvm/MC/MCInstBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "evm-asm-printer"

namespace {
class EVMAsmPrinter : public AsmPrinter {
public:
  explicit EVMAsmPrinter(TargetMachine &TM,
                           std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  void EmitStartOfAsmFile(Module &M) override;
  void EmitEndOfAsmFile(Module &M) override;

  StringRef getPassName() const override { return "EVM Assembly Printer"; }

  void EmitInstruction(const MachineInstr *MI) override;

  void printOperand(const MachineInstr *MI, unsigned OpNo, raw_ostream &OS);

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

  // void EmitToStreamer(MCStreamer &S, const MCInst &Inst);
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);

  // Wrapper needed for tblgenned pseudo lowering.
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const {
    return LowerEVMMachineOperandToMCOperand(MO, MCOp, *this);
    }

  void emitInitFreeMemoryPointer() const;
  void emitNonpayableCheck() const;
  void emitRetrieveConstructorParameters() const;
  void emitConstructorBody() const;
  void emitCopyRuntimeCodeToMemory() const;
  void emitReturnRuntimeCode() const;
};
}

void EVMAsmPrinter::emitInitFreeMemoryPointer() const {
    auto &STI = getSubtargetInfo();
    // Initialize the Free memory pointer
    OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x80), STI);
    OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x40), STI);
    OutStreamer->EmitInstruction(MCInstBuilder(EVM::MSTORE), STI);
}

void EVMAsmPrinter::emitNonpayableCheck() const {
  auto &STI = getSubtargetInfo();

  // Non-payable check
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::CALLVALUE), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::ISZERO), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH2).addImm(0x0010), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::JUMPI), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x00), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::REVERT), STI);
}

void EVMAsmPrinter::emitRetrieveConstructorParameters() const {
  auto &STI = getSubtargetInfo();
  /*
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::JUMPDEST), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::POP), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x40), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::MLOAD), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x20), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH2).addImm(0x0217), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP4), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::CODECOPY), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP2), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::ADD), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x40), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::SWAP1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP2), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::MSTORE), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::SWAP1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::MLOAD), STI);
  */
}

void EVMAsmPrinter::emitConstructorBody() const {
  auto &STI = getSubtargetInfo();

  // TODO: fill in Constructor Body
}

void EVMAsmPrinter::emitCopyRuntimeCodeToMemory() const {
  auto &STI = getSubtargetInfo();

  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH2).addImm(0x01d1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::DUP1), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH2).addImm(0x0046), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x00), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::CODECOPY), STI);
}

void EVMAsmPrinter::emitReturnRuntimeCode() const {
  auto &STI = getSubtargetInfo();

  OutStreamer->EmitInstruction(MCInstBuilder(EVM::PUSH1).addImm(0x00), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::RETURN), STI);
  OutStreamer->EmitInstruction(MCInstBuilder(EVM::STOP), STI);
}

void EVMAsmPrinter::EmitStartOfAsmFile(Module &M) {
  // Emit the skeleton of the asm file in text section.
  OutStreamer->SwitchSection(getObjFileLowering().getTextSection());

  emitInitFreeMemoryPointer();

  emitNonpayableCheck();
  
  emitRetrieveConstructorParameters();

  emitConstructorBody();

  emitCopyRuntimeCodeToMemory();

  emitReturnRuntimeCode();

}

void EVMAsmPrinter::EmitEndOfAsmFile(Module &M) {
  // Emit the skeleton of the asm file in text section.
  OutStreamer->SwitchSection(getObjFileLowering().getTextSection());

  // construct the skeleton framework
  auto &STI = getSubtargetInfo();

}

void EVMAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  EVMMCInstLower MCInstLowering(OutContext, *this);
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

void EVMAsmPrinter::printOperand(const MachineInstr *MI, unsigned OpNo,
                                 raw_ostream &OS) {
  const MachineOperand &MO = MI->getOperand(OpNo);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    break;
  case MachineOperand::MO_Immediate:
    OS << MO.getImm();
    break;
  case MachineOperand::MO_CImmediate:
    OS << MO.getCImm();
    break;
  case MachineOperand::MO_MachineBasicBlock:
    OS << *MO.getMBB()->getSymbol();
    break;
  default:
    llvm_unreachable("Not implemented yet!");
  }
}

bool EVMAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                    const char *ExtraCode, raw_ostream &OS) {
  printOperand(MI, OpNo, OS);
  return true;
}

bool EVMAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                            unsigned OpNo,
                                            const char *ExtraCode,
                                            raw_ostream &OS) {

  return AsmPrinter::PrintAsmMemoryOperand(MI, OpNo, ExtraCode, OS);
}

// Force static initialization.
extern "C" void LLVMInitializeEVMAsmPrinter() {
  RegisterAsmPrinter<EVMAsmPrinter> Y(getTheEVMTarget());
}
