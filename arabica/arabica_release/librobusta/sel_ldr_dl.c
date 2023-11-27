/*
 * Copyright 2013, SoS Laboratory, Lehigh University
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
 *     * Neither the name of SoS Laboratory, Lehigh University nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
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

/* gtan: sel_ldr_standard.c adapted for dynamic linking */

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
 * NaCl Simple/secure ELF loader (NaCl SEL).
 */

#include "native_client/src/include/portability.h"

#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "native_client/src/include/nacl_elf.h"
#include "native_client/src/trusted/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/platform/nacl_time.h"

#include "native_client/src/trusted/service_runtime/sel_memory.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/sel_addrspace.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_check.h"
#include "native_client/src/trusted/service_runtime/nacl_closure.h"
#include "native_client/src/trusted/service_runtime/nacl_sync_queue.h"
#include "native_client/src/trusted/service_runtime/tramp.h"

#define PTR_ALIGN_MASK  ((sizeof(void *))-1)

/*
 * Other than empty segments, these are the only ones that are allowed.
 */
struct NaClPhdrChecks nacl_dyn_phdr_check_data[] = {
  /* phdr */
  { PT_PHDR, PF_R, PCA_IGNORE, 0, 0, },
  /* MSNaCl: rodata */
  { PT_LOAD, PF_R, PCA_NONE, 0, 0, },
  /* text */
  //{ PT_LOAD, PF_R|PF_X, PCA_TEXT_CHECK, 1, NACL_TRAMPOLINE_END, },
  // MSNaCl
  { PT_LOAD, PF_R|PF_X, PCA_TEXT_CHECK, 1, 0, }, 
  /* rodata */
  { PT_LOAD, PF_R, PCA_NONE, 0, 0, },
  /* data/bss */
  { PT_LOAD, PF_R|PF_W, PCA_NONE, 0, 0, },
  /*
   * allow optional GNU stack permission marker, but require that the
   * stack is non-executable.
   */
  { PT_GNU_STACK, PF_R|PF_W, PCA_NONE, 0, 0, },
};

NaClErrorCode NaClProcessDynPhdrs(struct NaClApp *nap) {
    /* Scan phdrs and do sanity checks in-line.  Verify that the load
     * address is NACL_TRAMPOLINE_END, that we have a single text
     * segment.  Data and TLS segments are not required, though it is
     * hard to avoid with standard tools, but in any case there should
     * be at most one each.  Ensure that no segment's vaddr is outside
     * of the address space.  Ensure that PT_GNU_STACK is present, and
     * that x is off.
     */
  int         seen_seg[(sizeof nacl_dyn_phdr_check_data
                        / sizeof nacl_dyn_phdr_check_data[0])];
  int         segnum;
  Elf32_Phdr  *php;
  size_t      j;
  uintptr_t   max_vaddr;

  memset(seen_seg, 0, sizeof seen_seg);
  max_vaddr = NACL_TRAMPOLINE_END;
  /*
   * nacl_dyn_phdr_check_data is small, so O(|check_data| * nap->elf_hdr.e_phum)
   * is okay.
   */
  for (segnum = 0; segnum < nap->elf_hdr.e_phnum; ++segnum) {
    php = &nap->phdrs[segnum];
    NaClLog(3, "Looking at segment %d, type 0x%x, p_flags 0x%x\n",
            segnum, php->p_type, php->p_flags);
    php->p_flags &= ~PF_MASKOS;
    if (nap->elf_hdr.e_type == ET_DYN)
      php->p_vaddr += NACL_TRAMPOLINE_END; 
    for (j = 0;
         j < sizeof nacl_dyn_phdr_check_data/sizeof nacl_dyn_phdr_check_data[0];
         ++j) {
      if (php->p_type == nacl_dyn_phdr_check_data[j].p_type
          && php->p_flags == nacl_dyn_phdr_check_data[j].p_flags
	  && !seen_seg[j]) { 
        NaClLog(2, "Matched nacl_dyn_phdr_check_data[%"PRIdS"]\n", j);
	/* MSNaCl
        if (seen_seg[j]) {
          NaClLog(2, "Segment %d is a type that has been seen\n", segnum);
          return LOAD_DUP_SEGMENT;
	  }
	*/
        ++seen_seg[j];

        if (PCA_IGNORE == nacl_dyn_phdr_check_data[j].action) {
          NaClLog(3, "Ignoring\n");
          goto next_seg;
        }

        if (0 != php->p_memsz) {
          /*
           * We will load this segment later.  Do the sanity checks.
           */
          if (0 != nacl_dyn_phdr_check_data[j].p_vaddr
              && (nacl_dyn_phdr_check_data[j].p_vaddr != php->p_vaddr)) {
            NaClLog(2,
                    ("Segment %d: bad virtual address: 0x%08x,"
                     " expected 0x%08x\n"),
                    segnum,
                    php->p_vaddr,
                    nacl_dyn_phdr_check_data[j].p_vaddr);
            return LOAD_SEGMENT_BAD_LOC;
          }
          if (php->p_vaddr < NACL_TRAMPOLINE_END) {
            NaClLog(2, "Segment %d: virtual address (0x%08x) too low\n",
                    segnum,
                    php->p_vaddr);
            return LOAD_SEGMENT_OUTSIDE_ADDRSPACE;
          }
          /*
           * integer overflow?  Elf32_Addr and Elf32_Word are uint32_t,
           * so the addition/comparison is well defined.
           */
          if (php->p_vaddr + php->p_memsz < php->p_vaddr) {
            NaClLog(2,
                    "Segment %d: p_memsz caused integer overflow\n",
                    segnum);
            return LOAD_SEGMENT_OUTSIDE_ADDRSPACE;
          }
          if (php->p_vaddr + php->p_memsz >= (1U << nap->addr_bits)) {
            NaClLog(2,
                    "Segment %d: too large, ends at 0x%08x\n",
                    segnum,
                    php->p_vaddr + php->p_memsz);
            return LOAD_SEGMENT_OUTSIDE_ADDRSPACE;
          }
          if (php->p_filesz > php->p_memsz) {
            NaClLog(2,
                    ("Segment %d: file size 0x%08x larger"
                     " than memory size 0x%08x\n"),
                    segnum,
                    php->p_filesz,
                    php->p_memsz);
            return LOAD_SEGMENT_BAD_PARAM;
          }

          php->p_flags |= PF_OS_WILL_LOAD;
          /* record our decision that we will load this segment */

          /*
           * NACL_TRAMPOLINE_END <= p_vaddr
           *                     <= p_vaddr + p_memsz
           *                     < (1U << nap->addr_bits)
           */
          if (max_vaddr < php->p_vaddr + php->p_memsz) {
            max_vaddr = php->p_vaddr + php->p_memsz;
          }
        }

        switch (nacl_dyn_phdr_check_data[j].action) {
          case PCA_NONE:
            break;
          case PCA_TEXT_CHECK:
            if (0 == php->p_memsz) {
              return LOAD_BAD_ELF_TEXT;
            }
	    nap->text_region_end = php->p_vaddr + php->p_filesz; 
            break;
          case PCA_IGNORE:
            break;
        }
        goto next_seg;
      }
    }
    /* segment not in nacl_dyn_phdr_check_data */
    if (0 == php->p_memsz) {
      NaClLog(3, "Segment %d zero size: ignored\n", segnum);
      continue;
    }
    NaClLog(2,
            "Segment %d is of unexpected type 0x%x, flag 0x%x\n",
            segnum,
            php->p_type,
            php->p_flags);
    //return LOAD_BAD_SEGMENT;
    /* Ignore unrecognised segment types */ 
 next_seg:
    ;
  }
  for (j = 0;
       j < sizeof nacl_dyn_phdr_check_data/sizeof nacl_dyn_phdr_check_data[0];
       ++j) {
    if (nacl_dyn_phdr_check_data[j].required && !seen_seg[j]) {
      return LOAD_REQUIRED_SEG_MISSING;
    }
  }
  nap->data_end = nap->break_addr = max_vaddr;
  /*
   * Memory allocation will use NaClRoundPage(nap->break_addr), but
   * the system notion of break is always an exact address.  Even
   * though we must allocate and make accessible multiples of pages,
   * the linux-style brk system call (which returns current break on
   * failure) permits an arbitrarily aligned address as argument.
   */

  return LOAD_OK;
}

NaClErrorCode NaClAppLoadDynFile(struct Gio      *gp,
				 struct NaClApp  *nap) {
  NaClErrorCode ret = LOAD_INTERNAL;
  NaClErrorCode subret;

  int           cur_ph;

  /* NACL_MAX_ADDR_BITS < 32 */
  if (nap->addr_bits > NACL_MAX_ADDR_BITS) {
    ret = LOAD_ADDR_SPACE_TOO_BIG;
    goto done;
  }

  nap->stack_size = NaClRoundAllocPage(nap->stack_size);

  /* nap->addr_bits <= NACL_MAX_ADDR_BITS < 32 */
  if ((*gp->vtbl->Read)(gp,
                        &nap->elf_hdr,
                        sizeof nap->elf_hdr)
      != sizeof nap->elf_hdr) {
    ret = LOAD_READ_ERROR;
    goto done;
  }

#define DUMP(m,f)    do { NaClLog(2,                            \
                                  #m " = %" f "\n",             \
                                  nap->elf_hdr.m); } while (0)
  DUMP(e_ident+1, ".3s");
  DUMP(e_type, "#x");
  DUMP(e_machine, "#x");
  DUMP(e_version, "#x");
  DUMP(e_entry, "#x");
  DUMP(e_phoff, "#x");
  DUMP(e_shoff, "#x");
  DUMP(e_flags, "#x");
  DUMP(e_ehsize, "#x");
  DUMP(e_phentsize, "#x");
  DUMP(e_phnum, "#x");
  DUMP(e_shentsize, "#x");
  DUMP(e_shnum, "#x");
  DUMP(e_shstrndx, "#x");
#undef DUMP
  NaClLog(2, "sizeof(Elf32_Ehdr) = %x\n", (int) sizeof nap->elf_hdr);

  if (memcmp(nap->elf_hdr.e_ident, ELFMAG, SELFMAG)) {
    ret = LOAD_BAD_ELF_MAGIC;
    goto done;
  }
  if (ELFCLASS32 != nap->elf_hdr.e_ident[EI_CLASS]) {
    ret = LOAD_NOT_32_BIT;
    goto done;
  }
  /* gtan: Ignoreing the ABI check */
#if 0
  if (ELFOSABI_NACL != nap->elf_hdr.e_ident[EI_OSABI]) {
    NaClLog(LOG_ERROR, "Expected OSABI %d, got %d\n",
            ELFOSABI_NACL,
            nap->elf_hdr.e_ident[EI_OSABI]);
    ret = LOAD_BAD_ABI;
    goto done;
  }
  if (EF_NACL_ABIVERSION != nap->elf_hdr.e_ident[EI_ABIVERSION]) {
    NaClLog(LOG_ERROR, "Expected ABIVERSION %d, got %d\n",
            EF_NACL_ABIVERSION,
            nap->elf_hdr.e_ident[EI_ABIVERSION]);
    ret = LOAD_BAD_ABI;
    goto done;
  }
#endif
  //if (ET_EXEC != nap->elf_hdr.e_type) {
  // MSNaCl
  if (ET_EXEC != nap->elf_hdr.e_type &&
      ET_DYN != nap->elf_hdr.e_type) { 
    ret = LOAD_NOT_EXEC;
    goto done;
  }
  if (EM_386 != nap->elf_hdr.e_machine) {
    ret = LOAD_BAD_MACHINE;
    goto done;
  }
  if (EV_CURRENT != nap->elf_hdr.e_version) {
    ret = LOAD_BAD_ELF_VERS;
    goto done;
  }
  /* MSNaCl: add an offset to the entry point */
  if (nap->elf_hdr.e_type == ET_DYN)
      nap->elf_hdr.e_entry += NACL_TRAMPOLINE_END; 
  nap->entry_pt = nap->elf_hdr.e_entry;

  if (nap->elf_hdr.e_flags & EF_NACL_ALIGN_MASK) {
    unsigned long eflags = nap->elf_hdr.e_flags & EF_NACL_ALIGN_MASK;
    if (eflags == EF_NACL_ALIGN_16) {
      nap->align_boundary = 16;
    } else if (eflags == EF_NACL_ALIGN_32) {
      nap->align_boundary = 32;
    } else {
      ret = LOAD_BAD_ABI;
      goto done;
    }
  } else {
    nap->align_boundary = 32;
  }

  /* read program headers */
  if (nap->elf_hdr.e_phnum > NACL_MAX_PROGRAM_HEADERS) {
    ret = LOAD_TOO_MANY_SECT;  /* overloaded */
    goto done;
  }
  free(nap->phdrs);
  nap->phdrs = malloc(nap->elf_hdr.e_phnum * sizeof nap->phdrs[0]);
  if (!nap->phdrs) {
    ret = LOAD_NO_MEMORY;
    goto done;
  }
  if (nap->elf_hdr.e_phentsize < sizeof nap->phdrs[0]) {
    ret = LOAD_BAD_SECT;
    goto done;
  }
  for (cur_ph = 0; cur_ph < nap->elf_hdr.e_phnum; ++cur_ph) {
    if ((*gp->vtbl->Seek)(gp,
                          nap->elf_hdr.e_phoff
                          + cur_ph * nap->elf_hdr.e_phentsize,
                          SEEK_SET) == -1) {
      ret = LOAD_BAD_SECT;
      goto done;
    }
    if ((*gp->vtbl->Read)(gp,
                          &nap->phdrs[cur_ph],
                          sizeof nap->phdrs[0])
        != sizeof nap->phdrs[0]) {
      ret = LOAD_BAD_SECT;
      goto done;
    }
#define DUMP(mem) do {\
        NaClLog(2, "%s: %x\n", #mem, nap->phdrs[cur_ph].mem);  \
      } while (0)
    DUMP(p_type);
    DUMP(p_offset);
    DUMP(p_vaddr);
    DUMP(p_paddr);
    DUMP(p_filesz);
    DUMP(p_memsz);
    DUMP(p_flags);
    NaClLog(2, " (%s %s %s)\n",
            (nap->phdrs[cur_ph].p_flags & PF_R) ? "PF_R" : "",
            (nap->phdrs[cur_ph].p_flags & PF_W) ? "PF_W" : "",
            (nap->phdrs[cur_ph].p_flags & PF_X) ? "PF_X" : "");
    DUMP(p_align);
#undef  DUMP
    NaClLog(2, "\n");
  }

  /*
   * We need to determine the size of the CS region.  (The DS and SS
   * region sizes are obvious -- the entire application address
   * space.)  NaClProcessPhdrs will figure out nap->text_region_bytes.
   */

  subret = NaClProcessDynPhdrs(nap);
  if (subret != LOAD_OK) {
    ret = subret;
    goto done;
  }

  if (!NaClAddrIsValidEntryPt(nap, nap->entry_pt)) {
    ret = LOAD_BAD_ENTRY;
    goto done;
  }

  NaClLog(2, "Allocating address space\n");
  subret = NaClAllocAddrSpace(nap);
  if (subret != LOAD_OK) {
    ret = subret;
    goto done;
  }

  NaClLog(2, "Loading into memory\n");
  subret = NaClLoadImage(gp, nap);
  if (subret != LOAD_OK) {
    ret = subret;
    goto done;
  }

  NaClLog(2, "Validating image\n");
  /*subret = NaClValidateImage(nap);
  if (subret != LOAD_OK) {
    ret = subret;
    goto done;
  }*/

  NaClLog(2, "Installing trampoline\n");

  NaClLoadTrampoline(nap);

  NaClLog(2, "Installing springboard\n");

  NaClLoadSpringboard(nap);

  NaClLog(2, "Applying memory protection\n");

  subret = NaClMemoryProtection(nap);
  if (subret != LOAD_OK) {
    ret = subret;
    goto done;
  }

  ret = LOAD_OK;
done:
  return ret;
}

