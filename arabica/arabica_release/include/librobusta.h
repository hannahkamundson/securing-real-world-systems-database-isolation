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

#ifndef _LIBROBUSTA_JVM_H_
#define _LIBROBUSTA_JVM_H_

//#include "robusta_jni.h"
#include <jni.h>
#include <signal.h>

#ifdef SANDBOX_MEASUREMENT
	#include <sys/time.h>
#endif

typedef struct NaClAppThread * NATP_PTR;

#ifndef LISTH
typedef struct _LIST LIST;
#endif

//natp table
extern LIST * gl_natp_list;
extern pthread_mutex_t gl_natp_list_lock;

// the number of sandbox
#ifndef NUM_SANDBOX
#define NUM_SANDBOX 5
#endif

#ifndef _NPITEM_H_
#define _NPITEM_H_

typedef struct _npitem
{
  JNIEnv * env;
  NATP_PTR natp[NUM_SANDBOX];
} npitem;

#endif /* _NPITEM_H_ */

// a returning struct for multi-sandboxing loadLib and loadSym.
// it contains a pointer (of a handle or a symbol) and to which
// sandbox it belongs. Hence we call it ptsb (pointer and sandbox)
#ifndef _PTSB_H_
#define _PTSB_H_

typedef struct _ptsb
{
  void * pointer;
  int sb;
} ptsb;

#endif /* _PTSB_H_ */

// Jinn init mark for libjava.so
extern int bda_init_finish;

// Store the xlate_base for libjava.so
extern int convert_base;

//JVM Service Functions
//-------------------------------------------------------------------------------------------------------------------
extern void initRobusta();

extern void robusta_initJNIvars(JNIEnv * env);

extern ptsb loadLib(JNIEnv *env, char * filename);

extern void * loadSym(void * handle, char * symbolname, int sid);

extern void call_in_onload(void * addr);

extern void call_in(void * addr);

extern double * call_in_float(void * addr);

extern double * call_in_float_arg(void * addr);

extern long long * call_in_long(void * addr);

extern long long call_in_normal_float_arg(void * addr);

extern NATP_PTR _Z17robusta_init_natpv();

extern int robusta_methodUsesStubLibraries(char * method_name);
//extern int robusta_classUsesSandboxedLibraries(char * class_name);
//-------------------------------------------------------------------------------------------------------------------

//NACL Service Functions
//-------------------------------------------------------------------------------------------------------------------
extern void robusta_createMethodIDNode(LIST * methodID_list, JNIEnv * env, jclass clazz, jmethodID mid, const char * sys_sig);

//***returns -1 if the methodID was not found***
extern int robusta_findParameterCount(LIST * methodID_list, jmethodID mid);

extern void robusta_createFieldIDNode(LIST * methodID_list, JNIEnv * env, jclass clazz, jfieldID fid, const char * sys_sig);

extern void robusta_verifySetFieldOp(LIST * field_id_list, JNIEnv * env, jclass clazz, jfieldID fid);

extern int robusta_verifyOperation(JNIEnv * env, char * path, char * optype);

void robusta_addPath(char * path, int fd);

void robusta_removePath(int fd);

char * robusta_findPath(int fd);
//-------------------------------------------------------------------------------------------------------------------

//measurement instrumentation
#ifdef SANDBOX_MEASUREMENT
	jclass robusta_watcher;
	jmethodID update_sand;

	struct timeval before;
	struct timeval after;

	typedef struct timeval * TIMEVAL_PTR;

	void updateTime(JNIEnv * env, TIMEVAL_PTR before, TIMEVAL_PTR after);
#endif /* SANDBOX_MEASUREMENT */

#endif /* _LIBROBUSTA_JVM_H_ */
