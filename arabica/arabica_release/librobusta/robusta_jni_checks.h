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

#ifndef _ROBUSTA_JNI_CHECKS_H_
#define _ROBUSTA_JNI_CHECKS_H_

//#include "include/robusta_jni.h"
#include <jni.h>
#include <stdlib.h>

//convenience definintions
typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef JNIEnv * JNIEnvPtr;

#define INLINE_STATIC_CHECK static inline

//Verification Macros
//-------------------------------------------------------------------------------------------------------------

#define ASSERT(x, action) if (!(x)) {action;}

#define ERROR_ARGS(prefix, msg) \
{printf ("Safety violation: "); printf msg; printf ("\n"); exit(0);}
//{printf ("%s at %s :%u: ", prefix, __FILE__, __LINE__); printf msg; printf ("\n"); exit(0);}

#define SAFETY_ERROR_ARGS(msg) ERROR_ARGS("Safety violation", msg)
#define SAFETY_ERROR(msg) SAFETY_ERROR_ARGS((msg))

#define ASSERT_SAFETY_ARGS(x, msg) ASSERT(x, SAFETY_ERROR_ARGS(msg))
#define ASSERT_SAFETY(x, msg) ASSERT_SAFETY_ARGS(x, (msg))

#ifdef NDEBUG
#define BUG_ARGS(msg)
#define BUG(msg)
#define ASSERT_BUG_ARGS(x, msg)
#define ASSERT_BUG(x, msg)
#else
#define BUG_ARGS(msg) ERROR_ARGS("Bug", msg)
#define BUG(msg) BUG_ARGS((msg))
#define ASSERT_BUG_ARGS(x, msg) ASSERT(x, BUG_ARGS(msg))
#define ASSERT_BUG(x, msg) ASSERT_BUG_ARGS(x, (msg))
#endif 

// comment out the following tags to disable certain kinds of runtime checking.

// mes310
// comment out all checkings

//#define ARRAY_BOUNDS_CHECKING // controled by CCured, do not know how to disable this
//#define RUNTIME_TYPE_CHECKING // can only uncomment partial cost at this point, since the structures of jmethodid and jfieldid has been changed. there is a dereferencement for every ID operation.
//#define NONNULL_CHECKING
//#define EXCEPTION_CHECKING
//#define ACCESS_CONTROL

// COMMON checking macros

#ifdef RUNTIME_TYPE_CHECKING

#define CHECK_SIMPLE_TYPE(tp, simplet, msg) \
ASSERT_SAFETY(tp->tk == simpletype && tp->st == simplet, msg)

#define CHECK_OBJECT_TYPE(tp, msg) \
ASSERT_SAFETY(tp->tk == classtype || tp->tk == arraytype, msg)

//Changed this to make it simpler for the time being...
#define CHECK_STRING_TYPE(env,str) \
ASSERT_SAFETY((*env)->IsInstanceOf(env, str, glob_string_ref), "Expecting a String object.");

// returntype must be a simple type.
// This macro does two checks:
//   1. makes sure rtype matches the return type in javatp. otherwise,
//      print out ttypemsg
//   2. makes sure arglist's type matches the argument types in javatp.
#define CHECK_METHOD_TYPE(env, tp, rtype, rtypemsg, arglist)\
{CHECK_SIMPLE_TYPE(tp, rtype, rtypemsg); \
 argsTypeCheck(env, tp, arglist);}

#else

#define CHECK_SIMPLE_TYPE(javatp, simplet, msg)
#define CHECK_OBJECT_TYPE(javatp, msg)
#define CHECK_STRING_TYPE(env,str)
#define CHECK_METHOD_TYPE(env, tp, rtype, rtypemsg, arglist)

#endif


#ifdef NONNULL_CHECKING
#define CHECK_NONNULL(ptr, msg) ASSERT_SAFETY(ptr,msg)
#else
#define CHECK_NONNULL(ptr, msg)
#endif

#ifdef EXCEPTION_CHECKING
#define CHECK_EXCEPTION(env) \
ASSERT(!(*env)->ExceptionCheck(env), \
  printf("FATAL*** A JNI function was called with a pending exception..."); (*env)->ExceptionDescribe(env); exit(0))
#else
#define CHECK_EXCEPTION(env)
#endif

// A field can be accessed either it's a public member, or it access
// its own private field
#ifdef ACCESS_CONTROL
#define CHECK_ACCESS_FIELD(env, pub, orig, obj)\
ASSERT_SAFETY(pub || (*env)->IsSameObject(env, orig, obj), "Voilating access control rules.");
#else
#define CHECK_ACCESS_FIELD(env, pub, obj)
#endif

//Verification types
enum JavaTypeKinds {simpletype, classtype, arraytype};

enum JavaSimpleTypes {booleantype, bytetype, chartype, doubletype, floattype, inttype, longtype, shorttype, voidtype};

struct JavaTypeStruct {
  enum JavaTypeKinds tk;   // what kind of types
  enum JavaSimpleTypes st; // what kind of simple types; only makes sense when tk = simpletype
  char * name;             // name of the class or the array; only makes sense when tk = classtype or arraytype
  struct JavaTypeStruct * next;
};

typedef struct JavaTypeStruct JavaType;

typedef struct jfID {
  jfieldID __fid;
  jclass __clazz;
  JavaType * __tysig;
  BOOL __pubmember;
} robusta_jfieldID;

typedef struct jmID {
  jmethodID __mid;
  JavaType * __tysig;
  BOOL __pubmember;
  int __num_sig_words;
} robusta_jmethodID;

//-----------------------------------------------------------------------------
//these vars are used by initJNICheckIDs
jmethodID isArrayMethodID, getNameMethodID, getFieldID, getModifiersID;
jclass glob_bytearray_ref, glob_intarray_ref, glob_chararray_ref, glob_longarray_ref, glob_string_ref;

#define RUNTIME_TYPE_CHECKING

void initJNICheckIDs(JNIEnv *env);

//integrity checks...



//confidentiality checks...
//An alternative solution: call "clazz.getField()";
//if there's a security voilation, getField will raise an exception.
INLINE_STATIC_CHECK BOOL access_control (JNIEnvPtr env, jobject member)
//int access_control(JNIEnvPtr env, jobject member)
{
  int modifier = (*env)->CallIntMethod(env, member, getModifiersID);
  // the 0th bit of modifier indicates whether it's a public member.
  return ((modifier & 1) != 0);
}

#endif
