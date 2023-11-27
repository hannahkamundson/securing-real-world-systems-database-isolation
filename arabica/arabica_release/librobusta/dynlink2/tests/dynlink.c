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
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "robusta_jni.h"
#include "pthread_types.h"

#define DYNLINK_DEBUG 1

//jjs204
//later, put this in a header file
//remember, these functions are starting blocks, so they're in libblocks.so
extern void outofjail(int fnum, void * retval);
extern void initdynlinkwrappers(void *, void *, void *, void *, void *, void *);

//mes310 add another interface for JavaVM pointer
struct JNINativeInterface_ func_table;
struct JNIInvokeInterface_ vm_table;

void *dlopenWrapper (char * filename, int flag) {

  #ifdef DYNLINK_DEBUG
  printf("[dynlink.c: dlopenwrapper:] filename=%s\tflag=%d\n", filename, flag);
  fflush(stdout);
  #endif

  void * handle = dlopen(filename, flag);

  //mes310
  #ifdef DYNLINK_DEBUG
  printf("[dynlink.c: dlopenwrapper:] filename: %s, handle: %p\n", filename, handle);
  printf("[dynlink.c: dlopenwrapper:] error: %s\n", dlerror());
  fflush(stdout);
  #endif
  outofjail(5, (int *) &handle);
}

void  *dlsymWrapper (void *handle, const char *symbol) {
  char *error;

  #ifdef DYNLINK_DEBUG
  printf("[dynlink.c: dlsymWrapper:] handle=0x%x\tsymbol=%s\n", handle, symbol);
  fflush(stdout);
  #endif

  void *fn = dlsym(handle, symbol);

  if((error=dlerror())!= NULL)
  {
	#ifdef DYNLINK_DEBUG
	printf("[Dynlink: dlsymWrapper: ***FATAL***] %s\n", error);
	fflush(stdout);
	#endif
	
	fn =(void *)-1;
  }

  #ifdef DYNLINK_DEBUG
  printf("[Dynlink: dlsymWrapper:] symbol addr discovered: %p\n", fn);  
  fflush(stdout);
  #endif

  outofjail(5, (int *) &fn);
}

void * method_dispatch (long long int (*addr)())
//void * method_dispatch ()
{
	long long int result;

	/*printf("[dynlink.c: method_dispatch:] jumping to address %p\n", addr);
	printf("--------------------------------------------------------------\n");
	printf("--------------------------------------------------------------\n");
	fflush(stdout);*/

	//Back out the current stack frame so that the method can access
	//the java parameters sitting on the stack.
	//(this could be compiler-dependent...?)
	//NOTE: this causes the local variable "result" to be overwritten
	//by the local variables in the upcoming method call
	//(this shouldn't be a problem since "result" isn't used until after
	//the call!)
	//Note: the value 0x34 was obtained by disassembling dynlink, looking
	//at the compilers allocation for the current stack frame (0x28)
	//Then, we need also to back out the return address from calling the actual
	//function (0x4), the false return address (0x4) placed to allign 
	//method_dispatch. The grand total of backout in this case
	//would be (0x34)
	
        asm("add $0x24, %%esp"::);
	//asm("add $0x34, %%esp"::);

	result = (*addr)();
	/*	printf ("my result=%lld, =%e\n",  result, (double) result);
		fflush(stdout); */

	//result = 100;

	//restore the esp to the way it was before the call...
	//(so we don't lose the space allocated for result)
	asm("sub $0x24, %%esp"::);
	//asm("sub $0x34, %%esp"::);

	/*printf("--------------------------------------------------------------\n");
	printf("--------------------------------------------------------------\n");	
	printf("[dynlink.c: method_dispatch:] the result is %lld:\n", result);
	fflush(stdout);*/

	// switching out
	outofjail(6, &result);
}

//mes310
//Duplicate a method_dispatch for functions with floating point return value
void * method_dispatch_float (double (*addr)())
//void * method_dispatch ()
{
	double result;

	/*printf("[dynlink.c: method_dispatch:] jumping to address %p\n", addr);
	printf("--------------------------------------------------------------\n");
	printf("--------------------------------------------------------------\n");
	fflush(stdout);*/

	//Back out the current stack frame so that the method can access
	//the java parameters sitting on the stack.
	//(this could be compiler-dependent...?)
	//NOTE: this causes the local variable "result" to be overwritten
	//by the local variables in the upcoming method call
	//(this shouldn't be a problem since "result" isn't used until after
	//the call!)
	//Note: the value 0x34 was obtained by disassembling dynlink, looking
	//at the compilers allocation for the current stack frame (0x28)
	//Then, we need also to back out the return address from calling the actual
	//function (0x4), the false return address (0x4) placed to allign 
	//method_dispatch. The grand total of backout in this case
	//would be (0x34)
	
        asm("add $0x24, %%esp"::);
	//asm("add $0x34, %%esp"::);

	result = (*addr)();

	//restore the esp to the way it was before the call...
	//(so we don't lose the space allocated for result)
	asm("sub $0x24, %%esp"::);
	//asm("sub $0x34, %%esp"::);

	/*printf("--------------------------------------------------------------\n");
	printf("--------------------------------------------------------------\n");	
	printf("[dynlink.c: method_dispatch:] the result is %lld:\n", result);
	fflush(stdout);*/

	// switching out
	outofjail(6, &result);
}


int * dynlink_lazy_allocate_tdb_stack()
{
	//nc_thread_descriptor_t * new_tdb;
	//int tdb_size;
	char *thread_stack = NULL;
	void * esp;

#ifdef DYNLINK_DEBUG
	printf("[dynlink.c: dynlink_lazy_allocate_tdb_stack:] Allocating a stack...\n");
	fflush(stdout);
#endif

	/*int ret[3];
	int size = __nacl_tls_combined_size(sizeof(nc_thread_descriptor_t), 1);
	void * tls_tdb = malloc(size);
	memset(tls_tdb, 0, size);
	__nacl_tls_data_bss_initialize_from_template(tls_tdb);
	new_tdb = (nc_thread_descriptor_t *) __nacl_tls_tdb_start(tls_tdb);*/

	//nc_tdb_init(new_tdb, NULL);
	//new_tdb->tls_node = tls_tdb;

	//currently, we don't worry about tls, we just allocate a tdb and stack
	
	//i ignored their block-reclaiming algorithm for now, since we won't be
	//creating and destroying that many threads in our initial prototype
	//tdb_size = __nacl_tls_combined_size(sizeof(nc_thread_descriptor_t), 1)+1;
	//new_tdb = (nc_thread_descriptor_t *) malloc(tdb_size);
	//new_tdb = malloc(1);

	//if the descriptor is ever used, then initialize it here
	//for now, we do nothing with it...
	
	//__nacl_thrtead_stack_size is defined in tls.h and implemented in tls.c
	thread_stack = (char *) malloc(__nacl_thread_stack_size(1));

	//this calculation came from nc_thread.c -- see their comments
	esp = (void *) (thread_stack + __nacl_thread_stack_size(0) - 4);

	//package the esp and tdb in an array to return
	//ret[0] = (int) new_tdb;
	//ret[1] = (int) esp;
	//ret[2] = 1;
	//ret[2] = sizeof(nc_thread_descriptor_t);
	//ret[2] =  tdb_size;

	outofjail(5,(int*) &esp);
	
	//should never get here

	return 0;
}

void * safeExit()
{
	outofjail(4, 1);
}

int main()
{
	initdynlinkwrappers(&dlopenWrapper, &dlsymWrapper, &method_dispatch,
			    &method_dispatch_float,
			    &dynlink_lazy_allocate_tdb_stack, &safeExit);
	//initialize the address of all the JNIWrapper functions
	//printf("[dynlink.c: main:] Intializing JNI wrappers...\n");
	initJNIWrapperFunctions();
	//mes310
	initVMWrapperFunctions();

	//allocate space on the native heap for a false JNIEnv pointer
	//our JNIEnv_Wrapper lives on the heap
	//the pointer to this lives on the stack
	JNIEnv *env = malloc(sizeof(JNIEnv));
	// try allocating another javavm pointer
	JavaVM *vm = malloc(sizeof(JavaVM));

	//printf("[dynlink: main] env: %p, vm: %p\n", env, vm);
	
	//our function struct is global
	(*env) = &func_table;
	(*vm) = &vm_table;

	printf("[dynlink: main] addr of env: %p, addr of vm: %p\n", &env, &vm);
	fflush(stdout);
	
	//pass back the address of the JNIEnv_Wrapper_ heap object
	outofjail(5,(int *) &env);  
	//NACL_OUT_OF_JAIL(1, 0);

	printf("[dynlink.c: main:] ***FATAL*** Out of jail FAILED\n");
	fflush(stdout);

	return 0;
}
