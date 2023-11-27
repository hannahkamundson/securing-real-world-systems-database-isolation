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

/* Descriptors to model instructions, opcodes, and instruction operands. */

#include <stdio.h>

#define NEEDSNACLINSTTYPESTRING
#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"
#include "native_client/src/shared/utils/types.h"

/* Define the corresponding names of OpcodeFlag's. */
static const char* g_OpcodeFlagName[OpcodeFlagSize] = {
  "OpcodeUsesRexW",
  "OpcodeHasRexR",
  "OpcodeHasNoRexR",
  "OpcodeInModRm",
  "OpcodeUsesModRm",
  "OpcodeIb",
  "OpcodeIw",
  "OpcodeIv",
  "OpcodeIo",
  "OpcodePlusR",
  "OpcodePlusI",
  "OpcodeRex",
  "OpcodeLegacy",
  "OpcodeLockable",
  "Opcode32Only",
  "Opcode64Only",
  "OperandSize_b",
  "OperandSize_w",
  "OperandSize_v",
  "OperandSize_o",
  "OperandSizeDefaultIs64",
  "OpcodeOF"
};

const char* OpcodeFlagName(const OpcodeFlag flag) {
  return flag < OpcodeFlagSize ? g_OpcodeFlagName[flag] : "OpcodeFlag???";
}

/* Define the corresponding names of OperandKind. */
static const char* g_OperandKindName[OperandKindSize] = {
  "Unknown_Operand",
  "A_Operand",
  "Aw_Operand",
  "Av_Operand",
  "Ao_Operand",
  "E_Operand",
  "Eb_Operand",
  "Ew_Operand",
  "Ev_Operand",
  "Eo_Operand",
  "G_Operand",
  "Gb_Operand",
  "Gw_Operand",
  "Gv_Operand",
  "Go_Operand",
  "I_Operand",
  "Ib_Operand",
  "Iw_Operand",
  "Iv_Operand",
  "Io_Operand",
  "J_Operand",
  "Jb_Operand",
  "Jw_Operand",
  "Jv_Operand",
  "Jo_Operand",
  "M_Operand",
  "Mb_Operand",
  "Mw_Operand",
  "Mv_Operand",
  "Mo_Operand",
  "Mqo_Operand",
  "Mpw_Operand",
  "Mpv_Operand",
  "Mpo_Operand",
  "O_Operand",
  "Ob_Operand",
  "Ow_Operand",
  "Ov_Opernd",
  "Oo_Operand",
  "S_Operand",
  "RegUnknown",
  "RegAL",
  "RegBL",
  "RegCL",
  "RegDL",
  "RegAH",
  "RegBH",
  "RegCH",
  "RegDH",
  "RegDIL",
  "RegSIL",
  "RegBPL",
  "RegSPL",
  "RegR8L",
  "RegR9L",
  "RegR10L",
  "RegR11L",
  "RegR12L",
  "RegR13L",
  "RegR14L",
  "RegR15L",
  "RegAX",
  "RegBX",
  "RegCX",
  "RegDX",
  "RegSI",
  "RegDI",
  "RegBP",
  "RegSP",
  "RegR8W",
  "RegR9W",
  "RegR10W",
  "RegR11W",
  "RegR12W",
  "RegR13W",
  "RegR14W",
  "RegR15W",
  "RegEAX",
  "RegEBX",
  "RegECX",
  "RegEDX",
  "RegESI",
  "RegEDI",
  "RegEBP",
  "RegESP",
  "RegR8D",
  "RegR9D",
  "RegR10D",
  "RegR11D",
  "RegR12D",
  "RegR13D",
  "RegR14D",
  "RegR15D",
  "RegCS",
  "RegDS",
  "RegSS",
  "RegES",
  "RegFS",
  "RegGS",
  "RegEFLAGS",
  "RegRFLAGS",
  "RegEIP",
  "RegRIP",
  "RegRAX",
  "RegRBX",
  "RegRCX",
  "RegRDX",
  "RegRSI",
  "RegRDI",
  "RegRBP",
  "RegRSP",
  "RegR8",
  "RegR9",
  "RegR10",
  "RegR11",
  "RegR12",
  "RegR13",
  "RegR14",
  "RegR15",
  "RegRESP",
  "RegREAX",
  "RegGP7",
  "OpcodeBaseMinus0",
  "OpcodeBaseMinus1",
  "OpcodeBaseMinus2",
  "OpcodeBaseMinus3",
  "OpcodeBaseMinus4",
  "OpcodeBaseMinus5",
  "OpcodeBaseMinus6",
  "OpcodeBaseMinus7",
  "Opcode0",
  "Opcode1",
  "Opcode2",
  "Opcode3",
  "Opcode4",
  "Opcode5",
  "Opcode6",
  "Opcode7"
};

const char* OperandKindName(const OperandKind kind) {
  return kind < OperandKindSize ? g_OperandKindName[kind] : "OperandKind???";
}

/* Define the corresponding names of OperandFlag. */
static const char* g_OperandFlagName[OperandFlagSize] = {
  "RexExcludesAhBhChDh",
  "OpUse",
  "OpSet",
  "OpImplicit",
  "OperandExtendsOpcode",
};

const char* OperandFlagName(const OperandFlag flag) {
  return flag < OperandFlagSize ? g_OperandFlagName[flag] : "OperandFlag???";
}

/* Define the corresponding names of InstMnemonic. */
static const char* g_InstMnemonicName[InstMnemonicSize] = {
  "Undefined",
  "Aaa",
  "Aas",
  "Adc",
  "Add",
  "And",
  "Bound",
  "Cmp",
  "Daa",
  "Das",
  "Dec",
  "Inc",
  "Or",
  "Pop",
  "Popa",
  "Popad",
  "Push",
  "Pusha",
  "Pushad",
  "Sbb",
  "Sub",
  "Xor",
};

const char* InstMnemonicName(const InstMnemonic mnemonic) {
  return mnemonic < InstMnemonicSize
      ? g_InstMnemonicName[mnemonic]
      : "InstMnemonic???";
}

/* Define the corresponding names of OpcodePrefix. */
static const char* g_OpcodePrefixName[OpcodePrefixTypeSize] = {
  "NoPrefix",
  "Prefix0F",
  "PrefixF20F",
  "PrefixF30F",
  "Prefix66OF",
  "Prefix0F0F",
  "Prefix0F38",
  "Prefix660F38",
  "PrefixF20F38",
  "Prefix0F3A",
  "Prefix660F3A"
};

const char* OpcodePrefixName(OpcodePrefix prefix) {
  return prefix < OpcodePrefixTypeSize
      ? g_OpcodePrefixName[prefix]
      : "OpcodePrefix???";
}

void PrintOperand(FILE* f, Operand* operand) {
  fprintf(f, "      { %s, ", OperandKindName(operand->kind));
  if (operand->flags) {
    OperandFlag i;
    Bool first = TRUE;
    for (i = 0; i < OperandFlagSize; ++i) {
      if (operand->flags & OPF(i)) {
        if (first) {
          first = FALSE;
        } else {
          fprintf(f, " | ");
        }
        fprintf(f, "OPF(%s)", OperandFlagName(i));
      }
    }
  } else {
    fprintf(f, "0");
  }
  fprintf(f, " },\n");
}

void PrintOpcodeTablegen(FILE* f, int index, Opcode* opcode, int lookahead) {
  int i;
  if (index >= 0) {
    fprintf(f, "  /* %d */\n", index);
  }
  fprintf(f, "  { 0x%02x,\n", opcode->opcode);
  fprintf(f, "    %s,\n", kNaClInstTypeString[opcode->insttype]);
  fprintf(f, "    ");
  if (opcode->flags) {
    Bool first = TRUE;
    for (i = 0; i < OpcodeFlagSize; ++i) {
      if (opcode->flags & OCF(i)) {
        if (first) {
          first = FALSE;
        } else {
          fprintf(f, " | ");
        }
        fprintf(f, "OCF(%s)", OpcodeFlagName(i));
      }
    }
  } else {
    fprintf(f, "0");
  }
  fprintf(f, ",\n");
  fprintf(f, "    Inst%s,\n", InstMnemonicName(opcode->name));
  fprintf(f, "    %u, {\n", opcode->num_operands);
  for (i = 0; i < MAX_NUM_OPERANDS; ++i) {
    PrintOperand(f, opcode->operands + i);
  }
  fprintf(f, "    },\n");
  if (index < 0 || NULL == opcode->next_rule) {
    fprintf(f, "    NULL\n");
  } else {
    fprintf(f, "    g_Opcodes + %d\n", lookahead);
  }
  fprintf(f, "  }%s\n", index < 0 ? ";" : ",");
}

void PrintOpcode(FILE* f, Opcode* opcode) {
  PrintOpcodeTablegen(f, -1, opcode, 0);
}
