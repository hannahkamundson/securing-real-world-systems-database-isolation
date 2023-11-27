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


/* @file
 *
 * Minimal ELF header declaration / constants.  Only Elf values are
 * handled, and constants are defined only for fields that are actualy
 * used.  (Unused constants for used fields are include only for
 * "completeness", though of course in many cases there are more
 * values in use, e.g., the EM_* values for e_machine.)
 *
 * Note: We define both 32 and 64 bit versions of the elf file in
 * separate files, and then unify them here so that elf readers
 * can process elf files without having to specify the bit format.
 *
 * (Re)Created from the ELF specification at
 * http://x86.ddj.com/ftp/manuals/tools/elf.pdf which is referenced
 * from wikipedia article
 * http://en.wikipedia.org/wki/Executable_and_Linkable_Format
 */

#ifndef NATIVE_CLIENT_SRC_INCLUDE_ELF_H_
#define NATIVE_CLIENT_SRC_INCLUDE_ELF_H_ 1

#include "native_client/src/include/elf32.h"

#include "native_client/src/include/portability.h"

EXTERN_C_BEGIN


/* Note: We really should include elf64.h unconditionally. However,
 * The current include file is dependent on <stdint.h> which is
 * not supported on all Windows platforms. Until this problem
 * is fixed, we conditionally include elf64 only if needed.
 */

/* TODO: All PRIElf_xxx defines should redirect to appropriate ones in
 * inttypes.h/portability.h. portability.h should be the only place
 * that handles 32-bit/64-bit windows/linux/osx differences. Note:
 * here we don't address the fact that 64-bit Windows uses 32-bit
 * long.
 */
#if __WORDSIZE == 64

#include "native_client/src/include/elf64.h"

/* Define sub architecture neutral types */
typedef Elf64_Addr   Elf_Addr;
typedef Elf64_Off    Elf_Off;
typedef Elf64_Half   Elf_Half;
typedef Elf64_Word   Elf_Word;
typedef Elf64_Sword  Elf_Sword;
typedef Elf64_Xword  Elf_Xword;
typedef Elf64_Sxword Elf_Sxword;

/* Define print directives for each elf type. */
#define PRIElf_Addr   "llx"
#define PRIElf_AddrAll "016llx"
#define PRIElf_Off    "llu"
#define PRIElf_Half   "u"
#define PRIElf_Word   "u"
#define PRIElf_SWord  "d"
#define PRIElf_Xword  "llu"
#define PRIElf_Sxword "lld"

/* Define ranges for elf types. */
#define MIN_ELF_ADDR 0x0
#define MAX_ELF_ADDR 0xffffffffffffffff

/* Define a neutral form of the file header. */
typedef Elf64_Ehdr Elf_Ehdr;

/* Define a neutral form of a program header. */
typedef Elf64_Phdr Elf_Phdr;

/* Define neutral section headers. */
typedef Elf64_Shdr Elf_Shdr;

#else

/* Define sub architecture neutral types */
typedef Elf32_Addr  Elf_Addr;
typedef Elf32_Off   Elf_Off;
typedef Elf32_Half  Elf_Half;
typedef Elf32_Word  Elf_Word;
typedef Elf32_Sword Elf_Sword;
typedef Elf32_Word  Elf_Xword;
typedef Elf32_Sword Elf_Xsword;

/* Define print directives for each elf type. */
#define PRIElf_Addr   "x"
#define PRIElf_AddrAll "08x"
#define PRIElf_Off    "u"
#define PRIElf_Half   "u"
#define PRIElf_Word   "u"
#define PRIElf_SWord  "d"
#define PRIElf_Xword  "u"
#define PRIElf_Sxword "d"

/* Define ranges for elf types. */
#define MIN_ELF_ADDR 0x0
#define MAX_ELF_ADDR 0xffffffff

/* Define a neutral form of the file header. */
typedef Elf32_Ehdr Elf_Ehdr;

/* Define a neutral form of a program header. */
typedef Elf32_Phdr Elf_Phdr;

/* Define neutral section headers. */
typedef Elf32_Shdr Elf_Shdr;

#endif

EXTERN_C_END

#endif  /* NATIVE_CLIENT_SRC_INCLUDE_ELF_H_ */
