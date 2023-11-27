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
 * Translates the recognized opcode (instruction) in the instruction state
 * into an opcode expression.
 */

#include <stdio.h>
#include <assert.h>

#include "native_client/src/trusted/validator_x86/nc_inst_trans.h"
#include "native_client/src/trusted/validator_x86/nc_inst_state.h"
#include "native_client/src/trusted/validator_x86/nc_inst_state_internal.h"
#include "native_client/src/trusted/validator_x86/ncop_exps.h"

/* To turn on debugging of instruction decoding, change value of
 * DEBUGGING to 1.
 */
#define DEBUGGING 0

#if DEBUGGING
/* Defines to execute statement(s) s if in debug mode. */
#define DEBUG(s) s
#else
/* Defines to not include statement(s) s if not in debugging mode. */
#define DEBUG(s) do { if (0) { s; } } while(0)
#endif

/* Model an unknown expression for expression tres. */
static UndefinedOpExpression undefined_op = { { UndefinedTree } };

/* Defines the maximum predefined (immediate) value that will
 * be implemented as a predefined constant.
 */
#define PREDEFINED_IMMEDIATE_1_LIMIT 10

/* Model some predefined immediate values, so that we can use
 * then as predefined constants.
 */
static Immediate1OpExpression
  predefined_immediate_1[PREDEFINED_IMMEDIATE_1_LIMIT] = {
  { { { ImmediateNode }, 1 }, 0 },
  { { { ImmediateNode }, 1 }, 1 },
  { { { ImmediateNode }, 1 }, 2 },
  { { { ImmediateNode }, 1 }, 3 },
  { { { ImmediateNode }, 1 }, 4 },
  { { { ImmediateNode }, 1 }, 5 },
  { { { ImmediateNode }, 1 }, 6 },
  { { { ImmediateNode }, 1 }, 7 },
  { { { ImmediateNode }, 1 }, 8 },
  { { { ImmediateNode }, 1 }, 9 }
};

/* Allocate an expression node of the given size, using the given
 * memory workspace.
 */
OpExpressionNode* AllocOpExpressionNode(
    size_t size,
    OpExpressionWorkspace* workspace) {
  /* First compute the number of elements (within the workspace) that
   * are needed to hold an expression tree of the given size.
   */
  int num_elements =
      (size + sizeof(OpExpressionElement) - 1) / sizeof(OpExpressionElement);
  if (workspace->next_available + num_elements < OpExpressionWorkspaceSize) {
    /* The workspace still has room, allocate memory to meet the request. */
    OpExpressionNode* space =
        (OpExpressionNode*) (workspace->space + workspace->next_available);
    workspace->next_available += num_elements;
    return space;
  } else {
    /* Generate a warning so that we know this happens, but malloc so
     * that we don't kill the executable (just let the memory leak).
     * Note: We are assuming that this should be rare, and hence, a
     * little memory leakage is ok.
     */
    printf("*Warn* Out of preallocated OpExpression space\n");
    return (OpExpressionNode*) malloc(size);
  }
}

/* Allocate and initialize an opcode expression, with the given opcode
 * (instruction) pattern, in the given memory workspace. Initialize the
 * node as having no operands.
 */
OpcodeOpExpression* CreateOpcodeOp(
   Opcode* opcode,
   OpExpressionWorkspace* workspace) {
  int i;
  OpcodeOpExpression* op = (OpcodeOpExpression*)
      AllocOpExpressionNode(sizeof(OpcodeOpExpression), workspace);
  op->base_class.kind = OpcodeNode;
  op->opcode = opcode;
  op->number_operands = 0;
  for (i = 0; i < MAX_OP_OPERANDS; ++i) {
    op->operands[i] = NULL;
  }
  return op;
}

/* Allocate and initialize an operand expression, with the given
 * operand index (of the corresponding opcode pattern), in the
 * given memory workspace. Subtree is the expression tree to be
 * associated with the operand expression.
 */
OperandOpExpression* CreateOperandOp(
    int operand_index,
    Operand* operand,
    OpExpressionNode* subtree,
    OpExpressionWorkspace* workspace) {
  OperandOpExpression* op = (OperandOpExpression*)
      AllocOpExpressionNode(sizeof(OperandOpExpression), workspace);
  op->base_class.kind = OperandNode;
  op->operand = operand;
  op->operand_index = operand_index;
  op->subtree = subtree;
  return op;
}

/* Allocate and initialize a one byte immediate value, in the
 * given memory workspace.
 */
Immediate1OpExpression* CreateImmediate1Op(
    uint8_t value,
    OpExpressionWorkspace* workspace) {
  if (value < PREDEFINED_IMMEDIATE_1_LIMIT) {
    return predefined_immediate_1 + value;
  } else {
    Immediate1OpExpression* op = (Immediate1OpExpression*)
        AllocOpExpressionNode(sizeof(Immediate1OpExpression), workspace);
    op->base_class.base_class.kind = ImmediateNode;
    op->base_class.width = 1;
    op->value = value;
    return op;
  }
}

/* Allocate and initialize a two byte immediate value, in the
 * given memory workspace.
 */
Immediate2OpExpression* CreateImmediate2Op(
    uint16_t value,
    OpExpressionWorkspace* workspace) {
  Immediate2OpExpression* op = (Immediate2OpExpression*)
      AllocOpExpressionNode(sizeof(Immediate2OpExpression), workspace);
  op->base_class.base_class.kind = ImmediateNode;
  op->base_class.width = 2;
  op->value = value;
  return op;
}


/* Allocate and initialize a four byte immediate value, in the
 * given memory workspace.
 */
Immediate4OpExpression* CreateImmediate4Op(
    uint32_t value,
    OpExpressionWorkspace* workspace) {
  Immediate4OpExpression* op = (Immediate4OpExpression*)
      AllocOpExpressionNode(sizeof(Immediate4OpExpression), workspace);
  op->base_class.base_class.kind = ImmediateNode;
  op->base_class.width = 4;
  op->value = value;
  return op;
}

/* Allocate and initalize a segment address, using the given values,
 * in the given memory workspace.
 */
SegmentAddressOpExpression* CreateSegmentAddressOp(
    RegisterOpExpression* base,
    OpExpressionNode* offset,
    OpExpressionWorkspace* workspace) {
  SegmentAddressOpExpression* op = (SegmentAddressOpExpression*)
      AllocOpExpressionNode(sizeof(SegmentAddressOpExpression), workspace);
  op->base_class.kind = SegmentAddress;
  op->base = base;
  op->offset = offset;
  return op;
}

/* Allocate and initialize a memory offset, using the given values,
 * in the given memory workspace.
 */
MemoryOffsetOpExpression* CreateMemoryOffsetOp(
    ImmediateOpExpression* displacement,
    RegisterOpExpression* base,
    RegisterOpExpression* index,
    uint8_t scale,
    OpExpressionWorkspace* workspace) {
  MemoryOffsetOpExpression* op = (MemoryOffsetOpExpression*)
      AllocOpExpressionNode(sizeof(MemoryOffsetOpExpression), workspace);
  op->base_class.kind = MemoryOffset;
  op->displacement = displacement;
  op->base = base;
  op->index = index;
  op->scale = CreateImmediate1Op(scale, workspace);
  return op;
}

/* Unknown register */
static RegisterOpExpression register_Unknown = {{RegisterNode}, RegUnknown};

/* 8-bit registers */
static RegisterOpExpression register_AL = {{RegisterNode}, RegAL};
static RegisterOpExpression register_BL = {{RegisterNode}, RegBL};
static RegisterOpExpression register_CL = {{RegisterNode}, RegCL};
static RegisterOpExpression register_DL = {{RegisterNode}, RegDL};
static RegisterOpExpression register_AH = {{RegisterNode}, RegAH};
static RegisterOpExpression register_BH = {{RegisterNode}, RegBH};
static RegisterOpExpression register_CH = {{RegisterNode}, RegCH};
static RegisterOpExpression register_DH = {{RegisterNode}, RegDH};

static RegisterOpExpression register_DIL = {{RegisterNode}, RegDIL};
static RegisterOpExpression register_SIL = {{RegisterNode}, RegSIL};
static RegisterOpExpression register_BPL = {{RegisterNode}, RegBPL};
static RegisterOpExpression register_SPL = {{RegisterNode}, RegSPL};
static RegisterOpExpression register_R8L = {{RegisterNode}, RegR8L};
static RegisterOpExpression register_R9L = {{RegisterNode}, RegR9L};
static RegisterOpExpression register_R10L = {{RegisterNode}, RegR10L};
static RegisterOpExpression register_R11L = {{RegisterNode}, RegR11L};
static RegisterOpExpression register_R12L = {{RegisterNode}, RegR12L};
static RegisterOpExpression register_R13L = {{RegisterNode}, RegR13L};
static RegisterOpExpression register_R14L = {{RegisterNode}, RegR14L};
static RegisterOpExpression register_R15L = {{RegisterNode}, RegR15L};

/* 16-bit registers */
static RegisterOpExpression register_AX = {{RegisterNode}, RegAX};
static RegisterOpExpression register_BX = {{RegisterNode}, RegBX};
static RegisterOpExpression register_CX = {{RegisterNode}, RegCX};
static RegisterOpExpression register_DX = {{RegisterNode}, RegDX};
static RegisterOpExpression register_SI = {{RegisterNode}, RegSI};
static RegisterOpExpression register_DI = {{RegisterNode}, RegDI};
static RegisterOpExpression register_BP = {{RegisterNode}, RegBP};
static RegisterOpExpression register_SP = {{RegisterNode}, RegSP};

static RegisterOpExpression register_R8W = {{RegisterNode}, RegR8W};
static RegisterOpExpression register_R9W = {{RegisterNode}, RegR9W};
static RegisterOpExpression register_R10W = {{RegisterNode}, RegR10W};
static RegisterOpExpression register_R11W = {{RegisterNode}, RegR11W};
static RegisterOpExpression register_R12W = {{RegisterNode}, RegR12W};
static RegisterOpExpression register_R13W = {{RegisterNode}, RegR13W};
static RegisterOpExpression register_R14W = {{RegisterNode}, RegR14W};
static RegisterOpExpression register_R15W = {{RegisterNode}, RegR15W};

/* 32-bit registers */
static RegisterOpExpression register_EAX = {{RegisterNode}, RegEAX};
static RegisterOpExpression register_EBX = {{RegisterNode}, RegEBX};
static RegisterOpExpression register_ECX = {{RegisterNode}, RegECX};
static RegisterOpExpression register_EDX = {{RegisterNode}, RegEDX};
static RegisterOpExpression register_ESI = {{RegisterNode}, RegESI};
static RegisterOpExpression register_EDI = {{RegisterNode}, RegEDI};
static RegisterOpExpression register_EBP = {{RegisterNode}, RegEBP};
static RegisterOpExpression register_ESP = {{RegisterNode}, RegESP};

static RegisterOpExpression register_R8D = {{RegisterNode}, RegR8D};
static RegisterOpExpression register_R9D = {{RegisterNode}, RegR9D};
static RegisterOpExpression register_R10D = {{RegisterNode}, RegR10D};
static RegisterOpExpression register_R11D = {{RegisterNode}, RegR11D};
static RegisterOpExpression register_R12D = {{RegisterNode}, RegR12D};
static RegisterOpExpression register_R13D = {{RegisterNode}, RegR13D};
static RegisterOpExpression register_R14D = {{RegisterNode}, RegR14D};
static RegisterOpExpression register_R15D = {{RegisterNode}, RegR15D};

/* 64-bit registers */
static RegisterOpExpression register_RAX = {{RegisterNode}, RegRAX};
static RegisterOpExpression register_RBX = {{RegisterNode}, RegRBX};
static RegisterOpExpression register_RCX = {{RegisterNode}, RegRCX};
static RegisterOpExpression register_RDX = {{RegisterNode}, RegRDX};
static RegisterOpExpression register_RSI = {{RegisterNode}, RegRSI};
static RegisterOpExpression register_RDI = {{RegisterNode}, RegRDI};
static RegisterOpExpression register_RBP = {{RegisterNode}, RegRBP};
static RegisterOpExpression register_RSP = {{RegisterNode}, RegRSP};
static RegisterOpExpression register_R8 = {{RegisterNode}, RegR8};
static RegisterOpExpression register_R9 = {{RegisterNode}, RegR9};
static RegisterOpExpression register_R10 = {{RegisterNode}, RegR10};
static RegisterOpExpression register_R11 = {{RegisterNode}, RegR11};
static RegisterOpExpression register_R12 = {{RegisterNode}, RegR12};
static RegisterOpExpression register_R13 = {{RegisterNode}, RegR13};
static RegisterOpExpression register_R14 = {{RegisterNode}, RegR14};
static RegisterOpExpression register_R15 = {{RegisterNode}, RegR15};

/* Segment registers */
static RegisterOpExpression register_CS = {{RegisterNode}, RegCS};
static RegisterOpExpression register_DS = {{RegisterNode}, RegDS};
static RegisterOpExpression register_ES = {{RegisterNode}, RegES};
static RegisterOpExpression register_FS = {{RegisterNode}, RegFS};
static RegisterOpExpression register_GS = {{RegisterNode}, RegGS};
static RegisterOpExpression register_SS = {{RegisterNode}, RegSS};

/* Program status */
static RegisterOpExpression register_EFLAGS = {{RegisterNode}, RegEFLAGS};
static RegisterOpExpression register_RFLAGS = {{RegisterNode}, RegRFLAGS};

/* Instruction Pointer */
static RegisterOpExpression register_EIP = {{RegisterNode}, RegEIP};
static RegisterOpExpression register_RIP = {{RegisterNode}, RegRIP};

/* Define the number of general purpose registers defined for the given
 * subarchitecture.
 */
#if NACL_TARGET_SUBARCH == 64
#define REGISTER_TABLE_SIZE 16
#else
#define REGISTER_TABLE_SIZE 8
#endif

/* Define the available 8-bit registers, for the given subarchitecture. */
static RegisterOpExpression* RegisterTable8[REGISTER_TABLE_SIZE] = {
  &register_AL,
  &register_CL,
  &register_DL,
  &register_BL,
#if NACL_TARGET_SUBARCH == 64
  &register_SPL,
  &register_BPL,
  &register_SIL,
  &register_DIL,
  &register_R8L,
  &register_R9L,
  &register_R10L,
  &register_R11L,
  &register_R12L,
  &register_R13L,
  &register_R14L,
  &register_R15L
#else
  &register_AH,
  &register_CH,
  &register_DH,
  &register_BH
#endif
};

/* Define the available 16-bit registers, for the given subarchitecture. */
static RegisterOpExpression* RegisterTable16[REGISTER_TABLE_SIZE] = {
  &register_AX,
  &register_CX,
  &register_DX,
  &register_BX,
  &register_SP,
  &register_BP,
  &register_SI,
  &register_DI,
#if NACL_TARGET_SUBARCH == 64
  &register_R8W,
  &register_R9W,
  &register_R10W,
  &register_R11W,
  &register_R12W,
  &register_R13W,
  &register_R14W,
  &register_R15W,
#endif
};

/* Define the available 32-bit registers, for the given subarchitecture. */
static RegisterOpExpression* RegisterTable32[REGISTER_TABLE_SIZE] = {
  &register_EAX,
  &register_ECX,
  &register_EDX,
  &register_EBX,
  &register_ESP,
  &register_EBP,
  &register_ESI,
  &register_EDI,
#if NACL_TARGET_SUBARCH == 64
  &register_R8D,
  &register_R9D,
  &register_R10D,
  &register_R11D,
  &register_R12D,
  &register_R13D,
  &register_R14D,
  &register_R15D
#endif
};


/* Define the available 32-bit registers, for the given subarchitecture. */
#if NACL_TARGET_SUBARCH == 64
static RegisterOpExpression* RegisterTable64[REGISTER_TABLE_SIZE] = {
  &register_RAX,
  &register_RCX,
  &register_RDX,
  &register_RBX,
  &register_RSP,
  &register_RBP,
  &register_RSI,
  &register_RDI,
  &register_R8,
  &register_R9,
  &register_R10,
  &register_R11,
  &register_R12,
  &register_R13,
  &register_R14,
  &register_R15
};
#endif

/* Define a type corresponding to the arrays RegisterTable8,
 * RegisterTable16, RegisterTable32, and RegisterTable64.
 */
typedef RegisterOpExpression* RegisterTableGroup[REGISTER_TABLE_SIZE];

/* Define the set of available registers, categorized by size. */
static RegisterTableGroup* RegisterTable[] = {
  &RegisterTable8,
  &RegisterTable16,
  &RegisterTable32,
#if NACL_TARGET_SUBARCH == 64
  &RegisterTable64
#endif
  /* TODO(karl) Add MMX registers etc. */
};


/* Define possible register categories. */
typedef enum {
  RegSize8,
  RegSize16,
  RegSize32,
  RegSize64
} RegKind;

/* Given the given register category, and the corresponding index, return
 * the appropriate register.
 */
RegisterOpExpression* LookupRegister(RegKind kind, int reg_index) {
  RegisterOpExpression* r;
  DEBUG(printf("lookup(%u, %u)\n", kind, reg_index));
  r = (*(RegisterTable[kind]))[reg_index];
  DEBUG(PrintOpExpression(stdout, (OpExpressionNode*) r));
  DEBUG(putc('\n', stdout));
  return r;
}

/* Given we want to translate operand (of the form G_Operand),
 * for the given register index, generate the corresponding register
 * expression.
 */
static RegisterOpExpression* TranslateRegister(
    NcInstState* state, Operand* operand, int reg_index) {
  /* First compute the size register to look up */
  RegKind reg_kind = RegSize32;
  DEBUG(printf("reg_index = %0d\n", reg_index));
  /* TODO(karl) Is it necessary to have an oeprand override. */
  if (operand->kind >= Gb_Operand && operand->kind <= Go_Operand) {
    reg_kind = (RegKind) operand->kind - Gb_Operand;
  } else if (state->opcode->flags & OCF(OperandSize_b)) {
    reg_kind = RegSize8;
  } else if (state->operand_size == 4) {
    reg_kind = RegSize32;
  } else if (state->operand_size == 2) {
    reg_kind = RegSize16;
  } else if (state->operand_size == 8) {
    reg_kind = RegSize64;
  }
  return LookupRegister(reg_kind, reg_index);
}

/* For the given instruction state, and the corresponding 3-bit specification
 * of a register, update it to a 4-bit specification, based on the REX.R bit.
 */
int GetRexRRegister(NcInstState* state, int reg) {
  if (NACL_TARGET_SUBARCH == 64 && (state->rexprefix & 0x4)) {
    reg += 8;
  }
  return reg;
}

/* For the given instruction state, and the corresponding 3-bit specification
 * of a register, update it to a 4-bit specification, based on the REX.X bit.
 */
int GetRexXRegister(NcInstState* state, int reg) {
  if (NACL_TARGET_SUBARCH == 64 && (state->rexprefix & 0x2)) {
    reg += 8;
  }
  return reg;
}

/* For the given instruvtion state, and the corresponding 3-bit specification
 * of a register, update it to a 4-bit specification, based on the REX.B bit.
 */
int GetRexBRegister(NcInstState* state, int reg) {
  if (NACL_TARGET_SUBARCH == 64 && (state->rexprefix & 0x1)) {
    reg += 8;
  }
  return reg;
}

/* Return the general purpose register associated with the modrm.reg
 * field.
 */
int GetGenRegRegister(NcInstState* state) {
  return GetRexRRegister(state, modrm_reg(state->modrm));
}

/* Return the general purpose register associated with the modrm.rm
 * field.
 */
int GetGenRmRegister(NcInstState* state) {
  return GetRexBRegister(state, modrm_rm(state->modrm));
}

/* Convert the displacements bytes of the parsed instruction into
 * the corresponding expression node.
 */
ImmediateOpExpression* TranslateDisp(NcInstState* state) {

  /* First compute the displacement value. */
  uint32_t value = 0;
  int i;
  for (i = 0; i < state->num_disp_bytes; ++i) {
    value += (state->mpc[state->first_disp_byte + i] << i);
  }

  /* Now convert the displacement value into an immediate expression. */
  switch (state->num_disp_bytes) {
    case 1:
      return (ImmediateOpExpression*)
          CreateImmediate1Op((uint8_t) value, &state->workspace);
    case 2:
      return (ImmediateOpExpression*)
          CreateImmediate2Op((uint16_t) value, &state->workspace);
    case 4:
      return (ImmediateOpExpression*)
          CreateImmediate4Op(value, &state->workspace);
    default:
      /* This should not be reachable! */
      assert(0);
      break;
  }
  /* NOT REACHED */
  return NULL;
}

/* Convert the immediate bytes of the parsed instruction into the
 * corresponding expression node.
 */
ImmediateOpExpression* TranslateImmediate(NcInstState* state) {
  /* First compute the immediate value. */
  uint32_t value = 0;
  int i;
  for (i = 0; i< state->num_imm_bytes; ++i) {
    value += (state->mpc[state->first_imm_byte + i] << i);
  }

  /* Now convert the immediate value into a corresponding expression. */
  switch (state->num_imm_bytes) {
    case 1:
      return (ImmediateOpExpression*)
          CreateImmediate1Op((uint8_t) value, &state->workspace);
    case 2:
      return (ImmediateOpExpression*)
          CreateImmediate2Op((uint16_t) value, &state->workspace);
    case 4:
      return (ImmediateOpExpression*)
          CreateImmediate4Op(value, &state->workspace);
    default:
      /* This should not be reachable. */
      assert(0);
      break;
  }
  /* NOT REACHED */
  return NULL;
}

/* Define the possible scaling factors */
static uint8_t sib_scale[4] = { 1, 2, 4, 8 };

/* Convert the value in the SIB byte to a corresponding memory offset. */
OpExpressionNode* TranslateSib(NcInstState* state) {
  /* TODO(karl) Add effect of REX prefix to this (see table 2-5
   * of Intel Manual).
   */
  int index = sib_index(state->sib);
  int base = sib_base(state->sib);
  ImmediateOpExpression* disp = NULL;
  RegKind kind = NACL_TARGET_SUBARCH == 64 ? RegSize64 : RegSize32;
  RegisterOpExpression* r1 = NULL;
  RegisterOpExpression* r2 = NULL;
  uint8_t scale = 1;
  if (0x5 == base) {
    switch (modrm_mod(state->modrm)) {
      case 0:
        break;
      case 1:
      case 2:
        r1 = &register_EBP;
        break;
      default:
        /* This should not happen, return undefined. */
        return (OpExpressionNode*) &undefined_op;
    }
  } else {
    r1 = LookupRegister(kind, GetRexBRegister(state, sib_base(state->sib)));
  }
  if (0x4 != index) {
    r2 = LookupRegister(kind, GetRexXRegister(state, index));
    scale = sib_scale[sib_ss(state->sib)];
  }
  if (state->num_disp_bytes > 0) {
    disp = TranslateDisp(state);
  }
  if (r1 == NULL) {
    if (disp == NULL) {
      return (OpExpressionNode*) &undefined_op;
    } else {
      return (OpExpressionNode*) disp;
    }
  } else {
    return (OpExpressionNode*)
        CreateMemoryOffsetOp(disp, r1, r2, scale, &state->workspace);
  }
}

/* Get the Effective address in the mod/rm byte, if the modrm.mod field
 * is 00. Operand index is the index of the opcode operand being translated.
 */
OpExpressionNode* TranslateMod00EffectiveAddress(
    NcInstState* state, int operand_index) {
  /* TODO(karl) how does REX prefix effect this. */
  switch(modrm_rm(state->modrm)) {
    case 4:
      return TranslateSib(state);
    case 5:
      return (OpExpressionNode*) TranslateDisp(state);
    default: {
      return (OpExpressionNode*)
          CreateMemoryOffsetOp(
              (ImmediateOpExpression*) CreateImmediate1Op(0, &state->workspace),
              TranslateRegister(state,
                                &state->opcode->operands[operand_index],
                                GetGenRmRegister(state)),
              NULL,
              1,
              &state->workspace);
    }
  }
  return (OpExpressionNode*) &undefined_op;
}

/* Get the Effective address in the mod/rm byte, if the modrm.mod field
 * is 01. Operand index is the index of the opcode operand being translated.
 */
OpExpressionNode* TranslateMod01EffectiveAddress(
    NcInstState* state, int operand_index) {
  /* TODO(karl) how does REX prefix effect this */
  switch (modrm_rm(state->modrm)) {
    case 4:
      return TranslateSib(state);
      break;
    default: {
      return (OpExpressionNode*)
          CreateMemoryOffsetOp(
              TranslateDisp(state),
              TranslateRegister(state,
                                &state->opcode->operands[operand_index],
                                GetGenRmRegister(state)),
              NULL,
              1,
              &state->workspace);
    }
  }
  return (OpExpressionNode*) &undefined_op;
}

/* Get the Effective address in the mod/rm byte, if the modrm.mod field
 * is 10. Operand index is the index of the opcode operand being translated.
 */
OpExpressionNode* TranslateMod10EffectiveAddress(
    NcInstState* state, int operand_index) {
  /* TODO(karl) how does REX prefix effect this */
  switch (modrm_rm(state->modrm)) {
    case 4:
      return TranslateSib(state);
      break;
    default:
      return (OpExpressionNode*)
          CreateMemoryOffsetOp(
              TranslateDisp(state),
              TranslateRegister(state,
                                &state->opcode->operands[operand_index],
                                GetGenRmRegister(state)),
              NULL,
              1,
              &state->workspace);
  }
  return (OpExpressionNode*) &undefined_op;
}

/* Get the Effective address in the mod/rm byte, if the modrm.mod field
 * is 11. Operand index is the index of the opcode operand being translated.
 */
OpExpressionNode* TranslateMod11EffectiveAddress(
    NcInstState* state, int operand_index) {
  return (OpExpressionNode*)
      TranslateRegister(state,
                        &state->opcode->operands[operand_index],
                        GetGenRmRegister(state));
}

/* Trans the given (indexed) operand to the corresponding expression tree. */
OpExpressionNode* TranslateOperand(
    NcInstState* state,
    int operand_index) {
  OpExpressionNode* subtree = (OpExpressionNode*) &undefined_op;
  Operand* operand = &state->opcode->operands[operand_index];
  switch (operand->kind) {
    case E_Operand:
    case Eb_Operand:
    case Ew_Operand:
    case Ev_Operand:
    case Eo_Operand:
      switch(modrm_mod(state->modrm)) {
        case 0:
          subtree = TranslateMod00EffectiveAddress(state, operand_index);
          break;
        case 1:
          subtree = TranslateMod01EffectiveAddress(state, operand_index);
          break;
        case 2:
          subtree = TranslateMod10EffectiveAddress(state, operand_index);
          break;
        case 3:
          subtree = TranslateMod11EffectiveAddress(state, operand_index);
          break;
        default:
          break;
      }
      break;
    case G_Operand:
    case Gb_Operand:
    case Gw_Operand:
    case Gv_Operand:
    case Go_Operand:
      subtree = (OpExpressionNode*)
          TranslateRegister(state,
                            &state->opcode->operands[operand_index],
                            GetGenRegRegister(state));
      break;
    case I_Operand:
    case Ib_Operand:
    case Iw_Operand:
    case Iv_Operand:
    case Io_Operand:
      subtree = (OpExpressionNode*) TranslateImmediate(state);
      break;
    case RegUnknown:
      subtree = (OpExpressionNode*) &register_Unknown;
      break;
    case RegAL:
      subtree = (OpExpressionNode*) &register_AL;
      break;
    case RegBL:
      subtree = (OpExpressionNode*) &register_BL;
      break;
    case RegCL:
      subtree = (OpExpressionNode*) &register_CL;
      break;
    case RegDL:
      subtree = (OpExpressionNode*) &register_DL;
      break;
    case RegAH:
      subtree = (OpExpressionNode*) &register_AH;
      break;
    case RegBH:
      subtree = (OpExpressionNode*) &register_BH;
      break;
    case RegCH:
      subtree = (OpExpressionNode*) &register_CH;
      break;
    case RegDH:
      subtree = (OpExpressionNode*) &register_DH;
      break;
    case RegDIL:
      subtree = (OpExpressionNode*) &register_DIL;
      break;
    case RegSIL:
      subtree = (OpExpressionNode*) &register_SIL;
      break;
    case RegBPL:
      subtree = (OpExpressionNode*) &register_BPL;
      break;
    case RegSPL:
      subtree = (OpExpressionNode*) &register_SPL;
      break;
    case RegR8L:
      subtree = (OpExpressionNode*) &register_R8L;
      break;
    case RegR9L:
      subtree = (OpExpressionNode*) &register_R9L;
      break;
    case RegR10L:
      subtree = (OpExpressionNode*) &register_R10L;
      break;
    case RegR11L:
      subtree = (OpExpressionNode*) &register_R11L;
      break;
    case RegR12L:
      subtree = (OpExpressionNode*) &register_R12L;
      break;
    case RegR13L:
      subtree = (OpExpressionNode*) &register_R13L;
      break;
    case RegR14L:
      subtree = (OpExpressionNode*) &register_R14L;
      break;
    case RegR15L:
      subtree = (OpExpressionNode*) &register_R15L;
      break;
    case RegAX:
      subtree = (OpExpressionNode*) &register_AX;
      break;
    case RegBX:
      subtree = (OpExpressionNode*) &register_BX;
      break;
    case RegCX:
      subtree = (OpExpressionNode*) &register_CX;
      break;
    case RegDX:
      subtree = (OpExpressionNode*) &register_DX;
      break;
    case RegSI:
      subtree = (OpExpressionNode*) &register_SI;
      break;
    case RegDI:
      subtree = (OpExpressionNode*) &register_DI;
      break;
    case RegBP:
      subtree = (OpExpressionNode*) &register_BP;
      break;
    case RegSP:
      subtree = (OpExpressionNode*) &register_SP;
      break;
    case RegR8W:
      subtree = (OpExpressionNode*) &register_R8W;
      break;
    case RegR9W:
      subtree = (OpExpressionNode*) &register_R9W;
      break;
    case RegR10W:
      subtree = (OpExpressionNode*) &register_R10W;
      break;
    case RegR11W:
      subtree = (OpExpressionNode*) &register_R11W;
      break;
    case RegR12W:
      subtree = (OpExpressionNode*) &register_R12W;
      break;
    case RegR13W:
      subtree = (OpExpressionNode*) &register_R13W;
      break;
    case RegR14W:
      subtree = (OpExpressionNode*) &register_R14W;
      break;
    case RegR15W:
      subtree = (OpExpressionNode*) &register_R15W;
      break;
    case RegEAX:
      subtree = (OpExpressionNode*) &register_EAX;
      break;
    case RegEBX:
      subtree = (OpExpressionNode*) &register_EBX;
      break;
    case RegECX:
      subtree = (OpExpressionNode*) &register_ECX;
      break;
    case RegEDX:
      subtree = (OpExpressionNode*) &register_EDX;
      break;
    case RegESI:
      subtree = (OpExpressionNode*) &register_ESI;
      break;
    case RegEDI:
      subtree = (OpExpressionNode*) &register_EDI;
      break;
    case RegEBP:
      subtree = (OpExpressionNode*) &register_EBP;
      break;
    case RegESP:
      subtree = (OpExpressionNode*) &register_ESP;
      break;
    case RegR8D:
      subtree = (OpExpressionNode*) &register_R8D;
      break;
    case RegR9D:
      subtree = (OpExpressionNode*) &register_R9D;
      break;
    case RegR10D:
      subtree = (OpExpressionNode*) &register_R10D;
      break;
    case RegR11D:
      subtree = (OpExpressionNode*) &register_R11D;
      break;
    case RegR12D:
      subtree = (OpExpressionNode*) &register_R12D;
      break;
    case RegR13D:
      subtree = (OpExpressionNode*) &register_R13D;
      break;
    case RegR14D:
      subtree = (OpExpressionNode*) &register_R14D;
      break;
    case RegR15D:
      subtree = (OpExpressionNode*) &register_R15D;
      break;
    case RegCS:
      subtree = (OpExpressionNode*) &register_CS;
      break;
    case RegDS:
      subtree = (OpExpressionNode*) &register_DS;
      break;
    case RegSS:
      subtree = (OpExpressionNode*) &register_SS;
      break;
    case RegES:
      subtree = (OpExpressionNode*) &register_ES;
      break;
    case RegFS:
      subtree = (OpExpressionNode*) &register_FS;
      break;
    case RegGS:
      subtree = (OpExpressionNode*) &register_GS;
      break;
    case RegEFLAGS:
      subtree = (OpExpressionNode*) &register_EFLAGS;
      break;
    case RegRFLAGS:
      subtree = (OpExpressionNode*) &register_RFLAGS;
      break;
    case RegEIP:
      subtree = (OpExpressionNode*) &register_EIP;
      break;
    case RegRIP:
      subtree = (OpExpressionNode*) &register_RIP;
      break;
    case RegRAX:
      subtree = (OpExpressionNode*) &register_RAX;
      break;
    case RegRBX:
      subtree = (OpExpressionNode*) &register_RBX;
      break;
    case RegRCX:
      subtree = (OpExpressionNode*) &register_RCX;
      break;
    case RegRDX:
      subtree = (OpExpressionNode*) &register_RDX;
      break;
    case RegRSI:
      subtree = (OpExpressionNode*) &register_RSI;
      break;
    case RegRDI:
      subtree = (OpExpressionNode*) &register_RDI;
      break;
    case RegRBP:
      subtree = (OpExpressionNode*) &register_RBP;
      break;
    case RegRSP:
      subtree = (OpExpressionNode*) &register_RSP;
      break;
    case RegR8:
      subtree = (OpExpressionNode*) &register_R8;
      break;
    case RegR9:
      subtree = (OpExpressionNode*) &register_R9;
      break;
    case RegR10:
      subtree = (OpExpressionNode*) &register_R10;
      break;
    case RegR11:
      subtree = (OpExpressionNode*) &register_R11;
      break;
    case RegR12:
      subtree = (OpExpressionNode*) &register_R12;
      break;
    case RegR13:
      subtree = (OpExpressionNode*) &register_R13;
      break;
    case RegR14:
      subtree = (OpExpressionNode*) &register_R14;
      break;
    case RegR15:
      subtree = (OpExpressionNode*) &register_R15;
      break;
    case RegRESP:
      subtree = (OpExpressionNode*)
        (state->address_size == 64 ? &register_RSP : &register_ESP);
      break;
    case RegREAX:
      switch (state->operand_size) {
        case 1:
          subtree = (OpExpressionNode*) &register_AL;
          break;
        case 2:
          subtree = (OpExpressionNode*) &register_AX;
          break;
        case 8:
          subtree = (OpExpressionNode*) &register_RAX;
          break;
        case 4:
        default:
          subtree = (OpExpressionNode*) &register_EAX;
          break;
      }
      break;
      /* TODO(karl) fill in the rest of the possibilities of type
       * OperandKind, or remove them if not needed.
       */
    default:
      /* Give up, use the default of undefined. */
      break;
  }
  return (OpExpressionNode*)
      CreateOperandOp(operand_index,
                      operand,
                      subtree,
                      &state->workspace);
}

/* Translate the matched opcode (instruction) of the given state, into
 * the corresponding expression tree.
 */
OpExpressionNode* TranslateOpcode(NcInstState* state) {
  if (InstUndefined == state->opcode->insttype) {
    return (OpExpressionNode*) &undefined_op;
  } else {
    int i;
    OpcodeOpExpression* oc = CreateOpcodeOp(state->opcode, &state->workspace);
    for (i = 0; i < state->opcode->num_operands; ++i) {
      Operand* op = &state->opcode->operands[i];
      if (0 == (op->flags & OPF(OperandExtendsOpcode))) {
        OpExpressionNode* n = TranslateOperand(state, i);
        if (NULL != n) {
          oc->operands[oc->number_operands++] = n;
        }
      }
    }
    return (OpExpressionNode*) oc;
  }
}
