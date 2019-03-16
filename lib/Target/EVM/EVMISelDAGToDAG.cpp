//===-- EVMISelDAGToDAG.cpp - A dag to dag inst selector for EVM ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the EVM target.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/EVMMCTargetDesc.h"
#include "EVM.h"
#include "EVMTargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "evm-isel"

// EVM-specific code to select EVM machine instructions for
// SelectionDAG operations.
namespace {
class EVMDAGToDAGISel final : public SelectionDAGISel {
  const EVMSubtarget *Subtarget;

public:
  explicit EVMDAGToDAGISel(EVMTargetMachine &TargetMachine)
      : SelectionDAGISel(TargetMachine) {}

  StringRef getPassName() const override {
    return "EVM DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<EVMSubtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  void PostprocessISelDAG() override;

  void Select(SDNode *Node) override;

  bool SelectAddrFI(SDValue Addr, SDValue &Base, SDValue &Offset);

  // custom selecting
  void SelectSEXT(SDNode *Node);

// Include the pieces autogenerated from the target description.
#include "EVMGenDAGISel.inc"

private:
};
}

void EVMDAGToDAGISel::PostprocessISelDAG() {

}

void EVMDAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(dbgs() << "== "; Node->dump(CurDAG); dbgs() << '\n');
    return;
  }

  switch (Opcode) {
    case EVMISD::ARGUMENT:
      // do not select argument.
      return;
    case ISD::LOAD: {
      const LoadSDNode *LD = cast<LoadSDNode>(Node);

      switch (LD->getExtensionType()) {
        case ISD::SEXTLOAD: {
            // MLOAD -> SIGNEXTEND
            SDValue Src = LD->getBasePtr();
            uint64_t bytesToShift = 32 - (LD->getMemoryVT().getSizeInBits() / 8);
            SDValue shift =CurDAG->getConstant(bytesToShift, SDLoc(Node), MVT::i256);
            SDValue mload = SDValue(CurDAG->getMachineNode(EVM::MLOAD_r,
                                    SDLoc(Node), MVT::i256, Src), 0);
            MachineSDNode * signextend = CurDAG->getMachineNode(EVM::SIGNEXTEND_r,
                        SDLoc(Node), MVT::i256, mload, shift);
            ReplaceNode(Node, signextend);
            return;
        }
        case ISD::ZEXTLOAD: {
          // Load and then zsignextend.
          //  MLOAD > SLL > SRL
          SDValue Src = LD->getBasePtr();
          uint64_t bytesToFill = 256 - LD->getMemoryVT().getSizeInBits();
          SDValue multiplier =CurDAG->getConstant(1 << bytesToFill, SDLoc(Node), MVT::i256);
          SDValue mload = SDValue(CurDAG->getMachineNode(EVM::MLOAD_r,
                                  SDLoc(Node), MVT::i256, Src), 0);
          SDValue mul = SDValue(CurDAG->getMachineNode(EVM::MUL_r, SDLoc(Node),
                                                       MVT::i256, mload, multiplier), 0);

          MachineSDNode * div = CurDAG->getMachineNode(EVM::DIV_r, SDLoc(Node),
                                                       MVT::i256, mul, multiplier);
          ReplaceNode(Node, div);
          return;
        }
        case ISD::EXTLOAD:
        case ISD::NON_EXTLOAD:
          break;
      }
      break;
    }
  }

  SelectCode(Node);
}

bool EVMDAGToDAGISel::SelectAddrFI(SDValue Addr, SDValue &Base, SDValue &Offset) {
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(
        FIN->getIndex(), TLI->getPointerTy(CurDAG->getDataLayout()));
    Offset = CurDAG->getTargetConstant(0, SDLoc(Addr), MVT::i256);
    return true;
  }
  return false;
}

// This pass converts a legalized DAG into a EVM-specific DAG, ready
// for instruction scheduling.
FunctionPass *llvm::createEVMISelDag(EVMTargetMachine &TM) {
  return new EVMDAGToDAGISel(TM);
}
