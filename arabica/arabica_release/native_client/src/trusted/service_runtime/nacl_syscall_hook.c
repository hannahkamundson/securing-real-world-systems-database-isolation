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
 * NaCl service run-time.
 */

#include "native_client/src/include/portability.h"
#include "native_client/src/include/nacl_platform.h"

#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "native_client/src/trusted/platform/nacl_log.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_handlers.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"

#include "native_client/src/trusted/service_runtime/include/sys/errno.h"
#include "native_client/src/trusted/service_runtime/include/bits/nacl_syscalls.h"

/*
 * Simple RPC support.  The default socket file descriptor is invalid.
 */
int NaClSrpcFileDescriptor = -1;

int NaClArtificialDelay = -1;


/*
 * The first syscall is from the NaCl module's main thread, and there
 * are no other user threads yet, so NACLDELAY check and the NACLCLOCK
 * usages are okay; when the NaCl module is multithreaded, the
 * variables they initialize are read-only.
 */
typedef uint64_t tick_t;
static tick_t get_ticks() {
  tick_t  t = 0;
#if NACL_WINDOWS
  uint32_t  t_high, t_low;

  __asm rdtsc;
  __asm mov t_high, edx;
  __asm mov t_low, eax;
  t = (((tick_t) t_high) << 32) | t_low;
#else
  asm volatile("rdtsc" : "=A" (t));
#endif
  return t;
}

void NaClMicroSleep(int microseconds) {
  static int    initialized = 0;
  static tick_t cpu_clock = 0;
  tick_t        now;
  tick_t        end;

  if (!initialized) {
    char *env = getenv("NACLCLOCK");
    if (NULL != env) {
      cpu_clock = strtoul(env, (char **) NULL, 0);
    }

    initialized = 1;
  }

  now = get_ticks();
  end = now + (cpu_clock * microseconds) / 1000000;
  NaClLog(5, "Now %"PRId64".  Waiting until %"PRId64".\n", now, end);
  while (get_ticks() < end)
    ;
}


NORETURN void NaClSyscallCSegHook(int32_t ldt_ix) {
  struct NaClAppThread      *natp = nacl_thread[ldt_ix];
  struct NaClApp            *nap = natp->nap;
  struct NaClThreadContext  *user = &natp->user;
  uintptr_t                 tramp_addr;
  uint32_t                  tramp_ret;
  uint32_t                  aligned_tramp_ret;
  uint32_t                  sysnum;

  /* esp must be okay for control to have gotten here */
#if !BENCHMARK
  NaClLog(4, "Entered NaClSyscallCSegHook\n");
  NaClLog(4, "user esp 0x%08x\n", user->esp);
#endif

  /*
   * on user stack:
   *  esp+0:  retaddr from lcall
   *  esp+4:  code seg from lcall
   *  esp+8:  retaddr from syscall wrapper
   *  esp+c:  ...
   */
  //jjs204 get the sys addr of the top of the user stack
  tramp_addr = NaClUserToSys(nap, user->esp);

  //jjs204 getting the value in esp + 0 to use as a tramp return (user space)
  tramp_ret = *(uint32_t *) tramp_addr;

  /*
   * return addr could have been tampered with by another thread, but
   * the only result would be a bad sysnum.
   */
  /*jjs204 we figure out which system call the untrusted code wanted by examining
   * which trampoline we came from */
  sysnum = (tramp_ret - NACL_SYSCALL_START_ADDR)
      >> NACL_SYSCALL_BLOCK_SHIFT;

  /* jjs204 
 * NaClLog(4, "tramp_ret 0x%08x\n", tramp_ret);
 * NaClLog(4, "mem_start 0x%08x\n", nap->mem_start);
 * NaClLog(4, "SYSCALL_START 0x%08x\n", NACL_SYSCALL_START_ADDR);
 * NaClLog(4, "BLOCK_SHIFT %d\n", NACL_SYSCALL_BLOCK_SHIFT);*/

#if !BENCHMARK
  NaClLog(4, "system call %d\n", sysnum);
#endif
  /*
   * keep tramp_ret in user addr; do not bother to ensure tramp_addr +
   * 8 is valid, since even if we loaded two NaClApps next to each
   * other this would just load the trampoline code, or hit the
   * inaccessible page for NULL pointer detection (once we get that
   * implemented).  if it is not a valid address, we would just crash.
   */
  tramp_ret = *(uint32_t *) (tramp_addr + 8);
  if (0 != nap->xlate_base) {
    /*
     * ensure that tramp_ret value is ok.  no need to ensure that this
     * is in the app's address space, since the syscall return will
     * just result in a fault after we reconstitute the sandbox and
     * attempt to pass control to this address.
     */
    aligned_tramp_ret = tramp_ret & ~(nap->align_boundary - 1);
    if (tramp_ret != aligned_tramp_ret) {
      NaClLog(LOG_FATAL, ("NaClSyscallCSegHook: tramp_ret infinite loop:"
                          " %08x != %08x\nMake sure NaCl SDK and sel_ldr"
                          " agree on alignment (ELF header of NaCl app"
                          " claims alignment is %d).\n"),
              tramp_ret, aligned_tramp_ret, nap->align_boundary);
    }
    tramp_ret = aligned_tramp_ret;
  }

  //jjs204 Clear out the information on the stack associated with the last call
  //(see above for contents of stack)
  user->esp += 0xc;  /* call, lcall */
  if (sysnum >= NACL_MAX_SYSCALLS) {
    NaClLog(2, "INVALID system call %d\n", sysnum);
    natp->sysret = -NACL_ABI_EINVAL;
  } else {
#if !BENCHMARK
    NaClLog(4, "making system call %d, handler 0x%08"PRIxPTR"\n",
            sysnum, (uintptr_t) nacl_syscall[sysnum].handler);
#endif
    
    //jjs204 record the sys addr of the user stack
    natp->x_esp = (uint32_t *) (tramp_addr + 0xc);

    //jjs204 make the system call and record the result
    natp->sysret = (*nacl_syscall[sysnum].handler)(natp);
  }
#if !BENCHMARK
  NaClLog(4,
          ("returning from system call %d, return value %"PRId32
           " (0x%"PRIx32")\n"),
          sysnum, natp->sysret, natp->sysret);

  NaClLog(4, "return target 0x%08"PRIx32"\n", tramp_ret);
  NaClLog(4, "user esp 0x%08"PRIx32"\n", user->esp);
#endif
  if (-1 == NaClArtificialDelay) {
    char *delay = getenv("NACLDELAY");
    if (NULL != delay) {
      NaClArtificialDelay = strtol(delay, (char **) NULL, 0);
      NaClLog(0, "ARTIFICIAL DELAY %d us\n", NaClArtificialDelay);
    } else {
      NaClArtificialDelay = 0;
    }
  }
  if (0 != NaClArtificialDelay) {
    NaClMicroSleep(NaClArtificialDelay);
  }
  NaClSwitchToApp(natp, tramp_ret);
 /* NOTREACHED */

  fprintf(stderr, "NORETURN NaClSwitchToApp returned!?!\n");
  abort();
}
