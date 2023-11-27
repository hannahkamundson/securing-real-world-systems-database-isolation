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
 * NaCl run time.
 */
#include <fcntl.h>
#include <stdlib.h>

#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/platform/nacl_log.h"
#include "native_client/src/trusted/platform/nacl_sync_checked.h"

#include "native_client/src/trusted/service_runtime/dyn_array.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_check.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/nacl_ldt.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_rt.h"

/*
 * Allocate ldt for app, without creating the main thread.
 */
NaClErrorCode NaClAppPrepareToLaunch(struct NaClApp     *nap,
                                     int                in_desc,
                                     int                out_desc,
                                     int                err_desc)
{
  uintptr_t           code_start;
  size_t              code_bytes;
  size_t              code_pages;

  uintptr_t           data_start;
  size_t              data_pages;

  uint16_t            cs;
  uint16_t            des_seg;

  int                 i;
  struct NaClHostDesc *nhdp;

  int                 descs[3];

  NaClErrorCode       retval = LOAD_INTERNAL;

  descs[0] = in_desc;
  descs[1] = out_desc;
  descs[2] = err_desc;

  NaClXMutexLock(&nap->mu);
  /*
   * Determine what region of memory can be accessed as code.
   */
  code_start = nap->code_mem_start;
  code_bytes = nap->text_region_end;
  VCHECK((code_bytes & ((1 << NACL_PAGESHIFT) - 1)) == 0,
        ("code_bytes (0x%08"PRIxS") is not page aligned\n",
	code_bytes));
  code_pages = 1 << (nap->addr_bits - NACL_PAGESHIFT); 

  if (code_pages < 1) {
    NaClLog(LOG_FATAL, "NaClAppPrepareToLaunch: fewer than one code pages?\n");
  }
  NaClLog(2,
          "NaClLdtAllocatePageSelector(code, 1, 0x%08"PRIxPTR", 0x%"PRIxS"\n",
          code_start, code_pages);
  cs = NaClLdtAllocatePageSelector(NACL_LDT_DESCRIPTOR_CODE,
                                   1,
                                   (void *) code_start,
                                   code_pages);
  NaClLog(2, "got 0x%x\n", cs);
  if (0 == cs) {
    retval = SRT_NO_SEG_SEL;
    goto done;
  }

  data_start = nap->data_mem_start;
  CHECK(nap->addr_bits > NACL_PAGESHIFT);
  data_pages = 1 << (nap->addr_bits - NACL_PAGESHIFT);

  if (data_pages < 1) {
    NaClLog(LOG_FATAL,
            "NaClAppPrepareToLaunch: address space is fewer than one page?\n");
  }
  NaClLog(2,
          "NaClLdtAllocatePageSelector(data, 1, 0x%08"PRIxPTR", 0x%"PRIxS"\n",
          data_start, data_pages - 1);
  des_seg = NaClLdtAllocatePageSelector(NACL_LDT_DESCRIPTOR_DATA,
                                        0,
                                        (void *) data_start,
                                        data_pages);

  NaClLog(2, "got 0x%x\n", des_seg);
  if (0 == des_seg) {
    NaClLdtDeleteSelector(cs);
    retval = SRT_NO_SEG_SEL;
    goto done;
  }
  nap->code_seg_sel = cs;
  nap->data_seg_sel = des_seg;
  /*
   * Note that gs is thread-specific and not global, so that is allocated
   * elsewhere.  See nacl_app_thread.c.
   */

  /*
   * We dup the stdin, stdout, and stderr descriptors and wrap them in
   * NaClHostDesc objects.  Those in turn are wrapped by the
   * NaClDescIoDesc subclass of NaClDesc, and then put into the
   * open-file table.  NaCl app I/O operations will use these shared
   * descriptors, and if they close one of these descriptors it will
   * only be a duplicated descriptor.  NB: some fcntl/ioctl flags
   * apply to the descriptor (e.g., O_CLOEXEC) and some apply to the
   * underlying open file entry (e.g., O_NONBLOCK), so changes by the
   * NaCl app could affect the service runtime.
   */
  for (i = 0; i < 3; ++i) {
    nhdp = malloc(sizeof *nhdp);
    if (NULL == nhdp) {
      NaClLog(LOG_FATAL,
              "NaClAppPrepareToLaunch: no memory for abstract descriptor %d\n",
              i);
    }
    NaClHostDescPosixDup(nhdp, descs[i], (0 == i) ? O_RDONLY : O_WRONLY);
    NaClSetDesc(nap, i, (struct NaClDesc *) NaClDescIoDescMake(nhdp));
  }
  retval = LOAD_OK;
done:
  NaClXMutexUnlock(&nap->mu);
  return retval;
}
