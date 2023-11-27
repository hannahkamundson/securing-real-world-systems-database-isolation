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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "native_client/src/trusted/validator_x86/ncop_exps.h"
#include "native_client/src/trusted/validator_x86/nc_inst_state.h"
#include "native_client/src/shared/utils/types.h"

/* Print out an undefined expression tree to the given file. */
static void PrintUndefinedOp(FILE* file, UndefinedOpExpression* exp) {
  fprintf(file, "undefined");
}

/* Print out a one byte immediate value to the given file. */
static void PrintImmediate1Op(FILE* file, Immediate1OpExpression* exp) {
  fprintf(file, "0x%02x", exp->value);
}

/* Print out a two byte immediate value to the given file. */
static void PrintImmediate2Op(FILE* file, Immediate2OpExpression* exp) {
  fprintf(file, "0x%04x", exp->value);
}

/* Print out a four byte immediate value to the given file. */
static void PrintImmediate4Op(FILE* file, Immediate4OpExpression* exp) {
  fprintf(file, "0x%08x", exp->value);
}

/* Print out an immediate value to the given file. */
static void PrintImmediateOp(FILE* file, ImmediateOpExpression* exp) {
  switch (exp->width) {
    case 1:
      PrintImmediate1Op(file, (Immediate1OpExpression*) exp);
      break;
    case 2:
      PrintImmediate2Op(file, (Immediate2OpExpression*) exp);
      break;
    case 4:
      PrintImmediate4Op(file, (Immediate4OpExpression*) exp);
      break;
    default:
      assert(0);
  }
}

/* Print out the name of the register to the given file. */
static void PrintRegisterOp(FILE* file, RegisterOpExpression* reg) {
  fprintf(file, "%s", OperandKindName(reg->kind));
}

/* Print out a segment address to the given file. */
static void PrintSegmentAddressOp(
    FILE* file,
    SegmentAddressOpExpression* addr) {
  fprintf(file, "seg(");
  PrintOpExpression(file, (OpExpressionNode*) addr->base);
  putc(':', file);
  PrintOpExpression(file, (OpExpressionNode*) addr->offset);
  putc(')', file);
}

/* Print out a memory offset to the given file. */
static void PrintMemoryOffsetOp(
    FILE* file,
    MemoryOffsetOpExpression* moffset) {
  fprintf(file, "mem(");
  PrintOpExpression(file, (OpExpressionNode*) moffset->base);
  fprintf(file, ", ");
  PrintOpExpression(file, (OpExpressionNode*) moffset->index);
  fprintf(file, ", ");
  PrintOpExpression(file, (OpExpressionNode*) moffset->scale);
  fprintf(file, ", ");
  PrintOpExpression(file, (OpExpressionNode*) moffset->displacement);
  fprintf(file, ")");
}

/* Print out the given operand to the given file. */
static void PrintOperandOp(FILE* file, OperandOpExpression* op) {
  fprintf(file, "%d -> ", op->operand_index);
  PrintOpExpression(file, op->subtree);
}

/* Print out the given opcode (instruction) expression to the given file. */
static void PrintOpcodeOp(FILE* file, OpcodeOpExpression* op) {
  int i;
  fprintf(file, "%s(", InstMnemonicName(op->opcode->name));
  for (i = 0; i < op->number_operands; ++i) {
    if (i > 0) {
      fprintf(file, ", ");
    }
    PrintOpExpression(file, op->operands[i]);
  }
  fprintf(file, ")");
}

void PrintOpExpression(FILE* file, OpExpressionNode* node) {
  if (NULL == node) {
    fprintf(file, "null");
  } else {
    switch (node->kind) {
      case UndefinedTree:
        PrintUndefinedOp(file, (UndefinedOpExpression*) node);
        break;
      case ImmediateNode:
        PrintImmediateOp(file, (ImmediateOpExpression*) node);
        break;
      case RegisterNode:
        PrintRegisterOp(file, (RegisterOpExpression*) node);
        break;
      case SegmentAddress:
        PrintSegmentAddressOp(file, (SegmentAddressOpExpression*) node);
        break;
      case MemoryOffset:
        PrintMemoryOffsetOp(file, (MemoryOffsetOpExpression*) node);
        break;
      case OpcodeNode:
        PrintOpcodeOp(file, (OpcodeOpExpression*) node);
        break;
      case OperandNode:
        PrintOperandOp(file, (OperandOpExpression*) node);
        break;
      default:
        assert(0);
        break;
    }
  }
}

/* Print the characters in the given string using lower case. */
static void PrintLower(FILE* file, char* str) {
  while (*str) {
    putc(tolower(*str), file);
    ++str;
  }
}

/* Prints the disassembled text for the given expression node to
 * the given file.
 */
static void PrintDisassembled(FILE* file, OpExpressionNode* exp);

/* Returns true if the given immediate expression matches the given value.
 */
static Bool IsImmediate(ImmediateOpExpression* imm, uint32_t value) {
  switch (imm->width) {
    case 1:
      return ((Immediate1OpExpression*) imm)->value == value;
    case 2:
      return ((Immediate2OpExpression*) imm)->value == value;
    case 4:
      return ((Immediate4OpExpression*) imm)->value == value;
    default:
      break;
  }
  return FALSE;
}

/* Disassembles the given immediate value to the given file. */
static void PrintDisassembledImmediate(FILE* file, ImmediateOpExpression* imm) {
  switch (imm->width) {
    case 1:
      fprintf(file, "0x%x", ((Immediate1OpExpression*) imm)->value);
      break;
    case 2:
      fprintf(file, "0x%x", ((Immediate2OpExpression*) imm)->value);
      break;
    case 4:
      fprintf(file, "0x%x", ((Immediate4OpExpression*) imm)->value);
      break;
    default:
      fprintf(file, "???");
      break;
  }
}

/* Disassembles the given register expression to the given file. */
static void PrintDisassembledRegister(FILE* file, RegisterOpExpression* reg) {
  const char* name = OperandKindName(reg->kind);
  char* str = strstr(name, "Reg");
  putc('%', file);
  PrintLower(file, str == NULL ? (char*) name : str + strlen("Reg"));
}

/* Disassembles the given memory offset to the given file. */
static void PrintDisassembledMemoryOffset(
    FILE* file,
    MemoryOffsetOpExpression* moffset) {
  if (moffset->displacement != NULL && !IsImmediate(moffset->displacement, 0)) {
    PrintDisassembled(file, (OpExpressionNode*) moffset->displacement);
  }
  putc('(', file);
  PrintDisassembled(file, (OpExpressionNode*) moffset->base);
  if (moffset->index != NULL) {
    fprintf(file, ", ");
    PrintDisassembled(file, (OpExpressionNode*) moffset->index);
  }
  if (!IsImmediate((ImmediateOpExpression*) moffset->scale, 1)) {
    fprintf(file, ", ");
    PrintDisassembled(file, (OpExpressionNode*) moffset->scale);
  }
  putc(')', file);
}

/* Disassembles the given operand to the given file. */
static void PrintDisassembledOperand(FILE* file, OperandOpExpression* operand) {
  PrintDisassembled(file, operand->subtree);
}

/* Disassembles the given opcode to the given file. */
static void PrintDisassembledOpcode(FILE* file, OpcodeOpExpression* opcode) {
  int i;
  int count = 0;
  PrintLower(file, (char*) InstMnemonicName(opcode->opcode->name));
  putc(' ', file);
  for (i = 0; i < opcode->number_operands; ++i) {
    Bool print_value = TRUE;
    OpExpressionNode* op = opcode->operands[i];
    if (OperandNode == op->kind) {
      OperandOpExpression* operand = (OperandOpExpression*) op;
      if (operand->operand->flags & OPF(OpImplicit)) {
        print_value = FALSE;
      }
    }
    if (print_value) {
      if (count > 0) {
        fprintf(file, ", ");
      }
      PrintDisassembled(file, op);
      ++count;
    }
  }
}

/* Disassembles the given sesgment address to the given file. */
static void PrintDisassembledSegmentAddress(
    FILE* file,
    SegmentAddressOpExpression* segment) {
  PrintDisassembled(file, (OpExpressionNode*) segment->base);
  putc(':', file);
  PrintDisassembled(file, (OpExpressionNode*) segment->offset);
}

/* Disassembles the given instruction expression and prints to the
 * given file.
 */
static void PrintDisassembled(FILE* file, OpExpressionNode* exp) {
  switch (exp->kind) {
    case ImmediateNode:
      PrintDisassembledImmediate(file, (ImmediateOpExpression*) exp);
      break;
    case RegisterNode:
      PrintDisassembledRegister(file, (RegisterOpExpression*) exp);
      break;
    case SegmentAddress:
      PrintDisassembledSegmentAddress(file, (SegmentAddressOpExpression*) exp);
      break;
    case MemoryOffset:
      PrintDisassembledMemoryOffset(file, (MemoryOffsetOpExpression*) exp);
      break;
    case OpcodeNode:
      PrintDisassembledOpcode(file, (OpcodeOpExpression*) exp);
      break;
    case OperandNode:
      PrintDisassembledOperand(file, (OperandOpExpression*) exp);
      break;
    default:
      fprintf(file, "???");
      break;
  }
}

void NcInstStatePrintDisassembled(FILE* file, NcInstState* state) {
  /* Print out the address and sequence of bytes that define the instruction. */
  int i;
  int length = NcInstStateLength(state);
  fprintf(file, "%"PRIElf_AddrAll": ", NcInstStateVpc(state));
  for (i = 0; i < length; ++i) {
      fprintf(file, "%02x ", NcInstStateByte(state, i));
  }
  for (i = length; i < MAX_BYTES_PER_X86_INSTRUCTION; ++i) {
    fprintf(file, "   ");
  }
  /* Print out the assembly instruction it disassembles to. */
  PrintDisassembled(file, NcInstStateExpression(state));
  /* Print out if not allowed in native client (as a comment). */
  switch (NcInstStateOpcode(state)->insttype) {
  case NACLi_UNDEFINED:
  case NACLi_ILLEGAL:
  case NACLi_INVALID:
  case NACLi_SYSTEM:
    fprintf(file, "; *NACL Disallows!*");
    break;
  default:
    break;
  }
  putc('\n', file);
}
