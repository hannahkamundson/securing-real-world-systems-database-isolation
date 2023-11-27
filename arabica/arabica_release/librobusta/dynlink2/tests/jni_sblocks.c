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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

#include "jni_sblocks.h"
#include "sblocks.h"

//the structs is defined in dynlink.c
extern struct JNINativeInterface_ func_table;
extern struct JNIInvokeInterface_ vm_table;

void initVMWrapperFunctions()
{
  // fill the function table with a default value
  int * fcn = (int *) &(vm_table.reserved0);
  int i;
  for (i = 0; i < 8; i++) {
    *(fcn) = (int)&unimplementedJNIFunction;
    fcn++;
  }
  // for now we only implement the GetEnv function for libnet.so
  vm_table.GetEnv = &GetEnv;
}

void initJNIWrapperFunctions()
{
  //fill the function table with a default value
  int * fcn = (int *) &(func_table.GetVersion);
  int i;
  for(i=0; i< NUM_JNI_FUNCTIONS; i++) {
    *(fcn) = (int)&unimplementedJNIFunction;
    fcn++;
  }

  /*printf("[jni_wrappers.c: initJNIWrapperFunctions:] func_table %p\n", &func_table);
    printf("[jni_wrappers.c: initJNIWrapperFunctions:] Initializing GetStringUTFChars with %p\n", &func);*/

  func_table.GetVersion = &GetVersion;
  func_table.DefineClass = &DefineClass;
  func_table.FindClass = &FindClass;
  func_table.FromReflectedMethod = &FromReflectedMethod;
  func_table.FromReflectedField = &FromReflectedField;
  func_table.ToReflectedMethod = &ToReflectedMethod;
  func_table.GetSuperclass = &GetSuperclass;
  func_table.IsAssignableFrom = &IsAssignableFrom;
  func_table.ToReflectedField = &ToReflectedField;
  func_table.Throw = &Throw; /* 10 */
  func_table.ThrowNew = &ThrowNew;
  func_table.ExceptionOccurred = &ExceptionOccurred;
  func_table.ExceptionDescribe = &ExceptionDescribe;
  func_table.ExceptionClear = &ExceptionClear;
  func_table.FatalError = &FatalError;
  func_table.PushLocalFrame = &PushLocalFrame;
  func_table.PopLocalFrame = &PopLocalFrame;
  func_table.NewGlobalRef = &NewGlobalRef;
  func_table.DeleteGlobalRef = &DeleteGlobalRef;
  func_table.DeleteLocalRef = &DeleteLocalRef; /* 20 */
  func_table.IsSameObject = &IsSameObject;
  func_table.NewLocalRef = &NewLocalRef;
  func_table.EnsureLocalCapacity = &EnsureLocalCapacity;
  func_table.AllocObject = &AllocObject;
  func_table.NewObject = &NewObject;
  func_table.NewObjectV = &NewObjectV;
  func_table.NewObjectA = &NewObjectA;
  func_table.GetObjectClass = &GetObjectClass;
  func_table.IsInstanceOf = &IsInstanceOf;
  func_table.GetMethodID = &GetMethodID; /* 30 */
  func_table.CallObjectMethod = &CallObjectMethod;
  func_table.CallObjectMethodV = &CallObjectMethodV;
  func_table.CallObjectMethodA = &CallObjectMethodA;
  func_table.CallBooleanMethod = &CallBooleanMethod;
  func_table.CallBooleanMethodV = &CallBooleanMethodV;
  func_table.CallBooleanMethodA = &CallBooleanMethodA;
  func_table.CallByteMethod = &CallByteMethod;
  func_table.CallByteMethodV = &CallByteMethodV;
  func_table.CallByteMethodA = &CallByteMethodA;
  func_table.CallCharMethod = &CallCharMethod; /* 40 */
  func_table.CallCharMethodV = &CallCharMethodV;
  func_table.CallCharMethodA = &CallCharMethodA;
  func_table.CallShortMethod = &CallShortMethod;
  func_table.CallShortMethodV = &CallShortMethodV;
  func_table.CallShortMethodA = &CallShortMethodA;
  func_table.CallIntMethod = &CallIntMethod;
  func_table.CallIntMethodV = &CallIntMethodV;
  func_table.CallIntMethodA = &CallIntMethodA;
  func_table.CallLongMethod = &CallLongMethod;
  func_table.CallLongMethodV = &CallLongMethodV; /* 50 */
  func_table.CallLongMethodA = &CallLongMethodA;
  func_table.CallFloatMethod = &CallFloatMethod;
  func_table.CallFloatMethodV = &CallFloatMethodV;
  func_table.CallFloatMethodA = &CallFloatMethodA;
  func_table.CallDoubleMethod = &CallDoubleMethod;
  func_table.CallDoubleMethodV = &CallDoubleMethodV;
  func_table.CallDoubleMethodA = &CallDoubleMethodA;
  func_table.CallVoidMethod = &CallVoidMethod;
  func_table.CallVoidMethodV = &CallVoidMethodV;
  func_table.CallVoidMethodA = &CallVoidMethodA; /* 60 */
  func_table.CallNonvirtualObjectMethod = &CallNonvirtualObjectMethod;
  func_table.CallNonvirtualObjectMethodV = &CallNonvirtualObjectMethodV;
  func_table.CallNonvirtualObjectMethodA = &CallNonvirtualObjectMethodA;
  func_table.CallNonvirtualBooleanMethod = &CallNonvirtualBooleanMethod;
  func_table.CallNonvirtualBooleanMethodV = &CallNonvirtualBooleanMethodV;
  func_table.CallNonvirtualBooleanMethodA = &CallNonvirtualBooleanMethodA;
  func_table.CallNonvirtualByteMethod = &CallNonvirtualByteMethod;
  func_table.CallNonvirtualByteMethodV = &CallNonvirtualByteMethodV;
  func_table.CallNonvirtualByteMethodA = &CallNonvirtualByteMethodA;
  func_table.CallNonvirtualCharMethod = &CallNonvirtualCharMethod; /* 70 */
  func_table.CallNonvirtualCharMethodV = &CallNonvirtualCharMethodV;
  func_table.CallNonvirtualCharMethodA = &CallNonvirtualCharMethodA;
  func_table.CallNonvirtualShortMethod = &CallNonvirtualShortMethod;
  func_table.CallNonvirtualShortMethodV = &CallNonvirtualShortMethodV;
  func_table.CallNonvirtualShortMethodA = &CallNonvirtualShortMethodA;
  func_table.CallNonvirtualIntMethod = &CallNonvirtualIntMethod;
  func_table.CallNonvirtualIntMethodV = &CallNonvirtualIntMethodV;
  func_table.CallNonvirtualIntMethodA = &CallNonvirtualIntMethodA;
  func_table.CallNonvirtualLongMethod = &CallNonvirtualLongMethod;
  func_table.CallNonvirtualLongMethodV = &CallNonvirtualLongMethodV; /* 80 */
  func_table.CallNonvirtualLongMethodA = &CallNonvirtualLongMethodA;
  func_table.CallNonvirtualFloatMethod = &CallNonvirtualFloatMethod;
  func_table.CallNonvirtualFloatMethodV = &CallNonvirtualFloatMethodV;
  func_table.CallNonvirtualFloatMethodA = &CallNonvirtualFloatMethodA;
  func_table.CallNonvirtualDoubleMethod = &CallNonvirtualDoubleMethod;
  func_table.CallNonvirtualDoubleMethodV = &CallNonvirtualDoubleMethodV;
  func_table.CallNonvirtualDoubleMethodA = &CallNonvirtualDoubleMethodA;
  func_table.CallNonvirtualVoidMethod = &CallNonvirtualVoidMethod;
  func_table.CallNonvirtualVoidMethodV = &CallNonvirtualVoidMethodV;
  func_table.CallNonvirtualVoidMethodA = &CallNonvirtualVoidMethodA; /* 90 */
  func_table.GetFieldID = &GetFieldID;
  func_table.GetObjectField = &GetObjectField;
  func_table.GetBooleanField = &GetBooleanField;
  func_table.GetByteField = &GetByteField;
  func_table.GetCharField = &GetCharField;
  func_table.GetShortField = &GetShortField;
  func_table.GetIntField = &GetIntField;
  func_table.GetLongField = &GetLongField;
  func_table.GetFloatField = &GetFloatField;
  func_table.GetDoubleField = &GetDoubleField; /* 100 */
  func_table.SetObjectField = &SetObjectField;
  func_table.SetBooleanField = &SetBooleanField;
  func_table.SetByteField = &SetByteField;
  func_table.SetCharField = &SetCharField;
  func_table.SetShortField = &SetShortField;
  func_table.SetIntField = &SetIntField;
  func_table.SetLongField = &SetLongField;
  func_table.SetFloatField = &SetFloatField;
  func_table.SetDoubleField = &SetDoubleField;
  func_table.GetStaticMethodID = &GetStaticMethodID; /* 110 */
  func_table.CallStaticObjectMethod = &CallStaticObjectMethod;
  func_table.CallStaticObjectMethodV = &CallStaticObjectMethodV;
  func_table.CallStaticObjectMethodA = &CallStaticObjectMethodA;
  func_table.CallStaticBooleanMethod = &CallStaticBooleanMethod;
  func_table.CallStaticBooleanMethodV = &CallStaticBooleanMethodV;
  func_table.CallStaticBooleanMethodA = &CallStaticBooleanMethodA;
  func_table.CallStaticByteMethod = &CallStaticByteMethod;
  func_table.CallStaticByteMethodV = &CallStaticByteMethodV;
  func_table.CallStaticByteMethodA = &CallStaticByteMethodA;
  func_table.CallStaticCharMethod = &CallStaticCharMethod; /* 120 */
  func_table.CallStaticCharMethodV = &CallStaticCharMethodV;
  func_table.CallStaticCharMethodA = &CallStaticCharMethodA;
  func_table.CallStaticShortMethod = &CallStaticShortMethod;
  func_table.CallStaticShortMethodV = &CallStaticShortMethodV;
  func_table.CallStaticShortMethodA = &CallStaticShortMethodA;
  func_table.CallStaticIntMethod = &CallStaticIntMethod;
  func_table.CallStaticIntMethodV = &CallStaticIntMethodV;
  func_table.CallStaticIntMethodA = &CallStaticIntMethodA;
  func_table.CallStaticLongMethod = &CallStaticLongMethod;
  func_table.CallStaticLongMethodV = &CallStaticLongMethodV; /* 130 */
  func_table.CallStaticLongMethodA = &CallStaticLongMethodA;
  func_table.CallStaticFloatMethod = &CallStaticFloatMethod;
  func_table.CallStaticFloatMethodV = &CallStaticFloatMethodV;
  func_table.CallStaticFloatMethodA = &CallStaticFloatMethodA;
  func_table.CallStaticDoubleMethod = &CallStaticDoubleMethod;
  func_table.CallStaticDoubleMethodV = &CallStaticDoubleMethodV;
  func_table.CallStaticDoubleMethodA = &CallStaticDoubleMethodA;
  func_table.CallStaticVoidMethod = &CallStaticVoidMethod;
  func_table.CallStaticVoidMethodV = &CallStaticVoidMethodV;
  func_table.CallStaticVoidMethodA = &CallStaticVoidMethodA; /* 140 */
  func_table.GetStaticFieldID = &GetStaticFieldID;
  func_table.GetStaticObjectField = &GetStaticObjectField;
  func_table.GetStaticBooleanField = &GetStaticBooleanField;
  func_table.GetStaticByteField = &GetStaticByteField;
  func_table.GetStaticCharField = &GetStaticCharField;
  func_table.GetStaticShortField = &GetStaticShortField;
  func_table.GetStaticIntField = &GetStaticIntField;
  func_table.GetStaticLongField = &GetStaticLongField;
  func_table.GetStaticFloatField = &GetStaticFloatField;
  func_table.GetStaticDoubleField = &GetStaticDoubleField; /* 150 */
  func_table.SetStaticObjectField = &SetStaticObjectField;
  func_table.SetStaticBooleanField = &SetStaticBooleanField;
  func_table.SetStaticByteField = &SetStaticByteField;
  func_table.SetStaticCharField = &SetStaticCharField;
  func_table.SetStaticShortField = &SetStaticShortField;
  func_table.SetStaticIntField = &SetStaticIntField;
  func_table.SetStaticLongField = &SetStaticLongField;
  func_table.SetStaticFloatField = &SetStaticFloatField;
  func_table.SetStaticDoubleField = &SetStaticDoubleField;
  func_table.NewString = &NewString; /* 160 */
  func_table.GetStringLength = &GetStringLength;
  func_table.GetStringChars = &GetStringChars;
  func_table.ReleaseStringChars=&ReleaseStringChars;
  func_table.NewStringUTF = &NewStringUTF;
  func_table.GetStringUTFLength = &GetStringUTFLength;
  func_table.GetStringUTFChars = &GetStringUTFChars;
  func_table.ReleaseStringUTFChars = &ReleaseStringUTFChars;
  func_table.GetArrayLength = &GetArrayLength;
  func_table.NewObjectArray = &NewObjectArray;
  func_table.GetObjectArrayElement = &GetObjectArrayElement; /* 170 */
  func_table.SetObjectArrayElement = &SetObjectArrayElement;
  func_table.NewBooleanArray = &NewBooleanArray;
  func_table.NewByteArray = &NewByteArray;
  func_table.NewCharArray = &NewCharArray;
  func_table.NewShortArray = &NewShortArray;
  func_table.NewIntArray = &NewIntArray;
  func_table.NewLongArray = &NewLongArray;
  func_table.NewFloatArray = &NewFloatArray;
  func_table.NewDoubleArray = &NewDoubleArray;
  func_table.GetBooleanArrayElements=&GetBooleanArrayElements; /* 180 */
  func_table.GetByteArrayElements = &GetByteArrayElements;
  func_table.GetCharArrayElements = &GetCharArrayElements;
  func_table.GetShortArrayElements = &GetShortArrayElements;
  func_table.GetIntArrayElements = &GetIntArrayElements;
  func_table.GetLongArrayElements = &GetLongArrayElements;
  func_table.GetFloatArrayElements = &GetFloatArrayElements;
  func_table.GetDoubleArrayElements = &GetDoubleArrayElements;
  func_table.ReleaseBooleanArrayElements=&ReleaseBooleanArrayElements;
  func_table.ReleaseByteArrayElements = &ReleaseByteArrayElements;
  func_table.ReleaseCharArrayElements = &ReleaseCharArrayElements; /* 190 */
  func_table.ReleaseShortArrayElements = &ReleaseShortArrayElements;
  func_table.ReleaseIntArrayElements = &ReleaseIntArrayElements;
  func_table.ReleaseLongArrayElements = &ReleaseLongArrayElements;
  func_table.ReleaseFloatArrayElements = &ReleaseFloatArrayElements;
  func_table.ReleaseDoubleArrayElements = &ReleaseDoubleArrayElements;
  func_table.GetBooleanArrayRegion = &GetBooleanArrayRegion;
  func_table.GetByteArrayRegion = &GetByteArrayRegion;
  func_table.GetCharArrayRegion = &GetCharArrayRegion;
  func_table.GetShortArrayRegion = &GetShortArrayRegion;
  func_table.GetIntArrayRegion=&GetIntArrayRegion; /* 200 */
  func_table.GetLongArrayRegion = &GetLongArrayRegion;
  func_table.GetFloatArrayRegion = &GetFloatArrayRegion;
  func_table.GetDoubleArrayRegion = &GetDoubleArrayRegion;
  func_table.SetBooleanArrayRegion = &SetBooleanArrayRegion;
  func_table.SetByteArrayRegion = &SetByteArrayRegion;
  func_table.SetCharArrayRegion = &SetCharArrayRegion;
  func_table.SetShortArrayRegion = &SetShortArrayRegion;
  func_table.SetIntArrayRegion = &SetIntArrayRegion;
  func_table.SetLongArrayRegion = &SetLongArrayRegion;
  func_table.SetFloatArrayRegion = &SetFloatArrayRegion; /* 210 */
  func_table.SetDoubleArrayRegion = &SetDoubleArrayRegion;
  func_table.RegisterNatives = &RegisterNatives;
  func_table.UnregisterNatives = &UnregisterNatives;
  func_table.MonitorEnter = &MonitorEnter;
  func_table.MonitorExit = &MonitorExit;
  func_table.GetJavaVM = &GetJavaVM;
  func_table.GetStringRegion = &GetStringRegion;
  func_table.GetStringUTFRegion = &GetStringUTFRegion;
  func_table.GetPrimitiveArrayCritical = &GetPrimitiveArrayCritical;
  func_table.ReleasePrimitiveArrayCritical = &ReleasePrimitiveArrayCritical; /* 220 */
  func_table.GetStringCritical = &GetStringCritical;
  func_table.ReleaseStringCritical = &ReleaseStringCritical;
  func_table.NewWeakGlobalRef = &NewWeakGlobalRef;
  func_table.DeleteWeakGlobalRef = &DeleteWeakGlobalRef;
  func_table.ExceptionCheck= &ExceptionCheck;
  func_table.NewDirectByteBuffer = &NewDirectByteBuffer;
  func_table.GetDirectBufferAddress = &GetDirectBufferAddress;
  func_table.GetDirectBufferCapacity = &GetDirectBufferCapacity;
  func_table.GetObjectRefType = &GetObjectRefType;

  /*printf("[jni_wrappers.c: initJNIWrapperFunctions:] GetStringUTFChars %p\n", func_table.GetStringUTFChars);
    fflush(stdout);*/
}

/*
jthrowable ExceptionOccurred(JNIEnv *env);
void ExceptionDescribe(JNIEnv *env);
void ExceptionClear(JNIEnv *env);
*/

void unimplementedJNIFunction()
{
  printf("[jni_sblocks.c: unimplmentedJNIFunction:] This JNI function is not registered in the function table.\n");
}

/*
 * the function for the JavaVM interface
 */

jint GetEnv(JavaVM * vm, void ** penv, jint version)
{
  printf("here?\n");
  fflush(stdout);
  jint r = (jint) (NACL_SYSCALL(vm_call_getenv))(vm, penv, version);
  return r;
}

/*
 * the function for the JNI interface
 */

jint GetVersion(JNIEnv * env)
{
  //eventually, we may have to handle return of longs, etc here
  //since the return type of a NaCl syscall is int32_t
  return (jint) (NACL_SYSCALL(jni_call_getversion))(env);
}

jclass DefineClass(JNIEnv * env, const char * name, jobject loader, const jbyte * buf, jsize bufLen)
{
  //eventually, we may have to handle return of longs, etc here
  //since the return type of a NaCl syscall is int32_t
  return (jclass) (NACL_SYSCALL(jni_call_defineclass))(env, name, loader, buf, bufLen);
}

jclass FindClass(JNIEnv *env, const char * name)
{
  return (NACL_SYSCALL(jni_call_findclass))(env, name);
}

jmethodID FromReflectedMethod(JNIEnv *env, jobject method)
{
  return (NACL_SYSCALL(jni_call_fromreflectedmethod))(env, method);
}

jfieldID FromReflectedField(JNIEnv *env, jobject field)
{
  return (NACL_SYSCALL(jni_call_fromreflectedfield))(env, field);
}

jobject ToReflectedMethod(JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic)
{
  return (NACL_SYSCALL(jni_call_toreflectedmethod))(env, cls, methodID, isStatic);
}

jclass GetSuperclass(JNIEnv *env, jclass clazz)
{
  return (NACL_SYSCALL(jni_call_getsuperclass))(env, clazz);
}

jboolean IsAssignableFrom(JNIEnv *env, jclass clsa, jclass clsb)
{
  return (NACL_SYSCALL(jni_call_isassignablefrom))(env, clsa, clsb);
}

jobject ToReflectedField(JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic)
{
  return (NACL_SYSCALL(jni_call_toreflectedfield))(env, cls, fieldID, isStatic);
}

jint Throw(JNIEnv *env, jthrowable obj)
{
  return (NACL_SYSCALL(jni_call_throw))(env, obj);
}

jint ThrowNew(JNIEnv *env, jclass clazz, const char *message)
{
  return (NACL_SYSCALL(jni_call_thrownew))(env, clazz, message);
}

jthrowable ExceptionOccurred(JNIEnv *env)
{
  return (NACL_SYSCALL(jni_call_exceptionoccurred))(env);
}

void ExceptionDescribe(JNIEnv *env)
{
  (NACL_SYSCALL(jni_call_exceptiondescribe))(env);
}

void ExceptionClear(JNIEnv *env)
{
  (NACL_SYSCALL(jni_call_exceptionclear))(env);
}

void FatalError(JNIEnv *env, const char * msg)
{
  (NACL_SYSCALL(jni_call_fatalerror))(env, msg);
}

jint PushLocalFrame(JNIEnv *env, jint capacity)
{
  return (NACL_SYSCALL(jni_call_pushlocalframe))(env, capacity);
}

jobject PopLocalFrame(JNIEnv *env, jobject result)
{
  return (NACL_SYSCALL(jni_call_poplocalframe))(env, result);
}

jobject NewGlobalRef(JNIEnv *env, jobject obj)
{
  return (NACL_SYSCALL(jni_call_newglobalref))(env, obj);
}

void DeleteGlobalRef(JNIEnv *env, jobject gref)
{
  (NACL_SYSCALL(jni_call_deleteglobalref))(env, gref);
}

void DeleteLocalRef(JNIEnv *env, jobject gref)
{
  (NACL_SYSCALL(jni_call_deletelocalref))(env, gref);
}

jboolean IsSameObject(JNIEnv *env, jobject ref1, jobject ref2)
{
  return (NACL_SYSCALL(jni_call_issameobject))(env, ref1, ref2);
}

jobject NewLocalRef(JNIEnv *env, jobject ref)
{
  return (NACL_SYSCALL(jni_call_newlocalref))(env, ref);
}

jint EnsureLocalCapacity(JNIEnv *env, jint capacity)
{
  return (NACL_SYSCALL(jni_call_ensurelocalcapacity))(env, capacity);
}

jobject AllocObject(JNIEnv *env, jclass clazz)
{
  return (NACL_SYSCALL(jni_call_allocobject))(env, clazz);
}

jobject NewObject(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_newobject))(env, clazz, methodID);
}

jobject NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_newobjectv))(env, clazz, methodID, args);
}

jobject NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_newobjecta))(env, clazz, methodID, args);
}

jclass GetObjectClass(JNIEnv *env, jobject obj)
{
  return (NACL_SYSCALL(jni_call_getobjectclass))(env,obj);	
}

jboolean IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz)
{
  return (NACL_SYSCALL(jni_call_isinstanceof))(env, obj, clazz);	
}

jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
  return (NACL_SYSCALL(jni_call_getmethodid))(env, clazz, name, sig);
}

jobject CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callobjectmethod))(env, obj, methodID);
}

jobject CallObjectMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callobjectmethodv))(env, obj, methodID, args);
}

jobject CallObjectMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callobjectmethoda))(env, obj, methodID, args);
}

jboolean CallBooleanMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callbooleanmethod))(env, obj, methodID);
}

jboolean CallBooleanMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callbooleanmethodv))(env, obj, methodID, args);
}

jboolean CallBooleanMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callbooleanmethoda))(env, obj, methodID, args);
}

jbyte CallByteMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callbytemethod))(env, obj, methodID);	
}

jbyte CallByteMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callbytemethodv))(env, obj, methodID, args);
}
jbyte CallByteMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callbytemethoda))(env, obj, methodID, args);
}

jchar CallCharMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callcharmethod))(env, obj, methodID);	
}

jchar CallCharMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callcharmethodv))(env, obj, methodID, args);
}

jchar CallCharMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callcharmethoda))(env, obj, methodID, args);
}

jshort CallShortMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callshortmethod))(env, obj, methodID);
}

jshort CallShortMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callshortmethodv))(env, obj, methodID, args);
}

jshort CallShortMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callshortmethoda))(env, obj, methodID, args);
}

jint CallIntMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callintmethod))(env, obj, methodID);
}

jint CallIntMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callintmethodv))(env, obj, methodID, args);
}

jint CallIntMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callintmethoda))(env, obj, methodID, args);
}

jlong CallLongMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_calllongmethod))(env, obj, methodID);
}

jlong CallLongMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_calllongmethodv))(env, obj, methodID, args);
}

jlong CallLongMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_calllongmethoda))(env, obj, methodID, args);
}

jfloat CallFloatMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callfloatmethod))(env, obj, methodID);
}

jfloat CallFloatMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callfloatmethodv))(env, obj, methodID, args);
}

jfloat CallFloatMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callfloatmethoda))(env, obj, methodID, args);
}

jdouble CallDoubleMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_calldoublemethod))(env, obj, methodID);
}

jdouble CallDoubleMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_calldoublemethodv))(env, obj, methodID, args);
}

jdouble CallDoubleMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_calldoublemethoda))(env, obj, methodID, args);
}

void CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
  (NACL_SYSCALL(jni_call_callvoidmethod))(env, obj, methodID);	
}

void CallVoidMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
  (NACL_SYSCALL(jni_call_callvoidmethodv))(env, obj, methodID, args);	
}

void CallVoidMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
  (NACL_SYSCALL(jni_call_callvoidmethoda))(env, obj, methodID, args);
}

jobject CallNonvirtualObjectMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualobjectmethod))(env, obj, clazz, methodID);
}

jobject CallNonvirtualObjectMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualobjectmethodv))(env, obj, clazz, methodID, args);
}

jobject CallNonvirtualObjectMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualobjectmethoda))(env, obj, clazz, methodID, args);
}

jboolean CallNonvirtualBooleanMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualbooleanmethod))(env, obj, clazz, methodID);
}

jboolean CallNonvirtualBooleanMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualbooleanmethodv))(env, obj, clazz, methodID, args);
}

jboolean CallNonvirtualBooleanMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualbooleanmethoda))(env, obj, clazz, methodID, args);
}

jbyte CallNonvirtualByteMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualbytemethod))(env, obj, clazz, methodID);
}

jbyte CallNonvirtualByteMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualbytemethodv))(env, obj, clazz, methodID, args);
}

jbyte CallNonvirtualByteMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualbytemethoda))(env, obj, clazz, methodID, args);
}

jchar CallNonvirtualCharMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualcharmethod))(env, obj, clazz, methodID);
}

jchar CallNonvirtualCharMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualcharmethodv))(env, obj, clazz, methodID, args);
}

jchar CallNonvirtualCharMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualcharmethoda))(env, obj, clazz, methodID, args);
}

jshort CallNonvirtualShortMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualshortmethod))(env, obj, clazz, methodID);
}

jshort CallNonvirtualShortMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualshortmethodv))(env, obj, clazz, methodID, args);
}

jshort CallNonvirtualShortMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualshortmethoda))(env, obj, clazz, methodID, args);
}

jint CallNonvirtualIntMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualintmethod))(env, obj, clazz, methodID);
}

jint CallNonvirtualIntMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualintmethodv))(env, obj, clazz, methodID, args);
}

jint CallNonvirtualIntMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualintmethoda))(env, obj, clazz, methodID, args);
}

jlong CallNonvirtualLongMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtuallongmethod))(env, obj, clazz, methodID);
}

jlong CallNonvirtualLongMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtuallongmethodv))(env, obj, clazz, methodID, args);
}

jlong CallNonvirtualLongMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtuallongmethoda))(env, obj, clazz, methodID, args);
}

jfloat CallNonvirtualFloatMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualfloatmethod))(env, obj, clazz, methodID);
}

jfloat CallNonvirtualFloatMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualfloatmethodv))(env, obj, clazz, methodID, args);
}

jfloat CallNonvirtualFloatMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualfloatmethoda))(env, obj, clazz, methodID, args);
}

jdouble CallNonvirtualDoubleMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualdoublemethod))(env, obj, clazz, methodID);
}

jdouble CallNonvirtualDoubleMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualdoublemethodv))(env, obj, clazz, methodID, args);
}

jdouble CallNonvirtualDoubleMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callnonvirtualdoublemethoda))(env, obj, clazz, methodID, args);
}

void CallNonvirtualVoidMethod(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, ...)
{
  (NACL_SYSCALL(jni_call_callnonvirtualvoidmethod))(env, obj, clazz, methodID);
}

void CallNonvirtualVoidMethodV(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  (NACL_SYSCALL(jni_call_callnonvirtualvoidmethodv))(env, obj, clazz, methodID, args);
}

void CallNonvirtualVoidMethodA(JNIEnv * env, jobject obj, jclass clazz, jmethodID methodID, const jvalue * args)
{
  (NACL_SYSCALL(jni_call_callnonvirtualvoidmethoda))(env, obj, clazz, methodID, args);
}

jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
  return (NACL_SYSCALL(jni_call_getfieldid))(env, clazz, name, sig);
}

jobject GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getobjectfield))(env, obj, fieldID);
}

jboolean GetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getbooleanfield))(env, obj, fieldID);
}

jbyte GetByteField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getbytefield))(env, obj, fieldID);
}

jchar GetCharField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getcharfield))(env, obj, fieldID);
}

jshort GetShortField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getshortfield))(env, obj, fieldID);
}

jint GetIntField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getintfield))(env, obj, fieldID);
}

jlong GetLongField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getlongfield))(env, obj, fieldID);
}

jfloat GetFloatField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getfloatfield))(env, obj, fieldID);
}

jdouble GetDoubleField(JNIEnv *env, jobject obj, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getdoublefield))(env, obj, fieldID);
}

void SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, jobject value)
{
  (NACL_SYSCALL(jni_call_setobjectfield))(env, obj, fieldID, value);
}

void SetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value)
{
  (NACL_SYSCALL(jni_call_setbooleanfield))(env, obj, fieldID, value);
}

void SetByteField(JNIEnv *env, jobject obj, jfieldID fieldID, jbyte value)
{
  (NACL_SYSCALL(jni_call_setbytefield))(env, obj, fieldID, value);
}

void SetCharField(JNIEnv *env, jobject obj, jfieldID fieldID, jchar value)
{
  (NACL_SYSCALL(jni_call_setcharfield))(env, obj, fieldID, value);
}

void SetShortField(JNIEnv *env, jobject obj, jfieldID fieldID, jshort value)
{
  (NACL_SYSCALL(jni_call_setshortfield))(env, obj, fieldID, value);
}

void SetIntField(JNIEnv *env, jobject obj, jfieldID fieldID, jint value)
{
  (NACL_SYSCALL(jni_call_setintfield))(env, obj, fieldID, value);
}

void SetLongField(JNIEnv *env, jobject obj, jfieldID fieldID, jlong value)
{
  (NACL_SYSCALL(jni_call_setlongfield))(env, obj, fieldID, value);
}

void SetFloatField(JNIEnv *env, jobject obj, jfieldID fieldID, jfloat value)
{
  (NACL_SYSCALL(jni_call_setfloatfield))(env, obj, fieldID, value);
}

void SetDoubleField(JNIEnv *env, jobject obj, jfieldID fieldID, jdouble value)
{
  (NACL_SYSCALL(jni_call_setdoublefield))(env, obj, fieldID, value);
}

jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
  return (NACL_SYSCALL(jni_call_getstaticmethodid))(env, clazz, name, sig);
}

jobject CallStaticObjectMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticobjectmethod))(env, clazz, methodID);
}

jobject CallStaticObjectMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticobjectmethodv))(env, clazz, methodID, args);
}

jobject CallStaticObjectMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticobjectmethoda))(env, clazz, methodID, args);
}

jboolean CallStaticBooleanMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticbooleanmethod))(env, clazz, methodID);
}

jboolean CallStaticBooleanMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticbooleanmethodv))(env, clazz, methodID, args);
}

jboolean CallStaticBooleanMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticbooleanmethoda))(env, clazz, methodID, args);
}

jbyte CallStaticByteMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticbytemethod))(env, clazz, methodID);
}

jbyte CallStaticByteMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticbytemethodv))(env, clazz, methodID, args);
}

jbyte CallStaticByteMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticbytemethoda))(env, clazz, methodID, args);
}

jchar CallStaticCharMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticcharmethod))(env, clazz, methodID);
}

jchar CallStaticCharMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticcharmethodv))(env, clazz, methodID, args);
}

jchar CallStaticCharMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticcharmethoda))(env, clazz, methodID, args);
}

jshort CallStaticShortMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticshortmethod))(env, clazz, methodID);
}

jshort CallStaticShortMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticshortmethodv))(env, clazz, methodID, args);
}

jshort CallStaticShortMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticshortmethoda))(env, clazz, methodID, args);
}

jint CallStaticIntMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticintmethod))(env, clazz, methodID);
}

jint CallStaticIntMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticintmethodv))(env, clazz, methodID, args);
}

jint CallStaticIntMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticintmethoda))(env, clazz, methodID, args);
}

jlong CallStaticLongMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticlongmethod))(env, clazz, methodID);
}

jlong CallStaticLongMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticlongmethodv))(env, clazz, methodID, args);
}

jlong CallStaticLongMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticlongmethoda))(env, clazz, methodID, args);
}

jfloat CallStaticFloatMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticfloatmethod))(env, clazz, methodID);
}

jfloat CallStaticFloatMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticfloatmethodv))(env, clazz, methodID, args);
}

jfloat CallStaticFloatMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticfloatmethoda))(env, clazz, methodID, args);
}

jdouble CallStaticDoubleMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  return (NACL_SYSCALL(jni_call_callstaticdoublemethod))(env, clazz, methodID);
}

jdouble CallStaticDoubleMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  return (NACL_SYSCALL(jni_call_callstaticdoublemethodv))(env, clazz, methodID, args);
}

jdouble CallStaticDoubleMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  return (NACL_SYSCALL(jni_call_callstaticdoublemethoda))(env, clazz, methodID, args);
}

void CallStaticVoidMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
  (NACL_SYSCALL(jni_call_callstaticvoidmethod))(env, clazz, methodID);
}

void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{
  (NACL_SYSCALL(jni_call_callstaticvoidmethodv))(env, clazz, methodID, args);
}

void CallStaticVoidMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue * args)
{
  (NACL_SYSCALL(jni_call_callstaticvoidmethoda))(env, clazz, methodID, args);
}

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
  return (NACL_SYSCALL(jni_call_getstaticfieldid))(env, clazz, name, sig);
}

jobject GetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticobjectfield))(env, clazz, fieldID);
}

jboolean GetStaticBooleanField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticbooleanfield))(env, clazz, fieldID);
}

jbyte GetStaticByteField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticbytefield))(env, clazz, fieldID);
}

jchar GetStaticCharField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticcharfield))(env, clazz, fieldID);
}

jshort GetStaticShortField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticshortfield))(env, clazz, fieldID);
}

jint GetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticintfield))(env, clazz, fieldID);
}

jlong GetStaticLongField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticlongfield))(env, clazz, fieldID);
}

jfloat GetStaticFloatField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticfloatfield))(env, clazz, fieldID);
}

jdouble GetStaticDoubleField(JNIEnv *env, jclass clazz, jfieldID fieldID)
{
  return (NACL_SYSCALL(jni_call_getstaticdoublefield))(env, clazz, fieldID);
}

void SetStaticObjectField(JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value)
{
  (NACL_SYSCALL(jni_call_setstaticobjectfield))(env, clazz, fieldID, value);
}

void SetStaticBooleanField(JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value)
{
  (NACL_SYSCALL(jni_call_setstaticbooleanfield))(env, clazz, fieldID, value);
}

void SetStaticByteField(JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value)
{
  (NACL_SYSCALL(jni_call_setstaticbytefield))(env, clazz, fieldID, value);
}

void SetStaticCharField(JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value)
{
  (NACL_SYSCALL(jni_call_setstaticcharfield))(env, clazz, fieldID, value);
}

void SetStaticShortField(JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value)
{
  (NACL_SYSCALL(jni_call_setstaticshortfield))(env, clazz, fieldID, value);
}

void SetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID, jint value)
{
  (NACL_SYSCALL(jni_call_setstaticintfield))(env,clazz,fieldID,value);
}

void SetStaticLongField(JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value)
{
  (NACL_SYSCALL(jni_call_setstaticlongfield))(env, clazz, fieldID, value);
}

void SetStaticFloatField(JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value)
{
  (NACL_SYSCALL(jni_call_setstaticfloatfield))(env, clazz, fieldID, value);
}

void SetStaticDoubleField(JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value)
{
  (NACL_SYSCALL(jni_call_setstaticdoublefield))(env, clazz, fieldID, value);
}

jstring NewString(JNIEnv *env, const jchar * uchars, jsize len)
{
  return (NACL_SYSCALL(jni_call_newstring))(env, uchars, len);
}

jsize GetStringLength(JNIEnv *env, jstring string)
{
  return (jsize) (NACL_SYSCALL(jni_call_getstringlength))(env, string);
}

const jchar * GetStringChars(JNIEnv *env, jstring string, jboolean * isCopy)
{
  return (jchar *) (NACL_SYSCALL(jni_call_getstringchars))(env, string, isCopy);
}

void ReleaseStringChars(JNIEnv * env, jstring string, const jchar * chars)
{
  (NACL_SYSCALL(jni_call_releasestringchars))(env, string, chars);
}

jstring NewStringUTF(JNIEnv *env, const char * buf)
{
  return (NACL_SYSCALL(jni_call_newstringutf))(env, buf);
}

jsize GetStringUTFLength(JNIEnv *env, jstring string)
{
  return (jsize) (NACL_SYSCALL(jni_call_getstringutflength))(env, string);
}

const jbyte * GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy)
{
  //printf("[jni_sblocks.c: GetStringUTFChars] string: %p isCopy: %p\n", string, isCopy);
  //fflush(stdout);
  //discover the length of the string and allocate a sandbox byte array
  //which is big enough to hold that string
  jsize len = GetStringUTFLength(env, string);
	
  //printf("[jni_sblocks.c: GetStringUTFChars] Returned length: %i\n", len);
  //fflush(stdout);

  jbyte * usr_buf = (jbyte *) malloc(len * sizeof(jbyte)+1);	

  //printf("[jni_sblocks.c: GetStringUTFChars:] Allocated space: %p\n", usr_buf);
  //fflush(stdout);

  //do the copy
  (NACL_SYSCALL(jni_call_getstringutfregion))(env, string, 0, len, usr_buf);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL)
    {
      *(isCopy)=JNI_TRUE;
    }

  //printf("Returning to user function...\n");
  //fflush(stdout);

  return usr_buf;
}

void ReleaseStringUTFChars(JNIEnv * env, jstring string, const char * utf)
{
  //simply free this buffer, there's no need to notify the JVM
  free(utf);
	
  //(NACL_SYSCALL(jni_call_releasestringutfchars))(env,string,utf);
  //void function
}

jsize GetArrayLength(JNIEnv *env, jarray array)
{
  return (int)(NACL_SYSCALL(jni_call_getarraylength))(env, array);
}

jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement)
{
  return (NACL_SYSCALL(jni_call_newobjectarray))(env, length, elementClass, initialElement);
}

jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index)
{
  return (NACL_SYSCALL(jni_call_getobjectarrayelement))(env, array, index);
}

void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject value)
{
  (NACL_SYSCALL(jni_call_setobjectarrayelement))(env, array, index, value);
}

jbooleanArray NewBooleanArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newbooleanarray))(env, length);
}

jbyteArray NewByteArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newbytearray))(env, length);
}

jcharArray NewCharArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newchararray))(env, length);
}

jshortArray NewShortArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newshortarray))(env, length);
}

jintArray NewIntArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newintarray))(env, length);
}

jlongArray NewLongArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newlongarray))(env, length);
}

jfloatArray NewFloatArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newfloatarray))(env, length);
}

jdoubleArray NewDoubleArray(JNIEnv *env, jsize length)
{
  return (NACL_SYSCALL(jni_call_newdoublearray))(env, length);
}

jboolean * GetBooleanArrayElements(JNIEnv * env, jbooleanArray array, jboolean * isCopy)
{
  //printf("[jni_sblocks.c: GetBooleanArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env, array);

  //printf("[jni_sblocks.c: GetBooleanArrayElements] The length is: %i\n", len);

  //allocate space for the sandbox copy
  jboolean * buf = (jboolean *) malloc(len * sizeof(jboolean));
  //printf("[jni_sblocks.c: GetBooleanArrayElements] buf= %p.\n", buf);

  (NACL_SYSCALL(jni_call_getbooleanarrayregion))(env, array, 0, len, buf);
  //printf("[jni_sblocks.c: GetBooleanArrayElements] Done with syscall.");

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    *(isCopy) = 1;
  }

  return buf;
}

jbyte * GetByteArrayElements(JNIEnv *env, jbyteArray array, jboolean *isCopy)
{
  //printf("[jni_sblocks.c: GetByteArrayElements] Getting the length of array: %p\n", array);
  //fflush(stdout);
  //discover the length of the array
  jsize len = GetArrayLength(env,array);

  //printf("[jni_sblocks.c: GetByteArrayElements] The length is: %d\n", len);
  //fflush(stdout);

  //allocate space for the sandbox copy
  jbyte * buf = (jbyte *) malloc(len * sizeof(jbyte));
  //printf("[jni_sblocks.c: GetByteArrayElements] buf= %p.\n", buf);
  //fflush(stdout);

  (NACL_SYSCALL(jni_call_getbytearrayregion))(env, array, 0, len, buf);
  //printf("[jni_sblocks.c: GetByteArrayElements] Done with syscall.\n");
  //fflush(stdout);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    *(isCopy) = 1;
  }
	
  return buf;
}

jchar *GetCharArrayElements(JNIEnv *env, jcharArray array, jboolean *isCopy) 
{
  //printf("[jni_sblocks.c: GetCharArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env, array);

  //printf("[jni_sblocks.c: GetCharArrayElements] The length is: %i\n", len);

  //allocate space for the sandbox copy
  jchar * buf = (jchar *) malloc(len * sizeof(jchar));
  //printf("[jni_sblocks.c: GetCharArrayElements] buf= %p.\n", buf);

  (NACL_SYSCALL(jni_call_getchararrayregion))(env, array, 0, len, buf);
  //printf("[jni_sblocks.c: GetCharArrayElements] Done with syscall.");

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    *(isCopy) = JNI_TRUE;
  }
	
  return buf;
}

jshort * GetShortArrayElements(JNIEnv * env, jshortArray array, jboolean * isCopy)
{
  //printf("[jni_sblocks.c: GetShortArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env, array);

  //printf("[jni_sblocks.c: GetShortArrayElements] The length is: %i\n", len);

  //allocate space for the sandbox copy
  jshort * buf = (jshort *) malloc(len * sizeof(jshort));
  //printf("[jni_sblocks.c: GetShortArrayElements] buf= %p.\n", buf);

  (NACL_SYSCALL(jni_call_getshortarrayregion))(env, array, 0, len, buf);
  //printf("[jni_sblocks.c: GetShortArrayElements] Done with syscall.");

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    *(isCopy) = 1;
  }

  return buf;
}

jint *GetIntArrayElements(JNIEnv *env, jintArray array, jboolean *isCopy) 
{
  //printf("[jni_sblocks.c: GetIntArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env,array);

  //printf("[jni_sblocks.c: GetIntArrayElements] The length is: %i\n", len);

  //allocate space for the sandbox copy
  jint * buf = (jint *) malloc(len * sizeof(jint));
  //printf("[jni_sblocks.c: GetIntArrayElements] buf= %p.\n", buf);

  (NACL_SYSCALL(jni_call_getintarrayregion))(env,array,0,len,buf);
  //printf("[jni_sblocks.c: GetIntArrayElements] Done with syscall.");

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL)
    {
      *(isCopy) = 1;
    }
	
  return buf;
}

jlong * GetLongArrayElements(JNIEnv *env, jlongArray array, jboolean *isCopy)
{
  //printf("[jni_sblocks.c: GetLongArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env,array);

  //allocate space for the sandbox copy
  jlong * buf = (jlong *) malloc(len * sizeof(jlong));

  (NACL_SYSCALL(jni_call_getlongarrayregion))(env, array, 0, len, buf);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    *(isCopy) = 1;
  }
	
  return buf;
}

jfloat * GetFloatArrayElements(JNIEnv * env, jfloatArray array, jboolean * isCopy)
{
  //printf("[jni_sblocks.c: GetFloatArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env,array);

  //allocate space for the sandbox copy
  jfloat * buf = (jfloat *) malloc(len * sizeof(jfloat));

  (NACL_SYSCALL(jni_call_getfloatarrayregion))(env, array, 0, len, buf);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    * (isCopy) = 1;
  }
	
  return buf;
}

jdouble * GetDoubleArrayElements(JNIEnv * env, jdoubleArray array, jboolean * isCopy)
{
  //printf("[jni_sblocks.c: GetDoubleArrayElements] Getting the length\n");
  //discover the length of the array
  jsize len = GetArrayLength(env,array);

  //allocate space for the sandbox copy
  jdouble * buf = (jdouble *) malloc(len * sizeof(jdouble));

  (NACL_SYSCALL(jni_call_getdoublearrayregion))(env, array, 0, len, buf);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    * (isCopy) = 1;
  }
	
  return buf;
}

void ReleaseBooleanArrayElements(JNIEnv * env, jbooleanArray array, jboolean * elems, jint mode)
{
  //printf("[jni_sblocks.c: ReleaseBooleanArrayElements:] Finishing %p with mode %i\n", elems, mode);

  jsize len;
  switch(mode) {
    case 0:
      //From the JNI Spec:
      //"copy back the content and free the elems buffer"
      len = GetArrayLength(env, array);
      SetBooleanArrayRegion(env, array, 0, len, elems);
      free(elems);
      break;
    case JNI_COMMIT:
      //From the JNI Spec:
      //"copy back the content but do not free the elems buffer"
      len = GetArrayLength(env, array);
      SetBooleanArrayRegion(env, array, 0, len, elems);
      break;
    case JNI_ABORT:
      //From the JNI Spec:
      //"free the buffer without copying back the possible changes"
      free(elems);
      break;
    }
}

void ReleaseByteArrayElements(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode)
{
  jsize len;

  switch(mode) {
  case 0:
      //From the JNI Spec:
      //"copy the back the content and free the elems buffer"
      len = GetArrayLength(env, array);
      SetByteArrayRegion(env, array, 0, len, elems);
      free(elems);
      break;

  case JNI_COMMIT:
      //From the JNI Spec:
      //"copy back the content but do not free the elems buffer"
      len = GetArrayLength(env, array);
      SetByteArrayRegion(env, array, 0, len, elems);
      break;

  case JNI_ABORT:
      //From the JNI Spec:
      //"free the buffer without copying back the possible changes"
      free(elems);
      break;
  }
}

void ReleaseCharArrayElements(JNIEnv *env, jcharArray array, jchar *elems, jint mode) 
{
  //printf("[jni_sblocks.c: ReleaseCharArrayElements:] Finishing %p with mode %i\n", elems, mode);

  jsize len;

  switch(mode) {
  case 0:
    //From the JNI Spec:
    //"copy the back the content and free the elems buffer"
    len = GetArrayLength(env, array);
    SetCharArrayRegion(env, array, 0, len, elems);
    free(elems);
    break;

  case JNI_COMMIT:
    //From the JNI Spec:
    //"copy back the content but do not free the elems buffer"
    len = GetArrayLength(env, array);
    SetCharArrayRegion(env, array, 0, len, elems);
    break;

  case JNI_ABORT:
    //From the JNI Spec:
    //"free the buffer without copying back the possible changes"
    free(elems);
    break;
  }
}

void ReleaseShortArrayElements(JNIEnv * env, jshortArray array, jshort * elems, jint mode)
{
  //printf("[jni_sblocks.c: ReleaseShortArrayElements:] Finishing %p with mode %i\n", elems, mode);
	
  jsize len;

  switch(mode) {
  case 0:
      //From the JNI Spec:
      //"copy the back the content and free the elems buffer"
      len = GetArrayLength(env, array);
      SetShortArrayRegion(env, array, 0, len, elems);
      free(elems);
      break;

  case JNI_COMMIT:
      //From the JNI Spec:
      //"copy back the content but do not free the elems buffer"
      len = GetArrayLength(env, array);
      SetShortArrayRegion(env, array, 0, len, elems);
      break;

  case JNI_ABORT:
      //From the JNI Spec:
      //"free the buffer without copying back the possible changes"
      free(elems);
      break;
  }
}

void ReleaseIntArrayElements(JNIEnv *env, jintArray array, jint *elems, jint mode) 
{
  //printf("[jni_sblocks.c: ReleaseIntArrayElements:] Finishing %p with mode %i\n", elems, mode);

  jsize len;

  switch(mode) {
  case 0:
    //From the JNI Spec:
    //"copy the back the content and free the elems buffer"
    len = GetArrayLength(env, array);
    SetIntArrayRegion(env, array, 0, len, elems);
    free(elems);
    break;

  case JNI_COMMIT:
    //From the JNI Spec:
    //"copy back the content but do not free the elems buffer"
    len = GetArrayLength(env, array);
    SetIntArrayRegion(env, array, 0, len, elems);
    break;

  case JNI_ABORT:
    //From the JNI Spec:
    //"free the buffer without copying back the possible changes"
    free(elems);
    break;
  }
}

void ReleaseLongArrayElements(JNIEnv *env, jlongArray array, jlong *elems, jint mode)
{
  //printf("[jni_sblocks.c: ReleaseLongArrayElements:] Finishing %p with mode %i\n", elems, mode);

  jsize len;

  switch(mode) {
  case 0:
    //From the JNI Spec:
    //"copy the back the content and free the elems buffer"
    len = GetArrayLength(env, array);
    SetLongArrayRegion(env, array, 0, len, elems);
    free(elems);
    break;

  case JNI_COMMIT:
    //From the JNI Spec:
    //"copy back the content but do not free the elems buffer"
    len = GetArrayLength(env,array);
    SetLongArrayRegion(env,array,0,len,elems);
    break;

  case JNI_ABORT:
    //From the JNI Spec:
    //"free the buffer without copying back the possible changes"
    free(elems);
    break;
  }
}

void ReleaseFloatArrayElements(JNIEnv * env, jfloatArray array, jfloat * elems, jint mode)
{
  //printf("[jni_sblocks.c: ReleaseFloatArrayElements:] Finishing %p with mode %i\n", elems, mode);
	
  jsize len;

  switch(mode) {
  case 0:
    //From the JNI Spec:
    //"copy the back the content and free the elems buffer"
    len = GetArrayLength(env, array);
    SetFloatArrayRegion(env, array, 0, len, elems);
    free(elems);
    break;

  case JNI_COMMIT:
    //From the JNI Spec:
    //"copy back the content but do not free the elems buffer"
    len = GetArrayLength(env, array);
    SetFloatArrayRegion(env, array, 0, len, elems);
    break;

  case JNI_ABORT:
    //From the JNI Spec:
    //"free the buffer without copying back the possible changes"
    free(elems);
    break;
  }
}

void ReleaseDoubleArrayElements(JNIEnv * env, jdoubleArray array, jdouble * elems, jint mode)
{
  //printf("[jni_sblocks.c: ReleaseDoubleArrayElements:] Finishing %p with mode %i\n", elems, mode);
	
  jsize len;

  switch(mode) {
  case 0:
    //From the JNI Spec:
    //"copy the back the content and free the elems buffer"
    len = GetArrayLength(env, array);
    SetDoubleArrayRegion(env, array, 0, len, elems);
    free(elems);
    break;

  case JNI_COMMIT:
    //From the JNI Spec:
    //"copy back the content but do not free the elems buffer"
    len = GetArrayLength(env, array);
    SetDoubleArrayRegion(env, array, 0, len, elems);
    break;

  case JNI_ABORT:
    //From the JNI Spec:
    //"free the buffer without copying back the possible changes"
    free(elems);
    break;
  }
}

void GetBooleanArrayRegion(JNIEnv *env, jbooleanArray array, jsize start, jsize len, jboolean * buf)
{
  (NACL_SYSCALL(jni_call_getbooleanarrayregion))(env, array, start, len, buf);
}

void GetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf)
{
  //printf("[jni_sblocks.c: GetByteArrayRegion] env: %p, array: %p, start: %i, len: %i, buf: %p\n", env, array, start, len, buf);
  //fflush(stdout);
  (NACL_SYSCALL(jni_call_getbytearrayregion))(env, array, start, len, buf);
}

void GetCharArrayRegion(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf)
{
  (NACL_SYSCALL(jni_call_getchararrayregion))(env, array, start, len, buf);
}

void GetShortArrayRegion(JNIEnv *env, jshortArray array, jsize start, jsize len, jshort * buf)
{
  (NACL_SYSCALL(jni_call_getshortarrayregion))(env, array, start, len, buf);
}

void GetIntArrayRegion(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf)
{
  (NACL_SYSCALL(jni_call_getintarrayregion))(env, array, start, len, buf);
}

void GetLongArrayRegion(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf)
{
  (NACL_SYSCALL(jni_call_getlongarrayregion))(env, array, start, len, buf);
}

void GetFloatArrayRegion(JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf)
{
  (NACL_SYSCALL(jni_call_getfloatarrayregion))(env, array, start, len, buf);
}

void GetDoubleArrayRegion(JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf)
{
  (NACL_SYSCALL(jni_call_getdoublearrayregion))(env, array, start, len, buf);
}

void SetBooleanArrayRegion(JNIEnv *env, jbooleanArray array, jsize start, jsize len, const jboolean *buf)
{
  (NACL_SYSCALL(jni_call_setbooleanarrayregion))(env, array, start, len, buf);
}

void SetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, const jbyte *buf)
{
  (NACL_SYSCALL(jni_call_setbytearrayregion))(env, array, start, len, buf);
}

void SetCharArrayRegion(JNIEnv *env, jcharArray array, jsize start, jsize len, const jchar *buf)
{
  (NACL_SYSCALL(jni_call_setchararrayregion))(env, array, start, len, buf);
}

void SetShortArrayRegion(JNIEnv *env, jshortArray array, jsize start, jsize len, const jshort *buf)
{
  (NACL_SYSCALL(jni_call_setshortarrayregion))(env, array, start, len, buf);
}

void SetIntArrayRegion(JNIEnv *env, jintArray array, jsize start, jsize len, const jint *buf)
{
  (NACL_SYSCALL(jni_call_setintarrayregion))(env, array, start, len, buf);
}

void SetLongArrayRegion(JNIEnv *env, jlongArray array, jsize start, jsize len, const jlong *buf)
{
  (NACL_SYSCALL(jni_call_setlongarrayregion))(env, array, start, len, buf);
}

void SetFloatArrayRegion(JNIEnv *env, jfloatArray array, jsize start, jsize len, const jfloat *buf)
{
  (NACL_SYSCALL(jni_call_setfloatarrayregion))(env, array, start, len, buf);
}

void SetDoubleArrayRegion(JNIEnv *env, jdoubleArray array, jsize start, jsize len, const jdouble * buf)
{
  (NACL_SYSCALL(jni_call_setdoublearrayregion))(env, array, start, len, buf);
}

jint RegisterNatives(JNIEnv * env, jclass clazz, const JNINativeMethod * methods, jint nMethods)
{
  return (NACL_SYSCALL(jni_call_registernatives))(env, clazz, methods, nMethods);
}

jint UnregisterNatives(JNIEnv * env, jclass clazz)
{
  return (NACL_SYSCALL(jni_call_unregisternatives))(env, clazz);
}

jint MonitorEnter(JNIEnv * env, jobject obj)
{
  //printf("[jni_sblocks.c: MonitorEnter:] env %p, obj: %p\n", env, obj);
  return (NACL_SYSCALL(jni_call_monitorenter))(env, obj);
}

jint MonitorExit(JNIEnv *env, jobject obj)
{
  //printf("[jni_sblocks.c: MonitorExit:] env %p, obj: %p\n", env, obj);
  return (NACL_SYSCALL(jni_call_monitorexit))(env, obj);
}

jint GetJavaVM(JNIEnv * env, JavaVM ** vm)
{
  return (NACL_SYSCALL(jni_call_getjavavm))(env, vm);
}

void GetStringRegion(JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf)
{
  (NACL_SYSCALL(jni_call_getstringregion))(env, str, start, len, buf);
}

void GetStringUTFRegion(JNIEnv *env, jstring str, jsize start, jsize len, char *buf)
{
  (NACL_SYSCALL(jni_call_getstringutfregion))(env,str,start,len,buf);
}

void * GetPrimitiveArrayCritical(JNIEnv *env, jarray array, jboolean *isCopy)
{
  //discover the length of the array
  jsize len = GetArrayLength(env, array);

  //allocate space for the sandbox copy
  jbyte * buf = (jbyte *) malloc(len * sizeof(jbyte));

  (NACL_SYSCALL(jni_call_getbytearrayregion))(env, array, 0, len, buf);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL) {
    *(isCopy) = 1;
  }
	
  return buf;	
}

void ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array, void *carray, jint mode)
{
  jsize len;

  switch(mode) {
  case 0:
    //From the JNI Spec:
    //"copy back the content and free the elems buffer"
    len = GetArrayLength(env, array);
    SetByteArrayRegion(env, array, 0, len, carray);
    free(carray);
    break;

  case JNI_COMMIT:
    //From the JNI Spec:
    //"copy back the content but do not free the elems buffer"
    len = GetArrayLength(env, array);
    SetByteArrayRegion(env, array, 0, len, carray);
    break;

  case JNI_ABORT:
    //From the JNI Spec:
    //"free the buffer without copying back the possible changes"
    free(carray);
    break;
  }	
}

const jchar * GetStringCritical(JNIEnv * env, jstring string, jboolean * isCopy)
{
  //printf("[jni_sblocks.c: GetStringUTFChars] string: %p isCopy: %p\n", string, isCopy);
  //fflush(stdout);
  //discover the length of the string and allocate a sandbox byte array
  //which is big enough to hold that string
  jsize len = GetStringLength(env, string);
	
  //printf("[jni_sblocks.c: GetStringUTFChars] Returned length: %i\n", len);
  //fflush(stdout);

  jchar * usr_buf = (jchar *) malloc(len * sizeof(jchar)+1);

  //printf("[jni_sblocks.c: GetStringUTFChars:] Allocated space: %p\n", usr_buf);
  //fflush(stdout);

  //do the copy
  (NACL_SYSCALL(jni_call_getstringregion))(env, string, 0, len, usr_buf);

  //we can't return anything but a copy for now, so return always true
  if(isCopy != NULL)
    {
      *(isCopy) = JNI_TRUE;
    }

  //printf("Returning to user function...\n");
  //fflush(stdout);

  return (const jchar *) usr_buf;
}

void ReleaseStringCritical(JNIEnv * env, jstring string, const jchar * carray)
{
  //simply free this buffer, there's no need to notify the JVM
  //free(carray);
  (NACL_SYSCALL(jni_call_releasestringcritical))(env, string, carray);
}

jweak NewWeakGlobalRef(JNIEnv * env, jobject obj)
{
  return (jweak) (NACL_SYSCALL(jni_call_newweakglobalref))(env, obj);
}

void DeleteWeakGlobalRef(JNIEnv * env, jweak wref)
{
  (NACL_SYSCALL(jni_call_deleteweakglobalref))(env, wref);
}

jboolean ExceptionCheck(JNIEnv * env)
{
  return (jboolean) (NACL_SYSCALL(jni_call_exceptioncheck))(env);
}

jobject NewDirectByteBuffer(JNIEnv * env, void * address, jlong capacity)
{
  return (NACL_SYSCALL(jni_call_newdirectbytebuffer))(env, address, capacity);
}

void * GetDirectBufferAddress(JNIEnv *env, jobject buf)
{
  return (void *) (NACL_SYSCALL(jni_call_getdirectbufferaddress))(env, buf);
}

jlong GetDirectBufferCapacity(JNIEnv * env, jobject buf)
{
  return (NACL_SYSCALL(jni_call_getdirectbuffercapacity))(env, buf);
}

jobjectRefType GetObjectRefType(JNIEnv * env, jobject obj)
{
  return (NACL_SYSCALL(jni_call_getobjectreftype))(env, obj);
}
