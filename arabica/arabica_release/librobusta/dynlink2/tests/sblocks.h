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

#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <nacl_config.h>
#include <include/bits/nacl_syscalls.h>

#include "robusta_jni.h"

#define NACL_SYSCALL(s) ((TYPE_nacl_ ## s) NACL_SYSCALL_ADDR(NACL_sys_ ## s))

typedef void *(*TYPE_nacl_initdynlinkwrappers) (void * dlOpen, void * dlSym, void * method_dispatch, void * method_dispatch_float, void * dynlink_lazy_allocate_tdb_stack, void * safeExit);
typedef void *(*TYPE_nacl_outofjail) (int fnum, int * retval);

typedef void *(*TYPE_nacl_robustasleep) (int seconds);

//for JavaVM calls
typedef jint (*TYPE_nacl_vm_call_getenv)(JavaVM * vm, void ** penv, jint version);

//for JNI calls
typedef jint (*TYPE_nacl_jni_call_getversion)(JNIEnv * env);
typedef jclass (*TYPE_nacl_jni_call_defineclass)(JNIEnv * env, const char * p1, jobject p2, const jbyte * p3, jsize p4);
typedef jclass (*TYPE_nacl_jni_call_findclass)(JNIEnv * env, const char * p1);
typedef jmethodID (*TYPE_nacl_jni_call_fromreflectedmethod)(JNIEnv * env, jobject p1);
typedef jfieldID (*TYPE_nacl_jni_call_fromreflectedfield)(JNIEnv * env, jobject p1); /* 5 */
typedef jobject (*TYPE_nacl_jni_call_toreflectedmethod)(JNIEnv * env, jclass p1, jmethodID p2, jboolean p3);
typedef jclass (*TYPE_nacl_jni_call_getsuperclass)(JNIEnv * env, jclass p1);
typedef jboolean (*TYPE_nacl_jni_call_isassignablefrom)(JNIEnv * env, jclass p1, jclass p2);
typedef jobject (*TYPE_nacl_jni_call_toreflectedfield)(JNIEnv * env, jclass p1, jfieldID p2, jboolean p3);
typedef jint (*TYPE_nacl_jni_call_throw)(JNIEnv * env, jthrowable p1); /* 10 */
typedef jint (*TYPE_nacl_jni_call_thrownew)(JNIEnv * env, jclass p1, const char * p2);
typedef jthrowable (*TYPE_nacl_jni_call_exceptionoccurred)(JNIEnv * env);
typedef void (*TYPE_nacl_jni_call_exceptiondescribe)(JNIEnv * env);
typedef void (*TYPE_nacl_jni_call_exceptionclear)(JNIEnv * env);
typedef void (*TYPE_nacl_jni_call_fatalerror)(JNIEnv * env, const char * p1); /* 15 */
typedef jint (*TYPE_nacl_jni_call_pushlocalframe)(JNIEnv * env, jint p1);
typedef jobject (*TYPE_nacl_jni_call_poplocalframe)(JNIEnv * env, jobject p1);
typedef jobject (*TYPE_nacl_jni_call_newglobalref)(JNIEnv * env, jobject p1);
typedef void (*TYPE_nacl_jni_call_deleteglobalref)(JNIEnv * env, jobject p1);
typedef void (*TYPE_nacl_jni_call_deletelocalref)(JNIEnv * env, jobject p1); /* 20 */
typedef jboolean (*TYPE_nacl_jni_call_issameobject)(JNIEnv * env, jobject p1, jobject p2);
typedef jobject (*TYPE_nacl_jni_call_newlocalref)(JNIEnv * env, jobject p1);
typedef jint (*TYPE_nacl_jni_call_ensurelocalcapacity)(JNIEnv * env, jint p1);
typedef jobject (*TYPE_nacl_jni_call_allocobject)(JNIEnv * env, jclass p1);
typedef jobject (*TYPE_nacl_jni_call_newobject)(JNIEnv * env, jclass p1, jmethodID p2, ...); /* 25 */
typedef jobject (*TYPE_nacl_jni_call_newobjectv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jobject (*TYPE_nacl_jni_call_newobjecta)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jclass (*TYPE_nacl_jni_call_getobjectclass)(JNIEnv * env, jobject p1);
typedef jboolean (*TYPE_nacl_jni_call_isinstanceof)(JNIEnv * env, jobject p1, jclass p2);
typedef jmethodID (*TYPE_nacl_jni_call_getmethodid)(JNIEnv * env, jclass p1, const char * p2, const char * p3); /* 30 */
typedef jobject (*TYPE_nacl_jni_call_callobjectmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jobject (*TYPE_nacl_jni_call_callobjectmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jobject (*TYPE_nacl_jni_call_callobjectmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jboolean (*TYPE_nacl_jni_call_callbooleanmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jboolean (*TYPE_nacl_jni_call_callbooleanmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3); /* 35 */
typedef jboolean (*TYPE_nacl_jni_call_callbooleanmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jbyte (*TYPE_nacl_jni_call_callbytemethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jbyte (*TYPE_nacl_jni_call_callbytemethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jbyte (*TYPE_nacl_jni_call_callbytemethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jchar (*TYPE_nacl_jni_call_callcharmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...); /* 40 */
typedef jchar (*TYPE_nacl_jni_call_callcharmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jchar (*TYPE_nacl_jni_call_callcharmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jshort (*TYPE_nacl_jni_call_callshortmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jshort (*TYPE_nacl_jni_call_callshortmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jshort (*TYPE_nacl_jni_call_callshortmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3); /* 45 */
typedef jint (*TYPE_nacl_jni_call_callintmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jint (*TYPE_nacl_jni_call_callintmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jint (*TYPE_nacl_jni_call_callintmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jlong (*TYPE_nacl_jni_call_calllongmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jlong (*TYPE_nacl_jni_call_calllongmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3); /* 50 */
typedef jlong (*TYPE_nacl_jni_call_calllongmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jfloat (*TYPE_nacl_jni_call_callfloatmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef jfloat (*TYPE_nacl_jni_call_callfloatmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jfloat (*TYPE_nacl_jni_call_callfloatmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef jdouble (*TYPE_nacl_jni_call_calldoublemethod)(JNIEnv * env, jobject p1, jmethodID p2, ...); /* 55 */
typedef jdouble (*TYPE_nacl_jni_call_calldoublemethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef jdouble (*TYPE_nacl_jni_call_calldoublemethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
typedef void (*TYPE_nacl_jni_call_callvoidmethod)(JNIEnv * env, jobject p1, jmethodID p2, ...);
typedef void (*TYPE_nacl_jni_call_callvoidmethodv)(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
typedef void (*TYPE_nacl_jni_call_callvoidmethoda)(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3); /* 60 */
typedef jobject (*TYPE_nacl_jni_call_callnonvirtualobjectmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jobject (*TYPE_nacl_jni_call_callnonvirtualobjectmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jobject (*TYPE_nacl_jni_call_callnonvirtualobjectmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jboolean (*TYPE_nacl_jni_call_callnonvirtualbooleanmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jboolean (*TYPE_nacl_jni_call_callnonvirtualbooleanmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4); /* 65 */
typedef jboolean (*TYPE_nacl_jni_call_callnonvirtualbooleanmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jbyte (*TYPE_nacl_jni_call_callnonvirtualbytemethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jbyte (*TYPE_nacl_jni_call_callnonvirtualbytemethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jbyte (*TYPE_nacl_jni_call_callnonvirtualbytemethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jchar (*TYPE_nacl_jni_call_callnonvirtualcharmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...); /* 70 */
typedef jchar (*TYPE_nacl_jni_call_callnonvirtualcharmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jchar (*TYPE_nacl_jni_call_callnonvirtualcharmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jshort (*TYPE_nacl_jni_call_callnonvirtualshortmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jshort (*TYPE_nacl_jni_call_callnonvirtualshortmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jshort (*TYPE_nacl_jni_call_callnonvirtualshortmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4); /* 75 */
typedef jint (*TYPE_nacl_jni_call_callnonvirtualintmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jint (*TYPE_nacl_jni_call_callnonvirtualintmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jint (*TYPE_nacl_jni_call_callnonvirtualintmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jlong (*TYPE_nacl_jni_call_callnonvirtuallongmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jlong (*TYPE_nacl_jni_call_callnonvirtuallongmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4); /* 80 */
typedef jlong (*TYPE_nacl_jni_call_callnonvirtuallongmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jfloat (*TYPE_nacl_jni_call_callnonvirtualfloatmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef jfloat (*TYPE_nacl_jni_call_callnonvirtualfloatmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jfloat (*TYPE_nacl_jni_call_callnonvirtualfloatmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef jdouble (*TYPE_nacl_jni_call_callnonvirtualdoublemethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...); /* 85 */
typedef jdouble (*TYPE_nacl_jni_call_callnonvirtualdoublemethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef jdouble (*TYPE_nacl_jni_call_callnonvirtualdoublemethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
typedef void (*TYPE_nacl_jni_call_callnonvirtualvoidmethod)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
typedef void (*TYPE_nacl_jni_call_callnonvirtualvoidmethodv)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
typedef void (*TYPE_nacl_jni_call_callnonvirtualvoidmethoda)(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4); /* 90 */
typedef jfieldID (*TYPE_nacl_jni_call_getfieldid)(JNIEnv * env, jclass p1, const char * p2, const char * p3);
typedef jobject (*TYPE_nacl_jni_call_getobjectfield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jboolean (*TYPE_nacl_jni_call_getbooleanfield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jbyte (*TYPE_nacl_jni_call_getbytefield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jchar (*TYPE_nacl_jni_call_getcharfield)(JNIEnv * env, jobject p1, jfieldID p2); /* 95 */
typedef jshort (*TYPE_nacl_jni_call_getshortfield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jint (*TYPE_nacl_jni_call_getintfield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jlong (*TYPE_nacl_jni_call_getlongfield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jfloat (*TYPE_nacl_jni_call_getfloatfield)(JNIEnv * env, jobject p1, jfieldID p2);
typedef jdouble (*TYPE_nacl_jni_call_getdoublefield)(JNIEnv * env, jobject p1, jfieldID p2); /* 100 */
typedef void (*TYPE_nacl_jni_call_setobjectfield)(JNIEnv * env, jobject p1, jfieldID p2, jobject p3);
typedef void (*TYPE_nacl_jni_call_setbooleanfield)(JNIEnv * env, jobject p1, jfieldID p2, jboolean p3);
typedef void (*TYPE_nacl_jni_call_setbytefield)(JNIEnv * env, jobject p1, jfieldID p2, jbyte p3);
typedef void (*TYPE_nacl_jni_call_setcharfield)(JNIEnv * env, jobject p1, jfieldID p2, jchar p3);
typedef void (*TYPE_nacl_jni_call_setshortfield)(JNIEnv * env, jobject p1, jfieldID p2, jshort p3); /* 105 */
typedef void (*TYPE_nacl_jni_call_setintfield)(JNIEnv * env, jobject p1, jfieldID p2, jint p3);
typedef void (*TYPE_nacl_jni_call_setlongfield)(JNIEnv * env, jobject p1, jfieldID p2, jlong p3);
typedef void (*TYPE_nacl_jni_call_setfloatfield)(JNIEnv * env, jobject p1, jfieldID p2, jfloat p3);
typedef void (*TYPE_nacl_jni_call_setdoublefield)(JNIEnv * env, jobject p1, jfieldID p2, jdouble p3);
typedef jmethodID (*TYPE_nacl_jni_call_getstaticmethodid)(JNIEnv * env, jclass p1, const char * p2, const char * p3); /* 110 */
typedef jobject (*TYPE_nacl_jni_call_callstaticobjectmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jobject (*TYPE_nacl_jni_call_callstaticobjectmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jobject (*TYPE_nacl_jni_call_callstaticobjectmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jboolean (*TYPE_nacl_jni_call_callstaticbooleanmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jboolean (*TYPE_nacl_jni_call_callstaticbooleanmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3); /* 115 */
typedef jboolean (*TYPE_nacl_jni_call_callstaticbooleanmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jbyte (*TYPE_nacl_jni_call_callstaticbytemethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jbyte (*TYPE_nacl_jni_call_callstaticbytemethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jbyte (*TYPE_nacl_jni_call_callstaticbytemethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jchar (*TYPE_nacl_jni_call_callstaticcharmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...); /* 120 */
typedef jchar (*TYPE_nacl_jni_call_callstaticcharmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jchar (*TYPE_nacl_jni_call_callstaticcharmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jshort (*TYPE_nacl_jni_call_callstaticshortmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jshort (*TYPE_nacl_jni_call_callstaticshortmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jshort (*TYPE_nacl_jni_call_callstaticshortmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3); /* 125 */
typedef jint (*TYPE_nacl_jni_call_callstaticintmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jint (*TYPE_nacl_jni_call_callstaticintmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jint (*TYPE_nacl_jni_call_callstaticintmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jlong (*TYPE_nacl_jni_call_callstaticlongmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jlong (*TYPE_nacl_jni_call_callstaticlongmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3); /* 130 */
typedef jlong (*TYPE_nacl_jni_call_callstaticlongmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jfloat (*TYPE_nacl_jni_call_callstaticfloatmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef jfloat (*TYPE_nacl_jni_call_callstaticfloatmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jfloat (*TYPE_nacl_jni_call_callstaticfloatmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef jdouble (*TYPE_nacl_jni_call_callstaticdoublemethod)(JNIEnv * env, jclass p1, jmethodID p2, ...); /* 135 */
typedef jdouble (*TYPE_nacl_jni_call_callstaticdoublemethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef jdouble (*TYPE_nacl_jni_call_callstaticdoublemethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
typedef void (*TYPE_nacl_jni_call_callstaticvoidmethod)(JNIEnv * env, jclass p1, jmethodID p2, ...);
typedef void (*TYPE_nacl_jni_call_callstaticvoidmethodv)(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
typedef void (*TYPE_nacl_jni_call_callstaticvoidmethoda)(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3); /* 140 */
typedef jfieldID (*TYPE_nacl_jni_call_getstaticfieldid)(JNIEnv * env, jclass p1, const char * p2, const char * p3);
typedef jobject (*TYPE_nacl_jni_call_getstaticobjectfield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jboolean (*TYPE_nacl_jni_call_getstaticbooleanfield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jbyte (*TYPE_nacl_jni_call_getstaticbytefield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jchar (*TYPE_nacl_jni_call_getstaticcharfield)(JNIEnv * env, jclass p1, jfieldID p2); /* 145 */
typedef jshort (*TYPE_nacl_jni_call_getstaticshortfield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jint (*TYPE_nacl_jni_call_getstaticintfield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jlong (*TYPE_nacl_jni_call_getstaticlongfield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jfloat (*TYPE_nacl_jni_call_getstaticfloatfield)(JNIEnv * env, jclass p1, jfieldID p2);
typedef jdouble (*TYPE_nacl_jni_call_getstaticdoublefield)(JNIEnv * env, jclass p1, jfieldID p2); /* 150 */
typedef void (*TYPE_nacl_jni_call_setstaticobjectfield)(JNIEnv * env, jclass p1, jfieldID p2, jobject p3);
typedef void (*TYPE_nacl_jni_call_setstaticbooleanfield)(JNIEnv * env, jclass p1, jfieldID p2, jboolean p3);
typedef void (*TYPE_nacl_jni_call_setstaticbytefield)(JNIEnv * env, jclass p1, jfieldID p2, jbyte p3);
typedef void (*TYPE_nacl_jni_call_setstaticcharfield)(JNIEnv * env, jclass p1, jfieldID p2, jchar p3);
typedef void (*TYPE_nacl_jni_call_setstaticshortfield)(JNIEnv * env, jclass p1, jfieldID p2, jshort p3); /* 155 */
typedef void (*TYPE_nacl_jni_call_setstaticintfield)(JNIEnv * env, jclass p1, jfieldID p2, jint p3);
typedef void (*TYPE_nacl_jni_call_setstaticlongfield)(JNIEnv * env, jclass p1, jfieldID p2, jlong p3);
typedef void (*TYPE_nacl_jni_call_setstaticfloatfield)(JNIEnv * env, jclass p1, jfieldID p2, jfloat p3);
typedef void (*TYPE_nacl_jni_call_setstaticdoublefield)(JNIEnv * env, jclass p1, jfieldID p2, jdouble p3);
typedef jstring (*TYPE_nacl_jni_call_newstring)(JNIEnv * env, const jchar * p1, jsize p2); /* 160 */
typedef jsize (*TYPE_nacl_jni_call_getstringlength)(JNIEnv * env, jstring p1);
typedef const jchar * (*TYPE_nacl_jni_call_getstringchars)(JNIEnv * env, jstring p1, jboolean * p2);
typedef void (*TYPE_nacl_jni_call_releasestringchars)(JNIEnv * env, jstring p1, const jchar * p2);
typedef jstring (*TYPE_nacl_jni_call_newstringutf)(JNIEnv * env, const char * p1);
typedef jsize (*TYPE_nacl_jni_call_getstringutflength)(JNIEnv * env, jstring p1); /* 165 */
typedef const char * (*TYPE_nacl_jni_call_getstringutfchars)(JNIEnv * env, jstring p1, jboolean * p2);
typedef void (*TYPE_nacl_jni_call_releasestringutfchars)(JNIEnv * env, jstring p1, const char * p2);
typedef jsize (*TYPE_nacl_jni_call_getarraylength)(JNIEnv * env, jarray p1);
typedef jobjectArray (*TYPE_nacl_jni_call_newobjectarray)(JNIEnv * env, jsize p1, jclass p2, jobject p3);
typedef jobject (*TYPE_nacl_jni_call_getobjectarrayelement)(JNIEnv * env, jobjectArray p1, jsize p2); /* 170 */
typedef void (*TYPE_nacl_jni_call_setobjectarrayelement)(JNIEnv * env, jobjectArray p1, jsize p2, jobject p3); 
typedef jbooleanArray (*TYPE_nacl_jni_call_newbooleanarray)(JNIEnv * env, jsize p1);
typedef jbyteArray (*TYPE_nacl_jni_call_newbytearray)(JNIEnv * env, jsize p1);
typedef jcharArray (*TYPE_nacl_jni_call_newchararray)(JNIEnv * env, jsize p1);
typedef jshortArray (*TYPE_nacl_jni_call_newshortarray)(JNIEnv * env, jsize p1); /* 175 */
typedef jintArray (*TYPE_nacl_jni_call_newintarray)(JNIEnv * env, jsize p1);
typedef jlongArray (*TYPE_nacl_jni_call_newlongarray)(JNIEnv * env, jsize p1);
typedef jfloatArray (*TYPE_nacl_jni_call_newfloatarray)(JNIEnv * env, jsize p1);
typedef jdoubleArray (*TYPE_nacl_jni_call_newdoublearray)(JNIEnv * env, jsize p1);
typedef jboolean * (*TYPE_nacl_jni_call_getbooleanarrayelements)(JNIEnv * env, jbooleanArray p1, jboolean * p2); /* 180 */
typedef jbyte * (*TYPE_nacl_jni_call_getbytearrayelements)(JNIEnv * env, jbyteArray p1, jboolean * p2);
typedef jchar * (*TYPE_nacl_jni_call_getchararrayelements)(JNIEnv * env, jcharArray p1, jboolean * p2);
typedef jshort * (*TYPE_nacl_jni_call_getshortarrayelements)(JNIEnv * env, jshortArray p1, jboolean * p2);
typedef jint * (*TYPE_nacl_jni_call_getintarrayelements)(JNIEnv * env, jintArray p1, jboolean * p2);
typedef jlong * (*TYPE_nacl_jni_call_getlongarrayelements)(JNIEnv * env, jlongArray p1, jboolean * p2); /* 185 */
typedef jfloat * (*TYPE_nacl_jni_call_getfloatarrayelements)(JNIEnv * env, jfloatArray p1, jboolean * p2);
typedef jdouble * (*TYPE_nacl_jni_call_getdoublearrayelements)(JNIEnv * env, jdoubleArray p1, jboolean * p2);
typedef void (*TYPE_nacl_jni_call_releasebooleanarrayelements)(JNIEnv * env, jbooleanArray p1, jboolean * p2, jint p3);
typedef void (*TYPE_nacl_jni_call_releasebytearrayelements)(JNIEnv * env, jbyteArray p1, jbyte * p2, jint p3);
typedef void (*TYPE_nacl_jni_call_releasechararrayelements)(JNIEnv * env, jcharArray p1, jchar * p2, jint p3); /* 190 */
typedef void (*TYPE_nacl_jni_call_releaseshortarrayelements)(JNIEnv * env, jshortArray p1, jshort * p2, jint p3);
typedef void (*TYPE_nacl_jni_call_releaseintarrayelements)(JNIEnv * env, jintArray p1, jint * p2, jint p3);
typedef void (*TYPE_nacl_jni_call_releaselongarrayelements)(JNIEnv * env, jlongArray p1, jlong * p2, jint p3);
typedef void (*TYPE_nacl_jni_call_releasefloatarrayelements)(JNIEnv * env, jfloatArray p1, jfloat * p2, jint p3);
typedef void (*TYPE_nacl_jni_call_releasedoublearrayelements)(JNIEnv * env, jdoubleArray p1, jdouble * p2, jint p3); /* 195 */
typedef void (*TYPE_nacl_jni_call_getbooleanarrayregion)(JNIEnv * env, jbooleanArray p1, jsize p2, jsize p3, jboolean * p4);
typedef void (*TYPE_nacl_jni_call_getbytearrayregion)(JNIEnv * env, jbyteArray p1, jsize p2, jsize p3, jbyte * p4);
typedef void (*TYPE_nacl_jni_call_getchararrayregion)(JNIEnv * env, jcharArray p1, jsize p2, jsize p3, jchar * p4);
typedef void (*TYPE_nacl_jni_call_getshortarrayregion)(JNIEnv * env, jshortArray p1, jsize p2, jsize p3, jshort * p4);
typedef void (*TYPE_nacl_jni_call_getintarrayregion)(JNIEnv * env, jintArray p1, jsize p2, jsize p3, jint * p4); /* 200 */
typedef void (*TYPE_nacl_jni_call_getlongarrayregion)(JNIEnv * env, jlongArray p1, jsize p2, jsize p3, jlong * p4);
typedef void (*TYPE_nacl_jni_call_getfloatarrayregion)(JNIEnv * env, jfloatArray p1, jsize p2, jsize p3, jfloat * p4);
typedef void (*TYPE_nacl_jni_call_getdoublearrayregion)(JNIEnv * env, jdoubleArray p1, jsize p2, jsize p3, jdouble * p4);
typedef void (*TYPE_nacl_jni_call_setbooleanarrayregion)(JNIEnv * env, jbooleanArray p1, jsize p2, jsize p3, const jboolean * p4);
typedef void (*TYPE_nacl_jni_call_setbytearrayregion)(JNIEnv * env, jbyteArray p1, jsize p2, jsize p3, const jbyte * p4); /* 205 */
typedef void (*TYPE_nacl_jni_call_setchararrayregion)(JNIEnv * env, jcharArray p1, jsize p2, jsize p3, const jchar * p4);
typedef void (*TYPE_nacl_jni_call_setshortarrayregion)(JNIEnv * env, jshortArray p1, jsize p2, jsize p3, const jshort * p4);
typedef void (*TYPE_nacl_jni_call_setintarrayregion)(JNIEnv * env, jintArray p1, jsize p2, jsize p3, const jint * p4);
typedef void (*TYPE_nacl_jni_call_setlongarrayregion)(JNIEnv * env, jlongArray p1, jsize p2, jsize p3, const jlong * p4);
typedef void (*TYPE_nacl_jni_call_setfloatarrayregion)(JNIEnv * env, jfloatArray p1, jsize p2, jsize p3, const jfloat * p4); /* 210 */
typedef void (*TYPE_nacl_jni_call_setdoublearrayregion)(JNIEnv * env, jdoubleArray p1, jsize p2, jsize p3, const jdouble * p4);
typedef jint (*TYPE_nacl_jni_call_registernatives)(JNIEnv * env, jclass p1, const JNINativeMethod * p2, jint p3);
typedef jint (*TYPE_nacl_jni_call_unregisternatives)(JNIEnv * env, jclass p1);
typedef jint (*TYPE_nacl_jni_call_monitorenter)(JNIEnv * env, jobject p1);
typedef jint (*TYPE_nacl_jni_call_monitorexit)(JNIEnv * env, jobject p1); /* 215 */
typedef jint (*TYPE_nacl_jni_call_getjavavm)(JNIEnv * env, JavaVM ** p1);
typedef void (*TYPE_nacl_jni_call_getstringregion)(JNIEnv * env, jstring p1, jsize p2, jsize p3, jchar * p4);
typedef void (*TYPE_nacl_jni_call_getstringutfregion)(JNIEnv * env, jstring p1, jsize p2, jsize p3, char * p4);
typedef void * (*TYPE_nacl_jni_call_getprimitivearraycritical)(JNIEnv * env, jarray p1, jboolean * p2);
typedef void (*TYPE_nacl_jni_call_releaseprimitivearraycritical)(JNIEnv * env, jarray p1, void * p2, jint p3); /* 220 */
typedef const jchar * (*TYPE_nacl_jni_call_getstringcritical)(JNIEnv * env, jstring p1, jboolean * p2);
typedef void (*TYPE_nacl_jni_call_releasestringcritical)(JNIEnv * env, jstring p1, const jchar * p2);
typedef jweak (*TYPE_nacl_jni_call_newweakglobalref)(JNIEnv * env, jobject p1);
typedef void (*TYPE_nacl_jni_call_deleteweakglobalref)(JNIEnv * env, jweak p1);
typedef jboolean (*TYPE_nacl_jni_call_exceptioncheck)(JNIEnv * env); /* 225 */
typedef jobject (*TYPE_nacl_jni_call_newdirectbytebuffer)(JNIEnv * env, void * p1, jlong p2);
typedef void * (*TYPE_nacl_jni_call_getdirectbufferaddress)(JNIEnv * env, jobject p1);
typedef jlong (*TYPE_nacl_jni_call_getdirectbuffercapacity)(JNIEnv * env, jobject p1);
typedef jobjectRefType (*TYPE_nacl_jni_call_getobjectreftype)(JNIEnv * env, jobject p1);


























/*
typedef int (*TYPE_nacl_jni_call_getversion) ();

typedef const char *(*TYPE_nacl_jni_call_getstringutflength) (JNIEnv * env, jstring string);
typedef const char *(*TYPE_nacl_jni_call_getstringutfchars) (JNIEnv * env, jstring string, jbyte * usr_buf, jsize len);
typedef void (*TYPE_nacl_jni_call_releasestringutfchars) (JNIEnv * env, jstring string, const char * utf);
typedef const char *(*TYPE_nacl_jni_call_getstringutfregion) (JNIEnv *env, jstring str, jsize start, jsize len, char *buf);
typedef jstring (*TYPE_nacl_jni_call_newstringutf) (JNIEnv * env, const char *bytes);

typedef jsize (*TYPE_nacl_jni_call_getarraylength) (JNIEnv *env, jarray array);
typedef void (*TYPE_nacl_jni_call_getbytearrayregion) (JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
typedef void (*TYPE_nacl_jni_call_setbytearrayregion) (JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
typedef void (*TYPE_nacl_jni_call_getchararrayregion) (JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
typedef void (*TYPE_nacl_jni_call_setchararrayregion) (JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
typedef void (*TYPE_nacl_jni_call_getintarrayregion) (JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
typedef void (*TYPE_nacl_jni_call_setintarrayregion) (JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
typedef jint * (*TYPE_nacl_jni_call_getintarrayelements) (JNIEnv *env, jintArray array, jboolean *isCopy);
typedef void (*TYPE_nacl_jni_call_releaseintarrayelements) (JNIEnv *env, jintArray array, jint *elems, jint mode);

typedef jclass (*TYPE_nacl_jni_call_getobjectclass) (JNIEnv *env, jobject obj);
typedef jfieldID (*TYPE_nacl_jni_call_getfieldid) (JNIEnv *env, jclass clazz, const char *name, const char *sig);

typedef jboolean (*TYPE_nacl_jni_call_getbooleanfield) (JNIEnv *env, jobject obj, jfieldID fieldID); 
typedef void (*TYPE_nacl_jni_call_setbooleanfield) (JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value); 
typedef jlong (*TYPE_nacl_jni_call_getlongfield) (JNIEnv *env, jobject obj, jfieldID fieldID);
typedef jint (*TYPE_nacl_jni_call_getintfield) (JNIEnv *env, jobject obj, jfieldID fieldID);
typedef void (*TYPE_nacl_jni_call_setintfield) (JNIEnv *env, jobject obj, jfieldID fieldID, jint value);
typedef jobject (*TYPE_nacl_jni_call_getobjectfield) (JNIEnv *env, jobject obj, jfieldID fieldID);
typedef jfieldID (*TYPE_nacl_jni_call_getstaticfieldid) (JNIEnv *env, jclass clazz, const char *name, const char *sig);
typedef jint (*TYPE_nacl_jni_call_getstaticintfield) (JNIEnv *env, jclass clazz, jfieldID fieldID);
typedef void (*TYPE_nacl_jni_call_setstaticintfield) (JNIEnv *env, jclass clazz, jfieldID fieldID, jint value);

typedef jclass (*TYPE_nacl_jni_call_findclass) (JNIEnv *env, const char *name);
typedef jintArray (*TYPE_nacl_jni_call_newintarray) (JNIEnv *env, jsize length);
typedef jobjectArray (*TYPE_nacl_jni_call_newobjectarray) (JNIEnv *env, jsize length, jclass elementClass, jobject initialElement);
typedef void (*TYPE_nacl_jni_call_setobjectarrayelement) (JNIEnv *env, jobjectArray array, jsize index, jobject value);
typedef void (*TYPE_nacl_jni_call_deletelocalref) (JNIEnv *env, jobject localRef);

typedef jmethodID (*TYPE_nacl_jni_call_getmethodid)(JNIEnv *env, jclass clazz, const char *name, const char *sig);
typedef jmethodID (*TYPE_nacl_jni_call_getstaticmethodid)(JNIEnv *env, jclass clazz, const char *name, const char *sig);
typedef void (*TYPE_nacl_jni_call_callvoidmethod) (JNIEnv *env, jobject obj, jmethodID methodID, ...);
typedef void (*TYPE_nacl_jni_call_callvoidmethoda) (JNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);
typedef void (*TYPE_nacl_jni_call_callintmethod) (JNIEnv *env, jobject obj, jmethodID methodID, ...);
typedef void (*TYPE_nacl_jni_call_callstaticvoidmethod) (JNIEnv *env, jobject obj, jmethodID methodID, ...);

typedef jthrowable (*TYPE_nacl_jni_call_exceptionoccurred) (JNIEnv *env);
typedef void (*TYPE_nacl_jni_call_exceptiondescribe) (JNIEnv *env);
typedef void (*TYPE_nacl_jni_call_exceptionclear) (JNIEnv *env);
typedef jint (*TYPE_nacl_jni_call_thrownew) (JNIEnv *env, jclass clazz, const char *message);

typedef jint (*TYPE_nacl_jni_call_monitorenter)(JNIEnv *env, jobject obj);
typedef jint (*TYPE_nacl_jni_call_monitorexit)(JNIEnv *env, jobject obj);

typedef jbyteArray (*TYPE_nacl_jni_call_newbytearray)(JNIEnv *env, jsize length);
typedef jlongArray (*TYPE_nacl_jni_call_newlongarray) (JNIEnv *env, jsize length);

typedef void (*TYPE_nacl_jni_call_getlongarrayregion) (JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);
typedef void (*TYPE_nacl_jni_call_setlongarrayregion) (JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);

typedef jboolean (*TYPE_nacl_jni_call_callbooleanmethod) (JNIEnv *env, jobject obj, jmethodID methodID, ...); 

typedef void (*TYPE_nacl_jni_call_setobjectfield) (JNIEnv *env, jobject obj, jfieldID fieldID, jobject value);
typedef jobject (*TYPE_nacl_jni_call_callobjectmethod) (JNIEnv *env, jobject obj, jmethodID methodID, ...); 
typedef jobject (*TYPE_nacl_jni_call_getobjectarrayelement) (JNIEnv *env, jobjectArray array, jsize index);
typedef void (*TYPE_nacl_jni_call_setshortfield) (JNIEnv *env, jobject obj, jfieldID fieldID, jshort value); 
*/
