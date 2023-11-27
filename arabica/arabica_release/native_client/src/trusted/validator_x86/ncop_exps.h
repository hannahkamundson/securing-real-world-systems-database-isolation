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
 * ncop-exps.h - Models x86 instructions an expression trees .
 */
#ifndef NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCOP_EXPS_H_
#define NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCOP_EXPS_H_

#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"

/* Defines the state used to match an instruction, while walking
 * instructions using the NcInstIter.
 */
struct NcInstState;

/* Define the set of possible expression node tree types. */
typedef enum {
  /* Translator was unable to translate the corresponding part of
   * the opcode instruction.
   */
  UndefinedTree,
  /* The node defines a literal immediate (or displacement) value. */
  ImmediateNode,
  /* The node represents a register access. */
  RegisterNode,
  /* The node defines a segment address. */
  SegmentAddress,
  /* The node defines a memory offset with displacement, base, index, and
   * scale.
   */
  MemoryOffset,
  /* Root node defining the arguments of the opcode. */
  OpcodeNode,
  /* Associates Opcode Operand with subtree. */
  OperandNode,
  /* Special marker defining the number of elements in this list. */
  OpExpressionKindSize,
} OpExpressionKind;

/* Define the (abstract) root class of all expression nodes, which define the
 * type of node it represents.
 */
typedef struct OpExpressionNode {
  OpExpressionKind kind;
} OpExpressionNode;

/* Define an undefined expression, which will be used to model parts of
 * an instruction we don't know how to model.
 */
typedef struct UndefinedOpExpression {
  OpExpressionNode base_class;    /* base_class.kind == UndefinedTree */
} UndefinedOpExpression;

/* Model immediate (i.e. literal unsigned) values. */
typedef struct ImmediateOpExpression {
  OpExpressionNode base_class;    /* base_class.kind == ImmediateNode */
  uint8_t width;                  /* number of bytes for immediate value */
} ImmediateOpExpression;

/* Model immediate values that use one byte. */
typedef struct Immediate1OpExpression {
  ImmediateOpExpression base_class;    /* base_class.width == 1 */
  uint8_t value;                       /* the byte value. */
} Immediate1OpExpression;

/* Model immediate values that use two bytes. */
typedef struct Immediate2OpExpression {
  ImmediateOpExpression base_class;   /* base_class.width == 2 */
  uint16_t value;                     /* the two byte value. */
} Immediate2OpExpression;

/* Model immediate values that use 4 bytes. */
typedef struct Immediate4OpExpression {
  ImmediateOpExpression base_class;   /* base_class.width == 4 */
  uint32_t value;                     /* the four byte value. */
} Immediate4OpExpression;

/* Model a register reference. */
typedef struct RegisterOpExpression {
  /* base_class.kind == RegisterNode */
  OpExpressionNode base_class;
  /* corresponding register kind. Note:
   * the name can be derived from the
   * corresponding name of this field.
   */
  OperandKind kind;
} RegisterOpExpression;

/* Model a segment address. */
typedef struct SegmentAddressOpExpression {
  OpExpressionNode base_class;        /* base_class.kind == SegmentAddress */
  RegisterOpExpression* base;         /* XX in XX:YY.*/
  OpExpressionNode* offset;           /* YY in XX:YY */
} SegmentAddressOpExpression;

/* Holds 'base + (index * scale) + displacement' */
typedef struct MemoryOffsetOpExpression {
  OpExpressionNode base_class;          /* base_class.kind == MemoryOffset */
  ImmediateOpExpression* displacement;  /* Displacement of calculation. */
  RegisterOpExpression* base;           /* Base of calculation. */
  RegisterOpExpression* index;          /* index of calculation, may be null */
  /* Scale of calculation. Modeled as an Immediate1OpExpression
   * with value of 1, 2, 4, or 8.
   */
  Immediate1OpExpression* scale;
} MemoryOffsetOpExpression;

/* Define the maximum number of operands that can be modeled in an
 * expression tree.
 */
#define MAX_OP_OPERANDS 3

/* Associate the given subtree with the corresponding operand
 * of the modeled opcode (instruction). This node is provided so
 * that one can access additional information about the subtree
 * (such as set/use information).
 */
typedef struct OperandOpExpression {
  OpExpressionNode base_class;          /* base_class.kind == OperandNode */
  int operand_index;                    /* index of operand in opcode */
  Operand* operand;                     /* operand defining expression */
  /* The subtree the operand is associated with. */
  OpExpressionNode* subtree;
} OperandOpExpression;

/* Model the opcode (instruction) as an expression tree */
typedef struct OpcodeOpExpression {
  OpExpressionNode base_class;          /* base_class.kind == OpcodeNode */
  /* The (instruction) opcode being modeled. */
  Opcode* opcode;
  uint8_t number_operands;              /* Number of operands (i.e. kids). */
  OpExpressionNode* operands[MAX_OP_OPERANDS];  /* The list of operands. */
} OpcodeOpExpression;

/* Print out the given expression tree to the given file. Mostly
 * available for debugging.
 */
void PrintOpExpression(FILE* file, OpExpressionNode* node);

/* Given an iterator state, print out a disassembled version of
 * the currently matched instruction to the given file.
 */
void NcInstStatePrintDisassembled(FILE* file, struct NcInstState* state);

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_VALIDATOR_X86_NCOP_EXPS_H_ */
