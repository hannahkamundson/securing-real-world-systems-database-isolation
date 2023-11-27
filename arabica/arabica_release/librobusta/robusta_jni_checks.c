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

//#include "robusta_jni.h"
#include <jni.h>
#include "robusta_jni_checks.h"

//these global IDs are used for checks throughout robusta
//TODO: called by call_in currently, could be optimized elsewhere...
void initJNICheckIDs(JNIEnv *env)
{

  jclass cls = (*env)->FindClass(env, "java/lang/Class");
  ASSERT_BUG(cls, "Can't find the class java.lang.Clss.");

  isArrayMethodID =
    (*env)->GetMethodID(env,cls,"isArray","()Z");

  ASSERT_BUG(isArrayMethodID,
             "Can't find the methodID of isArray in java.lang.Class.");

  getNameMethodID =
    (*env)->GetMethodID(env,cls,"getName", "()Ljava/lang/String;");
  ASSERT_BUG(getNameMethodID,
             "Can't find the methodID of getName in java.lang.Class.");

  getFieldID =
    (*env)->GetMethodID(env,cls,"getField",
                        "(Ljava/lang/String;)Ljava/lang/reflect/Field;");
  ASSERT_BUG(getFieldID,
             "Can't find the methodID of getField in java.lang.Class.");


  jclass membercls = (*env)->FindClass(env, "java/lang/reflect/Member");
  ASSERT_BUG(membercls, "Can't find the interface java.lang.reflect.Member.");

  getModifiersID =
    (*env)->GetMethodID(env, membercls, "getModifiers", "()I");
  ASSERT_BUG(getModifiersID,
             "Can't find getModifiers ID in the class java.lang.reflect.Member.");

  //these are global references for checks
  glob_bytearray_ref = (*env)->FindClass(env, "[B");
  glob_intarray_ref = (*env)->FindClass(env, "[I");
  glob_chararray_ref = (*env)->FindClass(env, "[C");
  glob_longarray_ref = (*env)->FindClass(env, "[J");
  glob_string_ref = (*env)->FindClass(env,"java/lang/String");

  // The JNI jobject is no longer needed. The methodID will keep valid
  // until the correponding class is unloaded. In this case, cls and
  // membercls points to system classes, which won't be unloaded.
  (*env)->DeleteLocalRef(env, cls);
  (*env)->DeleteLocalRef(env, membercls);

}

