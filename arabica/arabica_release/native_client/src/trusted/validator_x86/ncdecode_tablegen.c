/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * New table driven generator for a decoder of x86 code.
 *
 * Note: Most of the organization of this document is based on the
 * Opcode Map appendix of one of the following documents:

 * (1) "Intel 64 and IA-32 Architectures
 * Software Developer's Manual (volumes 1, 2a, and 2b; documents
 * 253665.pdf, 253666.pdf, and 253667.pdf)".
 *
 * (2) "Intel 80386 Reference Programmer's Manual" (document
 * http://pdos.csail.mit.edu/6.828/2004/readings/i386/toc.htm).
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "native_client/src/include/portability.h"

#include "native_client/src/shared/utils/types.h"

#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"

/* Note: in general all errors in this module will be fatal.
 * To debug: use gdb or your favorite debugger.
 */
static void fatal(const char* s) {
  fprintf(stderr, "%s\n", s);
  fprintf(stderr, "fatal error, cannot recover\n");
  exit(-1);
}

/* Possible run modes for instructions. */
typedef enum {
  X86_32,       /* Model x86-32 bit instructions. */
  X86_64,       /* Model x86-64-bit instructions. */
  RunModeSize   /* Special end of list marker, denoting the number
                 * of run modes;
                 */
} RunMode;

/* Returns the print name of the given run mode. */
static const char* RunModeName(RunMode mode) {
  switch (mode) {
    case X86_32: return "x86-32 bit mode";
    case X86_64: return "x86-64 bit mode";
    default: assert(0);
  }

  /* NOTREACHED */
  return NULL;
}

/* Defines the run mode files that should be generated. */
static RunMode FLAGS_run_mode = X86_32;

/* Holds the current instruction prefix. */
OpcodePrefix current_opcode_prefix = NoPrefix;

/* Holds the current opcode instruction being built. */
Opcode* current_opcode = NULL;

/* Holds the opcode to model instructions that can't be parsed. */
static Opcode* undefined_opcode = NULL;

/* Holds all opcodes that require a single byte to define */
static Opcode* OpcodeTable[NCDTABLESIZE][OpcodePrefixTypeSize];

/* Holds encodings of prefix bytes. */
const char* PrefixTable[NCDTABLESIZE];

/* Define the prefix name for the given opcode, for the given run mode. */
static void EncodeModedPrefixName(const uint8_t byte, const char* name,
                                  const RunMode mode) {
  if (FLAGS_run_mode == mode) {
    PrefixTable[byte] = name;
  }
}

/* Define the prefix name for the given opcode, for all run modes. */
static void EncodePrefixName(const uint8_t byte, const char* name) {
  EncodeModedPrefixName(byte, name, FLAGS_run_mode);
}

/* Change the current opcode prefix to the given value. */
static void DefineOpcodePrefix(OpcodePrefix prefix) {
  current_opcode_prefix = prefix;
}

/* Define the next operand of the current opcode to have
 * the given kind and flags.
 */
static void DefineOperand(
    OperandKind kind,
    OperandFlags flags) {
  int index;
  assert(NULL != current_opcode);
  index = current_opcode->num_operands++;
  current_opcode->operands[index].kind = kind;
  current_opcode->operands[index].flags = flags;
}

/* Define the next opcode (instruction), initializing with
 * no operands.
 */
static void DefineOpcode(
    const uint8_t opcode,
    const NaClInstType insttype,
    OpcodeFlags flags,
    const InstMnemonic name) {
  int i;
  /* Create opcode and initialize */
  current_opcode = (Opcode*) malloc(sizeof(Opcode));
  if (NULL == current_opcode) {
    fatal("DefineOpcode: malloc failed");
  }
  current_opcode->opcode = opcode;
  current_opcode->insttype = insttype;
  current_opcode->flags = flags;
  current_opcode->name = name;
  current_opcode->next_rule = NULL;

  /* undefine all operands. */
  current_opcode->num_operands = 0;
  for (i = 0; i < MAX_NUM_OPERANDS; ++i) {
    DefineOperand(Unknown_Operand, 0);
  }
  /* Now reset number of operands to zero. */
  current_opcode->num_operands = 0;

  /* Dont install if not the correct run mode. */
  if (name == InstUndefined ||
      ((flags & OCF(Opcode32Only)) && FLAGS_run_mode == X86_64) ||
      ((flags & OCF(Opcode64Only)) && FLAGS_run_mode == X86_32)) {
    /* Instruction not defined for run mode, don't install. */
    return;
  }

  /* Install Opcode. */
  if (NULL == OpcodeTable[opcode][current_opcode_prefix]) {
    OpcodeTable[opcode][current_opcode_prefix] = current_opcode;
  } else {
    Opcode* next = OpcodeTable[opcode][current_opcode_prefix];
    while (NULL != next->next_rule) {
      next = next->next_rule;
    }
    next->next_rule = current_opcode;
  }
}

static void InitializeOpcodeTables() {
  int i;
  OpcodePrefix prefix;
  /* Before starting, verify that we have defined OpcodeFlags and OperandFlags
   * big enough to hold the flags associated with it.
   */
  assert(OpcodeFlagSize <= sizeof(OpcodeFlags)*8);
  assert(OperandFlagSize <= sizeof(OperandFlags)*8);

  for (i = 0; i < NCDTABLESIZE; ++i) {
    for (prefix = NoPrefix; prefix < OpcodePrefixTypeSize; ++prefix) {
      OpcodeTable[i][prefix] = NULL;
    }
    PrefixTable[i] = "0";
  }
  DefineOpcodePrefix(NoPrefix);
  DefineOpcode(0x0, NACLi_ILLEGAL, 0, InstUndefined);
  undefined_opcode = current_opcode;
}

/* Define binary operation XX+00 to XX+05, for the binary operators
 * add, or, adc, sbb, and, sub, xor, and cmp. Base is the value XX.
 * Name is the nae of the operation. Extra flags are any additional
 * flags that are true to a specific binary operator, rather than
 * all binary operators.
 */
static void BuildBinaryOps_00_05(const uint8_t base,
                                 const NaClInstType itype,
                                 const InstMnemonic name,
                                 const OperandFlags extra_flags) {
  DefineOpcode(
      base,
      itype,
      OCF(OpcodeUsesModRm) | OCF(OpcodeRex) | OCF(OperandSize_b) | extra_flags,
      name);
  DefineOperand(E_Operand,
                OPF(OpUse) | OPF(OpSet) | OPF(RexExcludesAhBhChDh));
  DefineOperand(G_Operand,
                OPF(OpUse) | OPF(RexExcludesAhBhChDh));

  DefineOpcode(
      base+1,
      itype,
      OCF(OpcodeUsesModRm) | OCF(OperandSize_v) | OCF(OperandSize_w) |
      extra_flags,
      name);
  DefineOperand(E_Operand, OPF(OpUse) | OPF(OpSet));
  DefineOperand(G_Operand, OPF(OpUse));

  DefineOpcode(
      base+1,
      itype,
      OCF(OpcodeUsesModRm) | OCF(OpcodeUsesRexW) | OCF(OperandSize_o) |
      OCF(Opcode64Only) | extra_flags,
      name);
  DefineOperand(Eo_Operand, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Go_Operand, OPF(OpUse));

  DefineOpcode(
      base+2,
      itype,
      OCF(OpcodeUsesModRm) | OCF(OpcodeRex) | OCF(OperandSize_b) | extra_flags,
      name);
  DefineOperand(G_Operand, OPF(OpUse) | OPF(OpSet) | OPF(RexExcludesAhBhChDh));
  DefineOperand(E_Operand, OPF(OpUse) | OPF(RexExcludesAhBhChDh));

  DefineOpcode(
      base+3,
      itype,
      OCF(OpcodeUsesModRm) | OCF(OperandSize_w) | OCF(OperandSize_v) |
      extra_flags,
      name);
  DefineOperand(G_Operand, OPF(OpUse) | OPF(OpSet));
  DefineOperand(E_Operand, OPF(OpUse));

  DefineOpcode(
      base+3,
      itype,
      OCF(OpcodeUsesModRm) | OCF(OpcodeUsesRexW) | OCF(OperandSize_o) |
      OCF(Opcode64Only) | extra_flags,
      name);
  DefineOperand(Go_Operand, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Eo_Operand, OPF(OpUse));

  DefineOpcode(
      base+4,
      itype,
      OCF(OpcodeIb) | extra_flags,
      name);
  DefineOperand(RegAL, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Ib_Operand, OPF(OpUse));

  DefineOpcode(
      base+5,
      itype,
      OCF(OpcodeIv) | OCF(OperandSize_v) | extra_flags,
      name);
  DefineOperand(RegEAX, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Iv_Operand, OPF(OpUse));

  DefineOpcode(
      base+5,
      itype,
      OCF(OpcodeIv) | OCF(OpcodeUsesRexW) | OCF(OperandSize_o) |
      OCF(Opcode64Only) | extra_flags,
      name);
  DefineOperand(RegRAX, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Iv_Operand, OPF(OpUse));

  DefineOpcode(
      base+5,
      itype,
      OCF(OpcodeIw) | extra_flags,
      name);
  DefineOperand(RegAX, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Iw_Operand, OPF(OpUse));
}

/* Holds the sequence of opcode bases that we could be offsetting with
 * a register value.
 */
static OperandKind OpcodeBaseMinus[8] = {
  OpcodeBaseMinus0,
  OpcodeBaseMinus1,
  OpcodeBaseMinus2,
  OpcodeBaseMinus3,
  OpcodeBaseMinus4,
  OpcodeBaseMinus5,
  OpcodeBaseMinus6,
  OpcodeBaseMinus7,
};

/* Define the increment and descrement operators XX+00 to XX+07. Base is
 * the value XX. Name is the name of the increment/decrement operator.
 */
static void DefineIncOrDec_00_07(
    const uint8_t base,
    const InstMnemonic name) {
  int i;
  for (i = 0; i < 8; ++i) {
    DefineOpcode(
        base+i,
        NACLi_386L,
        OCF(Opcode32Only) | OCF(OpcodePlusR) |
        OCF(OperandSize_w) | OCF(OperandSize_v),
        name);
    DefineOperand(OpcodeBaseMinus[i], OPF(OperandExtendsOpcode));
    DefineOperand(G_Operand, OPF(OpUse) | OPF(OpSet));
  }
}

/* Define the push and pop operators XX+00 to XX+17. Base is
 * the value of XX. Name is the name of the push/pop operator.
 */
static void DefinePushOrPop_00_07(
    const uint8_t base,
    const InstMnemonic name) {
  int i;
  for (i = 0; i < 8; ++i) {
    DefineOpcode(
        base+i,
        NACLi_386,
        OCF(OpcodePlusR) | OCF(OperandSize_w),
        name);
    DefineOperand(OpcodeBaseMinus[i], OPF(OperandExtendsOpcode));
    DefineOperand(RegRESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
    DefineOperand(Gw_Operand, OPF(OpSet));

    DefineOpcode(
        base+i,
        NACLi_386,
        OCF(OpcodePlusR) | OCF(OperandSize_v) | OCF(Opcode32Only),
        name);
    DefineOperand(OpcodeBaseMinus[i], OPF(OperandExtendsOpcode));
    DefineOperand(RegESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
    DefineOperand(G_Operand, OPF(OpSet));

    DefineOpcode(
        base+i,
        NACLi_386,
        OCF(OpcodePlusR) | OCF(OperandSize_o) |
        OCF(Opcode64Only) | OCF(OperandSizeDefaultIs64),
        name);
    DefineOperand(OpcodeBaseMinus[i], OPF(OperandExtendsOpcode));
    DefineOperand(RegRSP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
    DefineOperand(G_Operand, OPF(OpSet));
  }
}

static void DefinePrefixBytes() {
  EncodePrefixName(0x26, "kPrefixSEGSS");
  EncodePrefixName(0x2e, "kPrefixSEGCS");
  EncodePrefixName(0x3e, "kPrefixSEGDS");
  EncodePrefixName(0x64, "kPrefixSEGFS");
  EncodePrefixName(0x65, "kPrefixSEGGS");
  EncodePrefixName(0x66, "kPrefixDATA16");
  EncodePrefixName(0x67, "kPrefixADDR16");
  EncodePrefixName(0xf0, "kPrefixLOCK");
  EncodePrefixName(0xf2, "kPrefixREPNE");
  EncodePrefixName(0xf3, "kPrefixREP");

  if (NACL_TARGET_SUBARCH == 64) {
    int i;
    for (i = 0; i < 16; ++i) {
      EncodePrefixName(0x40+i, "kPrefixREX");
    }
  }
}

/* Build the set of x64 opcode (instructions). */
static void BuildOpcodeTables() {
  InitializeOpcodeTables();

  DefinePrefixBytes();

  DefineOpcodePrefix(NoPrefix);

  BuildBinaryOps_00_05(0x00, NACLi_386L, InstAdd, OCF(OpcodeLockable));

  DefineOpcode(0x06, NACLi_ILLEGAL, OCF(Opcode32Only), InstPush);
  DefineOperand(RegESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
  DefineOperand(RegES, OPF(OpUse));

  DefineOpcode(0x07, NACLi_ILLEGAL, OCF(Opcode32Only), InstPop);
  DefineOperand(RegESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
  DefineOperand(RegES, OPF(OpSet));

  BuildBinaryOps_00_05(0x08, NACLi_386L, InstOr, OCF(OpcodeLockable));

  DefineOpcode(0x0e, NACLi_ILLEGAL, OCF(Opcode32Only), InstPush);
  DefineOperand(RegRESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
  DefineOperand(RegCS, OPF(OpUse));

  /* 0x0F is a two-byte opcode prefix. */

  BuildBinaryOps_00_05(0x10, NACLi_386L, InstAdc, OCF(OpcodeLockable));

  DefineOpcode(0x16, NACLi_ILLEGAL, OCF(Opcode32Only), InstPush);
  DefineOperand(RegRESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
  DefineOperand(RegSS, OPF(OpUse));

  DefineOpcode(0x17, NACLi_ILLEGAL, OCF(Opcode32Only), InstPop);
  DefineOperand(RegRESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
  DefineOperand(RegSS, OPF(OpSet));

  BuildBinaryOps_00_05(0x18, NACLi_386L, InstSbb, OCF(OpcodeLockable));

  DefineOpcode(0x1e, NACLi_ILLEGAL, OCF(Opcode32Only), InstPush);
  DefineOperand(RegRESP, OPF(OpUse) | OPF(OpSet) | OPF(OpImplicit));
  DefineOperand(RegDS, OPF(OpUse));

  DefineOpcode(0x1f, NACLi_ILLEGAL, OCF(Opcode32Only), InstPop);

  BuildBinaryOps_00_05(0x20, NACLi_386L, InstAnd, OCF(OpcodeLockable));

  DefineOpcode(0x27, NACLi_ILLEGAL, OCF(Opcode32Only), InstDaa);

  BuildBinaryOps_00_05(0x28, NACLi_386L, InstSub, OCF(OpcodeLockable));

  DefineOpcode(0x2f, NACLi_ILLEGAL, OCF(Opcode32Only), InstDas);

  BuildBinaryOps_00_05(0x30, NACLi_386L, InstXor, OCF(OpcodeLockable));

  DefineOpcode(0x37, NACLi_ILLEGAL, OCF(Opcode32Only), InstAaa);

  BuildBinaryOps_00_05(0x38, NACLi_386, InstCmp, 0);

  /* Ox3e is segment ds prefix */

  DefineOpcode(0x3f, NACLi_ILLEGAL, OCF(Opcode32Only), InstAas);

  DefineIncOrDec_00_07(0x40, InstInc);
  DefineIncOrDec_00_07(0x48, InstDec);
  DefinePushOrPop_00_07(0x50, InstPush);
  DefinePushOrPop_00_07(0x58, InstPop);

  DefineOpcode(
      0x60,
      NACLi_ILLEGAL,
      OCF(Opcode32Only) | OCF(OperandSize_w),
      InstPusha);
  DefineOperand(RegESP, OPF(OpImplicit) | OPF(OpUse) | OPF(OpSet));
  DefineOperand(RegGP7, OPF(OpImplicit) | OPF(OpUse));

  DefineOpcode(
      0x60,
      NACLi_ILLEGAL,
      OCF(Opcode32Only) | OCF(OperandSize_v),
      InstPushad);
  DefineOperand(RegESP, OPF(OpImplicit) | OPF(OpUse) | OPF(OpSet));
  DefineOperand(RegGP7, OPF(OpImplicit) | OPF(OpUse));

  DefineOpcode(
      0x61,
      NACLi_ILLEGAL,
      OCF(Opcode32Only) | OCF(OperandSize_w),
      InstPopa);
  DefineOperand(RegESP, OPF(OpImplicit) | OPF(OpUse) | OPF(OpSet));
  DefineOperand(RegGP7, OPF(OpImplicit) | OPF(OpSet));

  DefineOpcode(
     0x61,
     NACLi_ILLEGAL,
     OCF(Opcode32Only) | OCF(OperandSize_v),
     InstPopad);
  DefineOperand(RegESP, OPF(OpImplicit) | OPF(OpUse) | OPF(OpSet));
  DefineOperand(RegGP7, OPF(OpImplicit) | OPF(OpSet));

  /* TODO(karl) figure out how we specify three registers. */
  /* TODO(karl) figure out how to print A2&A3 */
  DefineOpcode(
      0x62,
      NACLi_ILLEGAL,
      OCF(Opcode32Only) | OCF(OpcodeInModRm) | OCF(OperandSize_v),
      InstBound);
  DefineOperand(Gv_Operand, OPF(OpUse));
  DefineOperand(Ev_Operand, OPF(OpUse));
  DefineOperand(Ev_Operand, OPF(OpUse));

  /* TODO(karl) Finish adding instructions. */

  /* For the moment, show some examples of Opcodes in Mod/Rm. */
  DefineOpcode(
      0x80,
      NACLi_386L,
      OCF(OpcodeInModRm) | OCF(OpcodeIb) | OCF(OpcodeLockable),
      InstAdd);
  DefineOperand(Opcode0, OPF(OperandExtendsOpcode));
  DefineOperand(E_Operand, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Ib_Operand, OPF(OpUse));

  DefineOpcode(
      0x80,
      NACLi_386L,
      OCF(OpcodeInModRm) | OCF(Opcode64Only) | OCF(OpcodeRex) |
      OCF(OpcodeIb) | OCF(OpcodeLockable),
      InstAdd);
  DefineOperand(Opcode0, OPF(OperandExtendsOpcode));
  DefineOperand(E_Operand, OPF(OpUse) | OPF(OpSet));
  DefineOperand(Ib_Operand, OPF(OpUse));
}

/* Generate header information, based on the executable name in argv0,
 * and the file to be generated (defined by fname).
 */
static void PrintHeader(FILE* f, const char* argv0, const char* fname) {
  time_t timeofday;
  if (time(&timeofday) < 0) {
    fprintf(stderr, "time() failed\n");
    exit(-1);
  }
  fprintf(f, "/* %s\n", fname);
  fprintf(f, " * THIS FILE IS AUTO-GENERATED. DO NOT EDIT.\n");
  fprintf(f, " * This file was auto-generated by %s\n", argv0);
  fprintf(f, " * on: %s\n", ctime(&timeofday));
  fprintf(f, " *\n");
  fprintf(f, " * Compiled for %s.\n", RunModeName(FLAGS_run_mode));
  fprintf(f, " *\n");
  fprintf(f, " * You must include ncopcode_desc.h before this file.\n");
  fprintf(f, " */\n\n");
}

/* Collect the number of opcode (instructions) in the given table,
 * and return the number found.
 */
static int CountNumberOpcodes() {
  int i;
  OpcodePrefix prefix;
  int count = 0;
  for (i = 0; i < NCDTABLESIZE; ++i) {
    for (prefix = NoPrefix; prefix < OpcodePrefixTypeSize; ++prefix) {
      Opcode* next = OpcodeTable[i][prefix];
      while (NULL != next) {
        ++count;
        next = next->next_rule;
      }
    }
  }
  return count;
}

/* Print out which bytes correspond to prefix bytes. */
static void PrintPrefixTable(FILE* f) {
  int opc;
  fprintf(f, "static const uint32_t kPrefixTable[NCDTABLESIZE] = {");
  for (opc = 0; opc < NCDTABLESIZE; opc++) {
    if (0 == opc % 16) {
      fprintf(f, "\n  /* 0x%02x-0x%02x */\n  ", opc, opc + 15);
    }
    fprintf(f, "%s, ", PrefixTable[opc]);
  }
  fprintf(f, "\n};\n\n");
}

/* Print out the contents of the defined instructions into the given file. */
static void PrintDecodeTables(FILE* f) {
  int i;
  OpcodePrefix prefix;
  int count = 0;

  /* Build table of all possible instructions. Note that we build
   * build the list of instructions by using a "
  */

  int num_opcodes = CountNumberOpcodes();

  /* lookup_index holds the number of the opcode (instruction) that
   * begins the list of instructions for the corresponding opcode
   * in OpcodeTable.
   */
  int lookup_index[NCDTABLESIZE][OpcodePrefixTypeSize];

  fprintf(f, "static Opcode g_Opcodes[%d] = {\n", num_opcodes);
  for (prefix = NoPrefix; prefix < OpcodePrefixTypeSize; ++prefix) {
    for (i = 0; i < NCDTABLESIZE; ++i) {
      /* Build the list of instructions by knowing that the next
       * element in the list will always follow the current entry,
       * when added to the array.
       */
      Opcode* next = OpcodeTable[i][prefix];
      if (NULL == next) {
        lookup_index[i][prefix] = -1;
      } else {
        int lookahead = count + 1;
        lookup_index[i][prefix] = count;
        while (NULL != next) {
          PrintOpcodeTablegen(f, count, next, lookahead);
          next = next->next_rule;
          ++lookahead;
          ++count;
        }
      }
    }
  }
  fprintf(f, "};\n\n");

  /* Print out the undefined opcode */
  fprintf(f, "static Opcode g_Undefined_Opcode = \n");
  PrintOpcode(f, undefined_opcode);

  /* Now print lookup table of rules. */
  fprintf(f,
          "static Opcode* "
          "g_OpcodeTable[OpcodePrefixTypeSize][NCDTABLESIZE] = {\n");
  for (prefix = NoPrefix; prefix < OpcodePrefixTypeSize; ++prefix) {
    fprintf(f,"/* %s */\n", OpcodePrefixName(prefix));
    fprintf(f, "{\n");
    for (i = 0; i < NCDTABLESIZE; ++i) {
      /* Take advantage of the fact that the lists were added to
       * the array of opcodes such that the next element in the list
       * will always follow the current entry.
       */
      Opcode* next = OpcodeTable[i][prefix];
      fprintf(f, "  /* %02x */ ", i);
      if (NULL == next) {
        fprintf(f, "NULL");
      } else {
        fprintf(f, "g_Opcodes + %d", lookup_index[i][prefix]);
      }
      fprintf(f, "  ,\n");
    }
    fprintf(f, "},\n");
  }
  fprintf(f, "};\n\n");

  PrintPrefixTable(f);
}

/* Open the given file using the given directives (how). */
FILE* mustopen(const char* fname, const char* how) {
  FILE* f = fopen(fname, how);
  if (f == NULL) {
    fprintf(stderr, "could not fopen(%s, %s)\n", fname, how);
    fprintf(stderr, "exiting now\n");
    exit(-1);
  }
  return f;
}

/* Recognizes flags in argv, processes them, and then removes them.
 * Returns the updated value for argc.
 */
int GrokFlags(int argc, const char* argv[]) {
  int i;
  int new_argc;
  if (argc == 0) return 0;
  new_argc = 1;
  for (i = 1; i < argc; ++i) {
    if (0 == strcmp("-m32", argv[i])) {
      FLAGS_run_mode = X86_32;
    } else if (0 == strcmp("-m64", argv[i])) {
      FLAGS_run_mode = X86_64;
    } else {
      argv[new_argc++] = argv[i];
    }
  }
  return new_argc;
}

int main(const int argc, const char* argv[]) {
  FILE *f;
  int new_argc = GrokFlags(argc, argv);
  if (new_argc != 2) {
    fprintf(stderr,
            "ERROR: usage: ncdecode_tablegen [options] "
            "file\n");
    return -1;
  }

  BuildOpcodeTables();

  f = mustopen(argv[1], "w");
  PrintHeader(f, argv[0], argv[1]);
  PrintDecodeTables(f);
  fclose(f);

  return 0;
}
