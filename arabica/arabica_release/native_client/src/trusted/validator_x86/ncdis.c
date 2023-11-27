/*
 * Copyright 2008, Google Inc.
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
 * ncdis.c - disassemble using NaCl decoder.
 * Mostly for testing.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "native_client/src/include/nacl_elf.h"
#include "native_client/src/shared/utils/types.h"
#include "native_client/src/shared/utils/flags.h"
#include "native_client/src/trusted/validator_x86/nc_inst_iter.h"
#include "native_client/src/trusted/validator_x86/nc_segment.h"
#include "native_client/src/trusted/validator_x86/ncop_exps.h"
#include "native_client/src/trusted/validator_x86/ncdis_util.h"
#include "native_client/src/trusted/validator_x86/ncfileutil.h"
#include "native_client/src/trusted/validator_x86/ncvalidate_internaltypes.h"

/* The name of the executable that is being run. */
static const char* exec_name = "???";

void Fatal(const char *fmt, ...) {
  FILE* fp = stdout;
  va_list ap;
  fprintf(fp, "Fatal: ");
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
  exit(-1);
}

void Info(const char *fmt, ...) {
  FILE* fp = stdout;
  va_list ap;
  fprintf(fp, "Info: ");
  va_start(ap, fmt);
  vfprintf(fp, fmt, ap);
  va_end(ap);
}


static int AnalyzeSections(ncfile *ncf) {
  int badsections = 0;
  int ii;
  const Elf_Shdr* shdr = ncf->sheaders;

  for (ii = 0; ii < ncf->shnum; ii++) {
    Info("section %d sh_addr %x offset %x flags %x\n",
         ii, (uint32_t)shdr[ii].sh_addr,
         (uint32_t)shdr[ii].sh_offset, (uint32_t)shdr[ii].sh_flags);
    if ((shdr[ii].sh_flags & SHF_EXECINSTR) != SHF_EXECINSTR)
      continue;
    Info("parsing section %d\n", ii);
    NCDecodeSegment(ncf->data + (shdr[ii].sh_addr - ncf->vbase),
                    shdr[ii].sh_addr, shdr[ii].sh_size, NULL);
  }
  return -badsections;
}


static void AnalyzeCodeSegments(ncfile *ncf, const char *fname) {
  if (AnalyzeSections(ncf) < 0) {
    fprintf(stderr, "%s: text validate failed\n", fname);
  }
}

/* Don't apply native client rules for elf files. */
static Bool FLAGS_not_nc = FALSE;

/* Capture a sequence of bytes defining an instruction (up to a
 * MAX_BYTES_PER_X86_INSTRUCTION). This sequence is used to run
 * a (debug) test of the disassembler.
 */
static uint8_t FLAGS_decode_instruction[MAX_BYTES_PER_X86_INSTRUCTION];

/* Define the number of bytes supplied for a debug instruction. */
static int FLAGS_decode_instruction_size = 0;

/* Flag defining the value of the pc to use when decoding an instruction
 * through decode_instruction.
 */
static uint32_t FLAGS_decode_pc = 0;

/* Flag defining an input file to use as command line arguments
 * (one per input line). When specified, run the disassembler
 * on each command line. The empty string "" denotes that no command
 * line file was specified. A dash ("-") denotes that standard input
 * should be used to get command line arguments.
 */
static char* FLAGS_commands = "";

/* Flag, when used in combination with the commands flag, will turn
 * on input copy rules, making the genrated output contain comments
 * and the command line arguments as part of the corresponding
 * generated output. For more details on this, see ProcessInputFile
 * below.
 */
static Bool FLAGS_self_document = FALSE;

/* When true, use an instruction iterator instead of NCDecodeSegment.
 */
static Bool FLAGS_use_iter = FALSE;

/*
 * Store default values of flags on the first call. On subsequent
 * calls, resets the flags to the default value.
 *
 * *WARNING* In order for this to work, this function must be
 * called before GrokFlags
 *
 * NOTE: we only allow the specification of -use_iter at the top-level
 * command line..
 */
static void ResetFlags() {
  int i;
  static int DEFAULT_not_nc;
  static uint32_t DEFAULT_decode_pc;
  static char* DEFAULT_commands;
  static Bool DEFAULT_self_document;
  static Bool is_first_call = TRUE;
  if (is_first_call) {
    DEFAULT_not_nc = FLAGS_not_nc;
    DEFAULT_decode_pc = FLAGS_decode_pc;
    DEFAULT_commands = FLAGS_commands;
    DEFAULT_self_document = FLAGS_self_document;
    is_first_call = FALSE;
  }

  FLAGS_not_nc = DEFAULT_not_nc;
  FLAGS_decode_pc = DEFAULT_decode_pc;
  FLAGS_commands = DEFAULT_commands;
  FLAGS_self_document = DEFAULT_self_document;
  /* Always clear the decode instruction. */
  FLAGS_decode_instruction_size = 0;
  for (i = 0; i < MAX_BYTES_PER_X86_INSTRUCTION; ++i) {
    FLAGS_decode_instruction[i] = 0;
  }
}

/*
 * Attempts to parse a hexidecimal value corresponding to the
 * value of a byte, as a command line argument. Returns true iff able
 * to parse and set the corresponding found hexidecimal byte.
 */
Bool GrokHexByte(const char* name,
                 const char* arg,
                 uint8_t* flag) {
  uint32_t value = 0;
  Bool result = GrokUint32HexFlag(name, arg, &value);
  if (result && value >= 256) {
    Fatal("Flag %s specifies illegal hexidecimal byte value %u\n", name, value);
  }
  *flag = (uint8_t) value;
  return result;
}

/* Recognizes flags in argv, processes them, and then removes them.
 * Returns the updated value for argc.
 */
int GrokFlags(int argc, const char *argv[]) {
  int i;
  uint8_t decode_byte;
  int new_argc;
  if (argc == 0) return 0;
  exec_name = argv[0];
  new_argc = 1;
  for (i = 1; i < argc; ++i) {
    if (GrokBoolFlag("-not_nc", argv[i], &FLAGS_not_nc) ||
        GrokUint32HexFlag("-pc", argv[i], &FLAGS_decode_pc) ||
        GrokCstringFlag("-commands", argv[i], &FLAGS_commands) ||
        GrokBoolFlag("-self_document", argv[i], &FLAGS_self_document) ||
        GrokBoolFlag("-use_iter", argv[i], &FLAGS_use_iter)) {
    } else if (GrokHexByte("-b", argv[i], &decode_byte)) {
      FLAGS_decode_instruction[FLAGS_decode_instruction_size++] = decode_byte;
      if (FLAGS_decode_instruction_size >= MAX_BYTES_PER_X86_INSTRUCTION) {
        Fatal("-b option specified more than %u times\n",
              MAX_BYTES_PER_X86_INSTRUCTION);
      }
    } else {
      argv[new_argc++] = argv[i];
    }
  }
  return new_argc;
}

/* Process the command line arguments. */
static const char* GrokArgv(int argc, const char* argv[]) {
  if (argc != 2) {
    Fatal("no filename specified\n");
  }
  return argv[argc-1];
}

static void ProcessCommandLine(int argc, const char* argv[]);

/* Defines the maximum number of characters allowed on an input line
 * of the input text defined by the commands command line option.
 */
#define MAX_INPUT_LINE 4096

/* Defines the characters used as (token) separators to recognize command
 * line arguments when processing lines of text in the text file specified
 * by the commands command line option.
 */
#define CL_SEPARATORS " \t\n"

/* Copies the text from the input line (which should be command line options),
 * up to any trailing comments (i.e. the pound sign).
 *   input_line - The line of text to process.
 *   tokens - The extracted text from the input_line.
 *   max_length - The maximum length of input_line and tokens.
 *
 * Note: If input_line doesn't end with a null terminator, one is automatically
 * inserted.
 */
static void CopyCommandLineTokens(char* input_line,
                                  char* token_text,
                                  size_t max_length) {
  size_t i;
  for (i = 0; i < max_length; ++i) {
    char ch;
    if (max_length == i + 1) {
      /* Be sure we end the string with a null terminator. */
      input_line[i] = '\0';
    }
    ch = input_line[i];
    token_text[i] = ch;
    if (ch == '\0') return;
    if (ch == '#') {
      token_text[i] = '\0';
      return;
    }
  }
}

/* Tokenize the given text to find command line arguments, and
 * add them to the given list of command line arguments.
 *
 * *WARNING* This function will (destructively) modify the
 * contents of token_text, by converting command line option
 * separator characters into newlines.
 */
static void ExtractTokensAndAddToArgv(
    char* token_text,
    int* argc,
    const char* argv[]) {
  /* Note: Assume that each command line argument corresponds to
   * non-blank text, which is a HACK, but should be sufficient for
   * what we need.
   */
  char* token = strtok(token_text, CL_SEPARATORS);
  while (token != NULL) {
    argv[(*argc)++] = token;
    token = strtok(NULL, CL_SEPARATORS);
  }
}

/* Print out the contents of text, up to the first occurence of the
 * pound sign.
 */
static void PrintUpToPound(const char text[]) {
  int i;
  for (i = 0; i < MAX_INPUT_LINE; ++i) {
    char ch = text[i];
    switch (ch) {
      case '#':
        putchar(ch);
        return;
      case '\0':
        return;
      default:
        putchar(ch);
    }
  }
}

/* Reads the given text file and processes the command line options specified
 * inside of it. Each line specifies a separate sequence of command line
 * arguments to process.
 *
 * Note:
 * (a) The '#' is used as a comment delimiter.
 * (b) whitespace lines are ignored.
 * (c) If flag --self_document is specified, comment lines and whitespace
 *     lines will automatically be copied to stdout. In addition, command
 *     line arguments will be copied to stdout before processing them.
 *     Further, if the command line arguments are followed by a comment,
 *     only text up to (and including) the '#' will be copied. This allows
 *     the input file to contain the (hopefully single lined) output that
 *     would be generated by the given command line arguments. Therefore,
 *     if set up correctly, the output of the disassembler (in this case)
 *     should be the same as the input file (making it easy to use the
 *     input file as the the corresponding GOLD file to test against).
 */
static void ProcessInputFile(FILE* file) {
  char input_line[MAX_INPUT_LINE];
  const Bool self_document = FLAGS_self_document;
  while (fgets(input_line, MAX_INPUT_LINE, file) != NULL) {
    char token_text[MAX_INPUT_LINE];
    const char* line_argv[MAX_INPUT_LINE];
    int line_argc = 0;

    /* Copy the input line (up to the first #) into token_text */
    CopyCommandLineTokens(input_line, token_text, MAX_INPUT_LINE);

    /* Tokenize the commands to build argv.
     * Note: Since each token is separated by a blank,
     * and the input is no more than MAX_INPUT_LINE,
     * we know (without checking) that line_argc
     * will not exceed MAX_INPUT_LINE.
     */
    line_argv[line_argc++] = exec_name;
    ExtractTokensAndAddToArgv(token_text, &line_argc, line_argv);

    /* Process the parsed input line. */
    if (1 == line_argc) {
      /* No command line arguments. */
      if (self_document) {
        printf("%s", input_line);
      }
    } else {
      /* Process the tokenized command line. */
      if (self_document) {
        PrintUpToPound(input_line);
      }
      ProcessCommandLine(line_argc, line_argv);
    }
  }
  ResetFlags();
}

/* Run the disassembler using the given command line arguments. */
static void ProcessCommandLine(int argc, const char* argv[]) {
  int new_argc;

  ResetFlags();
  new_argc = GrokFlags(argc, argv);
  if (FLAGS_decode_instruction_size > 0) {
    /* Command line options specify an instruction to decode, run
     * the disassembler on the instruction to print out the decoded
     * results.
     */
    struct NCValidatorState mstate;
    if (new_argc > 1) {
      Fatal("unrecognized option '%s'\n", argv[1]);
    } else if (0 != strcmp(FLAGS_commands, "")) {
      Fatal("can't specify -commands and -b options simultaneously\n");
    }
    if (FLAGS_use_iter) {
      NcSegment segment;
      NcInstIter* iter;
      NcSegmentInitialize(FLAGS_decode_instruction,
                          FLAGS_decode_pc,
                          FLAGS_decode_instruction_size,
                          &segment);
      for (iter = NcInstIterCreate(&segment); NcInstIterHasNext(iter);
           NcInstIterAdvance(iter)) {
        NcInstStatePrintDisassembled(stdout, NcInstIterGetState(iter));
      }
      NcInstIterDestroy(iter);
    } else {
      NCDecodeSegment(FLAGS_decode_instruction,
                      FLAGS_decode_pc,
                      FLAGS_decode_instruction_size,
                      &mstate);
    }
  } else if (0 != strcmp(FLAGS_commands, "")) {
    /* Use the given input file to find command line arguments,
     * and process.
     */
    if (0 == strcmp(FLAGS_commands, "-")) {
      ProcessInputFile(stdin);
    } else {
      FILE* input = fopen(FLAGS_commands, "r");
      if (NULL == input) {
        Fatal("Can't open commands file: %s\n", FLAGS_commands);
      }
      ProcessInputFile(input);
      fclose(input);
    }
  } else {
    /* Command line should specify an executable to disassemble.
     * Read the file and disassemble it.
     */
    ncfile *ncf;
    const char* filename = GrokArgv(new_argc, argv);

    Info("processing %s", filename);
    ncf = nc_loadfile_depending(filename, !FLAGS_not_nc);
    if (ncf == NULL) {
      Fatal("nc_loadfile(%s): %s\n", strerror(errno));
    }

    if (FLAGS_use_iter) {
      Fatal("Can't use instruction iterator yet!\n");
    } else {
      AnalyzeCodeSegments(ncf, filename);
    }

    nc_freefile(ncf);
  }
}

int main(int argc, const char *argv[]) {
  NCDecodeRegisterCallbacks(PrintInstStdout, NULL, NULL, NULL);
  ProcessCommandLine(argc, argv);
  return 0;
}
