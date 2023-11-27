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

#include <stdio.h>
#include "sblocks.h"

void initdynlinkwrappers(void * dlOpen, void * dlSym, void * method_dispatch, void * method_dispatch_float, void * dynlink_lazy_allocate_tdb_stack, void * safeExit)
{
	//printf("[sblocks.c: initdynlinwrappers:] Initializing dynlink wrappers.\n");
	//fflush(stdout);
  (NACL_SYSCALL(initdynlinkwrappers))(dlOpen, dlSym, method_dispatch, method_dispatch_float, dynlink_lazy_allocate_tdb_stack, safeExit);
}

void outofjail(int fnum, int * retval)
{
	//printf("[sblocks.c: outofjail:] Return value is: %lld\n", retval);
	//fflush(stdout);
	(NACL_SYSCALL(outofjail))(fnum, retval);
}

//a hack for the thread test programs (since I couldn't get the regular sleep() function to work...)
void robustasleep(int seconds)
{
	(NACL_SYSCALL(robustasleep))(seconds);
}
