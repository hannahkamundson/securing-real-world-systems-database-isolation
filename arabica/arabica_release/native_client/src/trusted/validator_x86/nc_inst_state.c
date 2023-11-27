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
 * Defines an instruction (decoder), based on the current location of
 * the instruction iterator. The instruction decoder takes a list
 * of candidate opcode (instruction) patterns, and searches for the
 * first candidate that matches the requirements of the opcode pattern.
 */

#include <stdio.h>
#include <assert.h>

#include "native_client/src/trusted/validator_x86/nc_inst_state.h"
#include "native_client/src/trusted/validator_x86/nc_inst_iter.h"
#include "native_client/src/trusted/validator_x86/nc_inst_state_internal.h"
#include "native_client/src/trusted/validator_x86/nc_inst_trans.h"
#include "native_client/src/trusted/validator_x86/nc_segment.h"
#include "native_client/src/trusted/validator_x86/ncop_exps.h"
#include "native_client/src/trusted/validator_x86/ncopcode_desc.h"
#include "gen/native_client/src/trusted/validator_x86/nc_opcode_table.h"

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

/* Given the current location of the instruction iterator, initialize
 * the given state (to match).
 */
static void NcInstStateInit(NcInstIter* iter, NcInstState* state) {
  state->mpc = iter->segment->mbase + iter->index;
  state->vpc = iter->segment->vbase + iter->index;
  state->length = 0;
  state->length_limit = (uint8_t) (iter->segment->size - iter->index);
  state->num_prefix_bytes = 0;
  state->rexprefix = 0;
  state->prefix_mask = 0;
  state->opcode = NULL;
  state->opcode_exp = NULL;
  state->workspace.next_available = 0;
}

/* Computes the number of bytes defined for operands of the matched
 * instruction of the given state.
 */
static int ExtractOperandSize(NcInstState* state) {
  if (NACL_TARGET_SUBARCH == 64) {
    if ((state->rexprefix && state->rexprefix & 0x8) ||
        (state->opcode->flags & OCF(OperandSizeDefaultIs64))) {
      return 8;
    }
  }
  if (state->prefix_mask & kPrefixDATA16) {
    return 2;
  }
  return 4;
}

/* Computes the number of bits defined for addresses of the matched
 * instruction of the given state.
 */
static int ExtractAddressSize(NcInstState* state) {
  if (NACL_TARGET_SUBARCH == 64) {
    return (state->prefix_mask & kPrefixADDR16) ? 32 : 64;
  } else {
    return (state->prefix_mask & kPrefixADDR16) ? 16 : 32;
  }
}

/* Match any prefix bytes that can be associated with the instruction
 * currently being matched.
 */
static Bool ConsumePrefixBytes(NcInstState* state) {
  uint8_t next_byte;
  int i;
  uint32_t prefix_form;
  int lock_index = -1;
  int rex_index = -1;
  for (i = 0; i < kMaxPrefixBytes; ++i) {
    /* Quit early if no more bytes in segment. */
    if (state->length >= state->length_limit) break;

    /* Look up the corresponding prefix bit associated
     * with the next byte in the segment, and record it.
     */
    next_byte = state->mpc[state->length];
    prefix_form = kPrefixTable[next_byte];
    if (prefix_form == 0) break;
    DEBUG(printf("Consume prefix[%d]: %02x => %x\n",
                 i, next_byte, prefix_form));
    state->prefix_mask |= prefix_form;
    ++state->num_prefix_bytes;
    ++state->length;
    DEBUG(printf("  prefix mask: %08x\n", state->prefix_mask));

    /* If the prefix byte is a REX prefix, record its value, since
     * bits 5-8 of this prefix bit may be needed later.
     */
    if (NACL_TARGET_SUBARCH == 64) {
      if (prefix_form == kPrefixREX) {
        state->rexprefix = next_byte;
        rex_index = i;
      } else if (prefix_form == kPrefixLOCK) {
        lock_index = i;
      }
    }
  }
  if (NACL_TARGET_SUBARCH == 64) {
    /* REX prefix must be last, unless FO exists. If FO
     * exists, it must be after REX.
     */
    if (rex_index >= 0) {
      if (lock_index >= 0) {
        return ((rex_index + 1) == lock_index) &&
          ((lock_index + 1) == state->num_prefix_bytes);
      } else {
        return (rex_index + 1) == state->num_prefix_bytes;
      }
    }
  }
  return TRUE;
}

/* Assuming we have matched the byte sequence OF 38, consume the corresponding
 * following (instruction) opcode byte, returning the possible list of
 * patterns that may match (or NULL if no such patterns).
 */
static Opcode* Consume0F38XXOpcodeBytes(NcInstState* state) {
  /* Fail if there are no more bytes. Otherwise, read the next
   * byte.
   */
  uint8_t opcode_byte;
  if (state->length >= state->length_limit) return NULL;
  opcode_byte = state->mpc[state->length++];

  /* TODO(karl) figure out if we need to encode prefix bytes,
   * or if the opcode flags do the same thing.
   */
  if (state->prefix_mask & kPrefixDATA16) {
    return g_OpcodeTable[Prefix660F38][opcode_byte];
  } else if (state->prefix_mask & kPrefixREPNE) {
    return g_OpcodeTable[PrefixF20F38][opcode_byte];
  } else if ((state->prefix_mask & ~kPrefixREX) == 0) {
    return g_OpcodeTable[Prefix0F38][opcode_byte];
  } else {
    /* Other prefixes like F3 cause an undefined instruction error. */
    return NULL;
  }
  /* NOT REACHED */
  return NULL;
}

/* Assuming we have matched the byte sequence OF 3A, consume the corresponding
 * following (instruction) opcode byte, returning the possible list of
 * patterns that may match (or NULL if no such patterns).
 */
static Opcode* Consume0F3AXXOpcodeBytes(NcInstState* state) {
  /* Fail if there are no more bytes. Otherwise, read the next
   * byte.
   */
  uint8_t opcode_byte;
  if (state->length >= state->length_limit) return NULL;
  opcode_byte = state->mpc[state->length++];

  /* TODO(karl) figure out if we need to encode prefix bytes,
   * or if the opcode flags do the same thing.
   */
  if (state->prefix_mask & kPrefixDATA16) {
    return g_OpcodeTable[Prefix660F3A][opcode_byte];
  } else if ((state->prefix_mask & ~kPrefixREX) == 0) {
    return g_OpcodeTable[Prefix0F3A][opcode_byte];
  } else {
    /* Other prefixes like F3 cause an undefined instruction error. */
    return NULL;
  }
  /* NOT REACHED */
  return NULL;
}

/* Assuming we have matched byte OF, consume the corresponding
 * following (instruction) opcode byte, returning the possible list of
 * patterns that may match (or NULL if no such pattern).
 */
static Opcode* Consume0FXXOpcodeBytes(NcInstState* state, uint8_t opcode_byte) {
  /* TODO(karl) figure out if we need to encode prefix bytes,
   * or if the opcode flags do the same thing.
   */
  if (state->prefix_mask & kPrefixDATA16) {
    return g_OpcodeTable[Prefix66OF][opcode_byte];
  } else if (state->prefix_mask & kPrefixREPNE) {
    return g_OpcodeTable[PrefixF20F][opcode_byte];
  } else if (state->prefix_mask & kPrefixREP) {
    return g_OpcodeTable[PrefixF30F][opcode_byte];
  } else {
    return g_OpcodeTable[Prefix0F][opcode_byte];
  }
  /* NOT REACHED */
  return NULL;
}

/* Consume the sequence of bytes corresponding to the 1-3 byte opcode.
 * Return the list of opcode (instruction) patterns that apply to
 * the matched instruction bytes (or NULL if no such patterns).
 */
static Opcode* ConsumeOpcodeBytes(NcInstState* state) {
  uint8_t opcode_byte;
  Opcode* cand_opcodes;

  /* Be sure that we don't exceed the segment length. */
  if (state->length >= state->length_limit) return NULL;

  /* Record the opcode(s) we matched. */
  opcode_byte = state->mpc[state->length++];
  if (opcode_byte == 0x0F) {
    uint8_t opcode_byte2;
    if (state->length >= state->length_limit) return NULL;
    opcode_byte2 = state->mpc[state->length++];
    switch (opcode_byte2) {
    case 0x38:
      cand_opcodes = Consume0F38XXOpcodeBytes(state);
      break;
    case 0x3a:
      cand_opcodes = Consume0F3AXXOpcodeBytes(state);
      break;
    default:
      cand_opcodes = Consume0FXXOpcodeBytes(state, opcode_byte2);
      break;
    }
  } else {
    cand_opcodes = g_OpcodeTable[NoPrefix][opcode_byte];
  }
  state->opcode = cand_opcodes;
  if (NULL != cand_opcodes) {
    DEBUG(printf("opcode pattern:\n"));
    DEBUG(PrintOpcode(stdout, state->opcode));
  }
  return cand_opcodes;
}

/* Compute the operand and address sizes for the instruction. Then, verify
 * that the opcode (instruction) pattern allows for such sizes. Aborts
 * the pattern match if any problems.
 */
static Bool ConsumeOperandSizeFlags(NcInstState* state) {
  state->operand_size = ExtractOperandSize(state);
  state->address_size = ExtractAddressSize(state);
  DEBUG(printf("operand size = %d, address size = %d\n",
               state->operand_size, state->address_size));
  if (state->opcode->flags &
      (OCF(OperandSize_w) | OCF(OperandSize_v) | OCF(OperandSize_o))) {
    OpcodeFlags good = 1;
    switch (state->operand_size) {
      case 2:
        good = (state->opcode->flags & OCF(OperandSize_w));
        break;
      case 4:
        good = (state->opcode->flags & OCF(OperandSize_v));
        break;
      case 8:
        good = (state->opcode->flags & OCF(OperandSize_o));
        break;
      default:
        good = 0;
        break;
    }
    if (!good) {
      /* The flags associated with the opcode (instruction) don't
       * allow the computed sizes, abort the  match of the instruction.
       */
      DEBUG(printf("Operand size %d doesn't match flag requirement!\n",
                   state->operand_size));
      return FALSE;
    }
  }
  return TRUE;
}

/* Consume the Mod/Rm byte of the instruction, if applicable.
 * Aborts the pattern match if any problems.
 */
static Bool ConsumeModRm(NcInstState* state) {
  /* First check if the opcode (instruction) pattern specifies that
   * a Mod/Rm byte is needed, and that reading it will not walk
   * past the end of the code segment.
   */
  if (state->opcode->flags &
      (OCF(OpcodeUsesModRm) | OCF(OpcodeInModRm))) {
    /* Has modrm byte. */
    if (state->length >= state->length_limit) {
      DEBUG(printf("Can't read mod/rm, no more bytes!\n"));
      return FALSE;
    }
    state->modrm = state->mpc[state->length++];
    state->num_disp_bytes = 0;
    state->first_disp_byte = 0;
    state->sib = 0;
    state->has_sib = FALSE;
    DEBUG(printf("consume modrm = %02x\n", state->modrm));

    /* Consume the remaining opcode value in the mod/rm byte
     * if applicable.
     */
    if (state->opcode->flags & OCF(OpcodeInModRm)) {
      /* TODO(karl) Optimize this with faster match on
       * opcode value in first operand.
       */
      if (modrm_opcode(state->modrm) !=
          (state->opcode->operands[0].kind - Opcode0)) {
        DEBUG(printf("Discarding, opcode in mrm byte (%02x) does not match\n",
                     modrm_opcode(state->modrm)));
        return FALSE;
      }
    }

    /* Compute whether displacement bytes and SIB bytes are needed. */
    if (16 == state->address_size) {
      /* Corresponding to table 2-1 of the Intel manual. */
      switch (modrm_mod(state->modrm)) {
      case 0x0:
        if (modrm_rm(state->modrm) == 0x06) {
          state->num_disp_bytes = 2;  /* disp16 */
        }
        break;
      case 0x1:
        state->num_disp_bytes = 1;    /* disp8 */
        break;
      case 0x2:
        state->num_disp_bytes = 2;    /* disp16 */
        break;
      default:
        break;
      }
    } else {
      /* Note: in 64-bit mode, 64-bit addressing is treated the same as 32-bit
       * addressing. Hence, this section covers the 32-bit addressing.
       */
      switch(modrm_mod(state->modrm)) {
      case 0x0:
        if (modrm_rm(state->modrm) == 0x05) {
          state->num_disp_bytes = 4;  /* disp32 */
        }
        break;
      case 0x1:
        state->num_disp_bytes = 1;    /* disp8 */
        break;
      case 0x2:
        state->num_disp_bytes = 4;    /* disp32 */
        break;
      default:
        break;
      }
      state->has_sib =
        (modrm_rm(state->modrm) == 0x04 &&
         modrm_mod(state->modrm) != 0x3);
    }
    DEBUG(printf("num disp bytes = %u, has sib = %u\n",
                 state->num_disp_bytes,
                 state->has_sib));
  }
  return TRUE;
}

/* Consume the SIB byte of the instruction, if applicable. Aborts the pattern
 * match if any problems are found.
 */
static Bool ConsumeSib(NcInstState* state) {
  /* First check that the opcode (instruction) pattern specifies that
   * a SIB byte is needed, and that reading it will not walk past
   * the end of the code segment.
   */
  if (state->has_sib) {
    if (state->length >= state->length_limit) {
      DEBUG(printf("Can't consume sib, no more bytes!\n"));
      return FALSE;
    }
    /* Read the SIB byte and record. */
    state->sib = state->mpc[state->length++];
    DEBUG(printf("sib = %02x\n", state->sib));
    if (sib_base(state->sib) == 0x05) {
      switch (modrm_mod(state->modrm)) {
        case 0:
          state->num_disp_bytes = 4;
          break;
        case 1:
          state->num_disp_bytes = 1;
          break;
        case 2:
          state->num_disp_bytes = 4;
          break;
        default:
          DEBUG(printf("Sib byte implies modrm.mod field <= 2, match fails\n"));
          return FALSE;
      }
      DEBUG(printf("sib implies num disp bytes = %u\n", state->num_disp_bytes));
    }
  }
  return TRUE;
}

/* Consume the needed displacement bytes, if applicable. Abort the
 * pattern match if any problems are found.
 */
static Bool ConsumeDispBytes(NcInstState* state) {
  /* First check if the opcode (instruction) pattern specifies that
   * displacement bytes should be read, and that reading it will not
   * walk past the end of the code segment.
   */
  if (state->num_disp_bytes > 0) {
    int new_length = state->length + state->num_disp_bytes;
    if (new_length > state->length_limit) {
      DEBUG(printf("Can't consume disp, no more bytes!\n"));
      return FALSE;
    }
    /* Read the displacement bytes. */
    state->first_disp_byte = state->length;
    state->length = new_length;
  }
  return TRUE;
}

/* Consume the needed immediate bytes, if applicable. Abort the
 * pattern match if any problems are found.
 */
static Bool ConsumeImmediateBytes(NcInstState* state) {
  /* find out how many immediate bytes are expected. */
  state->num_imm_bytes = 0;
  state->first_imm_byte = 0;
  if (state->opcode->flags &
      (OCF(OpcodeIb) | OCF(OpcodeIw) | OCF(OpcodeIb) | OCF(OpcodeIo))) {
    int new_length;
    if (state->opcode->flags & OCF(OpcodeIv)) {
      state->num_imm_bytes = 4;
    } else if (state->opcode->flags & OCF(OpcodeIb)) {
      state->num_imm_bytes = 1;
    } else if (state->opcode->flags & OCF(OpcodeIw)) {
      state->num_imm_bytes = 2;
    } else if (state->opcode->flags & OCF(OpcodeIo)) {
      state->num_imm_bytes = 8;
    }
    DEBUG(printf("num immediate bytes = %d\n", state->num_imm_bytes));
    /* Before reading immediate bytes, be sure that we won't walk
     * past the end of the code segment.
     */
    new_length = state->length + state->num_imm_bytes;
    if (new_length > state->length_limit) {
      DEBUG(printf("Can't consume immediate, no more bytes!\n"));
      return FALSE;
    }
    /* Read the immediate bytes. */
    state->first_imm_byte = state->length;
    state->length = new_length;
  }
  return TRUE;
}

/* Validate that any opcode (instruction) pattern prefix assumptions are
 * met by prefix bits. If not, abort the pattern match.
 */
static Bool ValidatePrefixFlags(NcInstState* state) {
  /* Check lock prefix assumptions. */
  if ((state->prefix_mask & kPrefixLOCK) &&
      0 == (state->opcode->flags & OCF(OpcodeLockable))) {
    DEBUG(printf("Instruction doesn't allow lock prefix\n"));
    return FALSE;
  }
  /* Check REX prefix assumptions. */
  if (NACL_TARGET_SUBARCH == 64 &&
      (state->prefix_mask & kPrefixREX)) {
    if (state->opcode->flags &
        (OCF(OpcodeUsesRexW) | OCF(OpcodeHasRexR) | OCF(OpcodeRex))) {
      if (((state->opcode->flags & OCF(OpcodeUsesRexW)) &&
           0 == (state->rexprefix & 0x8)) ||
          ((state->opcode->flags & OCF(OpcodeHasRexR)) &&
           0 == (state->rexprefix & 0x4))) {
        DEBUG(printf("can't match REX prefix requirement\n"));
        return FALSE;
      }
    }
  }
  return TRUE;
}

/* Given the current location of the (relative) pc of the given instruction
 * iterator, update the given state to hold the (found) matched opcode
 * (instruction) pattern. If no matching pattern exists, set the state
 * to a matched undefined opcode (instruction) pattern. In all cases,
 * update the state to hold all information on the matched bytes of the
 * instruction. Note: The iterator expects that the opcode field is
 * changed from NULL to non-NULL by this fuction.
 */
void DecodeInstruction(
    NcInstIter* iter,
    NcInstState* state) {
  uint8_t opcode_length;
  Opcode* cand_opcodes;
  /* Start by consuming the prefix bytes, and getting the possible
   * candidate opcode (instruction) patterns that can match, based
   * on the consumed opcode bytes.
   */
  NcInstStateInit(iter, state);
  if (ConsumePrefixBytes(state)) {
    cand_opcodes = ConsumeOpcodeBytes(state);
    /* Try matching all possible candidates, in the order they are specified.
     * Quit when the first pattern is matched.
     */
    opcode_length = state->length;
    while (cand_opcodes != NULL) {
      state->opcode = cand_opcodes;
      state->length = opcode_length;
      DEBUG(printf("try opcode pattern:\n"));
      DEBUG(PrintOpcode(stdout, state->opcode));
      if (ConsumeOperandSizeFlags(state) &&
          ConsumeModRm(state) &&
          ConsumeSib(state) &&
          ConsumeDispBytes(state) &&
          ConsumeImmediateBytes(state) &&
          ValidatePrefixFlags(state)) {
        /* found a match, exit loop. */
        break;
      } else {
        /* match failed, try next candidate pattern. */
        state->opcode = NULL;
        cand_opcodes = cand_opcodes->next_rule;
      }
    }
  }

  /* If we did not match a defined opcode, match the undefined opcode,
   * forcing field opcode to be non-NULL.
   */
  if (NULL == state->opcode) {
    DEBUG(printf("no instruction found, converting to undefined\n"));

    /* Can't figure out instruction, give up. */
    state->opcode = &g_Undefined_Opcode;
    state->length = opcode_length;
    if (state->length == 0 && state->length < state->length_limit) {
      /* Make sure we eat at least one byte. */
      ++state->length;
    }
    state->modrm = 0;
    state->has_sib = FALSE;
    state->sib = 0;
    state->num_disp_bytes = 0;
    state->first_disp_byte = 0;
    state->num_imm_bytes = 0;
    state->first_imm_byte = 0;
    state->operand_size = 32;
    state->address_size = 32;
  }
}

Opcode* NcInstStateOpcode(NcInstState* state) {
  return state->opcode;
}

Elf_Addr NcInstStateVpc(NcInstState* state) {
  return state->vpc;
}

OpExpressionNode* NcInstStateExpression(NcInstState* state) {
  if (NULL == state->opcode_exp) {
    state->opcode_exp = TranslateOpcode(state);
  }
  return state->opcode_exp;
}

uint8_t NcInstStateLength(NcInstState* state) {
  return state->length;
}

uint8_t NcInstStateByte(NcInstState* state, uint8_t index) {
  assert(index < state->length);
  return state->mpc[index];
}
