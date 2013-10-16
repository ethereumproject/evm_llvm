//===- llvm/unittest/DebugInfo/DWARFFormValueTest.cpp ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../lib/CodeGen/AsmPrinter/DIE.h"
#include "../lib/CodeGen/AsmPrinter/DIEHash.h"
#include "llvm/Support/Dwarf.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {
TEST(DIEHashData1Test, DIEHash) {
  DIEHash Hash;
  DIE Die(dwarf::DW_TAG_base_type);
  DIEInteger Size(4);
  Die.addValue(dwarf::DW_AT_byte_size, dwarf::DW_FORM_data1, &Size);
  uint64_t MD5Res = Hash.computeTypeSignature(&Die);
  ASSERT_EQ(0x1AFE116E83701108ULL, MD5Res);
}

TEST(DIEHashTrivialTypeTest, DIEHash) {
  // A complete, but simple, type containing no members and defined on the first
  // line of a file.
  DIE FooType(dwarf::DW_TAG_structure_type);
  DIEInteger One(1);
  FooType.addValue(dwarf::DW_AT_byte_size, dwarf::DW_FORM_data1, &One);

  // Line and file number are ignored.
  FooType.addValue(dwarf::DW_AT_decl_file, dwarf::DW_FORM_data1, &One);
  FooType.addValue(dwarf::DW_AT_decl_line, dwarf::DW_FORM_data1, &One);
  uint64_t MD5Res = DIEHash().computeTypeSignature(&FooType);

  // The exact same hash GCC produces for this DIE.
  ASSERT_EQ(0x715305ce6cfd9ad1ULL, MD5Res);
}
}
