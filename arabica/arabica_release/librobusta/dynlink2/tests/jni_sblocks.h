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

#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "robusta_jni.h"

void unimplementedJNIFunction();

//JavaVM
jint GetEnv(JavaVM *, void **, jint);

//JNI
jint GetVersion(JNIEnv * env);
jclass DefineClass(JNIEnv * env, const char * p1, jobject p2, const jbyte * p3, jsize p4);
jclass FindClass(JNIEnv * env, const char * p1);
jmethodID FromReflectedMethod(JNIEnv * env, jobject p1);
jfieldID FromReflectedField(JNIEnv * env, jobject p1); /* 5 */
jobject ToReflectedMethod(JNIEnv * env, jclass p1, jmethodID p2, jboolean p3);
jclass GetSuperclass(JNIEnv * env, jclass p1);
jboolean IsAssignableFrom(JNIEnv * env, jclass p1, jclass p2);
jobject ToReflectedField(JNIEnv * env, jclass p1, jfieldID p2, jboolean p3);
jint Throw(JNIEnv * env, jthrowable p1); /* 10 */
jint ThrowNew(JNIEnv * env, jclass p1, const char * p2);
jthrowable ExceptionOccurred(JNIEnv * env);
void ExceptionDescribe(JNIEnv * env);
void ExceptionClear(JNIEnv * env);
void FatalError(JNIEnv * env, const char * p1); /* 15 */
jint PushLocalFrame(JNIEnv * env, jint p1);
jobject PopLocalFrame(JNIEnv * env, jobject p1);
jobject NewGlobalRef(JNIEnv * env, jobject p1);
void DeleteGlobalRef(JNIEnv * env, jobject p1);
void DeleteLocalRef(JNIEnv * env, jobject p1); /* 20 */
jboolean IsSameObject(JNIEnv * env, jobject p1, jobject p2);
jobject NewLocalRef(JNIEnv * env, jobject p1);
jint EnsureLocalCapacity(JNIEnv * env, jint p1);
jobject AllocObject(JNIEnv * env, jclass p1);
jobject NewObject(JNIEnv * env, jclass p1, jmethodID p2, ...); /* 25 */
jobject NewObjectV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jobject NewObjectA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jclass GetObjectClass(JNIEnv * env, jobject p1);
jboolean IsInstanceOf(JNIEnv * env, jobject p1, jclass p2);
jmethodID GetMethodID(JNIEnv * env, jclass p1, const char * p2, const char * p3); /* 30 */
jobject CallObjectMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jobject CallObjectMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jobject CallObjectMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jboolean CallBooleanMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jboolean CallBooleanMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3); /* 35 */
jboolean CallBooleanMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jbyte CallByteMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jbyte CallByteMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jbyte CallByteMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jchar CallCharMethod(JNIEnv * env, jobject p1, jmethodID p2, ...); /* 40 */
jchar CallCharMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jchar CallCharMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jshort CallShortMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jshort CallShortMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jshort CallShortMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3); /* 45 */
jint CallIntMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jint CallIntMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jint CallIntMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jlong CallLongMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jlong CallLongMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3); /* 50 */
jlong CallLongMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jfloat CallFloatMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
jfloat CallFloatMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jfloat CallFloatMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
jdouble CallDoubleMethod(JNIEnv * env, jobject p1, jmethodID p2, ...); /* 55 */
jdouble CallDoubleMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
jdouble CallDoubleMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3);
void CallVoidMethod(JNIEnv * env, jobject p1, jmethodID p2, ...);
void CallVoidMethodV(JNIEnv * env, jobject p1, jmethodID p2, va_list p3);
void CallVoidMethodA(JNIEnv * env, jobject p1, jmethodID p2, const jvalue * p3); /* 60 */
jobject CallNonvirtualObjectMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jobject CallNonvirtualObjectMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jobject CallNonvirtualObjectMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jboolean CallNonvirtualBooleanMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jboolean CallNonvirtualBooleanMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4); /* 65 */
jboolean CallNonvirtualBooleanMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jbyte CallNonvirtualByteMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jbyte CallNonvirtualByteMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jbyte CallNonvirtualByteMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jchar CallNonvirtualCharMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...); /* 70 */
jchar CallNonvirtualCharMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jchar CallNonvirtualCharMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jshort CallNonvirtualShortMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jshort CallNonvirtualShortMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jshort CallNonvirtualShortMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4); /* 75 */
jint CallNonvirtualIntMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jint CallNonvirtualIntMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jint CallNonvirtualIntMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jlong CallNonvirtualLongMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jlong CallNonvirtualLongMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4); /* 80 */
jlong CallNonvirtualLongMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jfloat CallNonvirtualFloatMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
jfloat CallNonvirtualFloatMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jfloat CallNonvirtualFloatMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
jdouble CallNonvirtualDoubleMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...); /* 85 */
jdouble CallNonvirtualDoubleMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
jdouble CallNonvirtualDoubleMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4);
void CallNonvirtualVoidMethod(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, ...);
void CallNonvirtualVoidMethodV(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, va_list p4);
void CallNonvirtualVoidMethodA(JNIEnv * env, jobject p1, jclass p2, jmethodID p3, const jvalue * p4); /* 90 */
jfieldID GetFieldID(JNIEnv * env, jclass p1, const char * p2, const char * p3);
jobject GetObjectField(JNIEnv * env, jobject p1, jfieldID p2);
jboolean GetBooleanField(JNIEnv * env, jobject p1, jfieldID p2);
jbyte GetByteField(JNIEnv * env, jobject p1, jfieldID p2);
jchar GetCharField(JNIEnv * env, jobject p1, jfieldID p2); /* 95 */
jshort GetShortField(JNIEnv * env, jobject p1, jfieldID p2);
jint GetIntField(JNIEnv * env, jobject p1, jfieldID p2);
jlong GetLongField(JNIEnv * env, jobject p1, jfieldID p2);
jfloat GetFloatField(JNIEnv * env, jobject p1, jfieldID p2);
jdouble GetDoubleField(JNIEnv * env, jobject p1, jfieldID p2); /* 100 */
void SetObjectField(JNIEnv * env, jobject p1, jfieldID p2, jobject p3);
void SetBooleanField(JNIEnv * env, jobject p1, jfieldID p2, jboolean p3);
void SetByteField(JNIEnv * env, jobject p1, jfieldID p2, jbyte p3);
void SetCharField(JNIEnv * env, jobject p1, jfieldID p2, jchar p3);
void SetShortField(JNIEnv * env, jobject p1, jfieldID p2, jshort p3); /* 105 */
void SetIntField(JNIEnv * env, jobject p1, jfieldID p2, jint p3);
void SetLongField(JNIEnv * env, jobject p1, jfieldID p2, jlong p3);
void SetFloatField(JNIEnv * env, jobject p1, jfieldID p2, jfloat p3);
void SetDoubleField(JNIEnv * env, jobject p1, jfieldID p2, jdouble p3);
jmethodID GetStaticMethodID(JNIEnv * env, jclass p1, const char * p2, const char * p3); /* 110 */
jobject CallStaticObjectMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jobject CallStaticObjectMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jobject CallStaticObjectMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jboolean CallStaticBooleanMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jboolean CallStaticBooleanMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3); /* 115 */
jboolean CallStaticBooleanMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jbyte CallStaticByteMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jbyte CallStaticByteMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jbyte CallStaticByteMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jchar CallStaticCharMethod(JNIEnv * env, jclass p1, jmethodID p2, ...); /* 120 */
jchar CallStaticCharMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jchar CallStaticCharMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jshort CallStaticShortMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jshort CallStaticShortMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jshort CallStaticShortMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3); /* 125 */
jint CallStaticIntMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jint CallStaticIntMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jint CallStaticIntMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jlong CallStaticLongMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jlong CallStaticLongMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3); /* 130 */
jlong CallStaticLongMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jfloat CallStaticFloatMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
jfloat CallStaticFloatMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jfloat CallStaticFloatMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
jdouble CallStaticDoubleMethod(JNIEnv * env, jclass p1, jmethodID p2, ...); /* 135 */
jdouble CallStaticDoubleMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
jdouble CallStaticDoubleMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3);
void CallStaticVoidMethod(JNIEnv * env, jclass p1, jmethodID p2, ...);
void CallStaticVoidMethodV(JNIEnv * env, jclass p1, jmethodID p2, va_list p3);
void CallStaticVoidMethodA(JNIEnv * env, jclass p1, jmethodID p2, const jvalue * p3); /* 140 */
jfieldID GetStaticFieldID(JNIEnv * env, jclass p1, const char * p2, const char * p3);
jobject GetStaticObjectField(JNIEnv * env, jclass p1, jfieldID p2);
jboolean GetStaticBooleanField(JNIEnv * env, jclass p1, jfieldID p2);
jbyte GetStaticByteField(JNIEnv * env, jclass p1, jfieldID p2);
jchar GetStaticCharField(JNIEnv * env, jclass p1, jfieldID p2); /* 145 */
jshort GetStaticShortField(JNIEnv * env, jclass p1, jfieldID p2);
jint GetStaticIntField(JNIEnv * env, jclass p1, jfieldID p2);
jlong GetStaticLongField(JNIEnv * env, jclass p1, jfieldID p2);
jfloat GetStaticFloatField(JNIEnv * env, jclass p1, jfieldID p2);
jdouble GetStaticDoubleField(JNIEnv * env, jclass p1, jfieldID p2); /* 150 */
void SetStaticObjectField(JNIEnv * env, jclass p1, jfieldID p2, jobject p3);
void SetStaticBooleanField(JNIEnv * env, jclass p1, jfieldID p2, jboolean p3);
void SetStaticByteField(JNIEnv * env, jclass p1, jfieldID p2, jbyte p3);
void SetStaticCharField(JNIEnv * env, jclass p1, jfieldID p2, jchar p3);
void SetStaticShortField(JNIEnv * env, jclass p1, jfieldID p2, jshort p3); /* 155 */
void SetStaticIntField(JNIEnv * env, jclass p1, jfieldID p2, jint p3);
void SetStaticLongField(JNIEnv * env, jclass p1, jfieldID p2, jlong p3);
void SetStaticFloatField(JNIEnv * env, jclass p1, jfieldID p2, jfloat p3);
void SetStaticDoubleField(JNIEnv * env, jclass p1, jfieldID p2, jdouble p3);
jstring NewString(JNIEnv * env, const jchar * p1, jsize p2); /* 160 */
jsize GetStringLength(JNIEnv * env, jstring p1);
const jchar * GetStringChars(JNIEnv * env, jstring p1, jboolean * p2);
void ReleaseStringChars(JNIEnv * env, jstring p1, const jchar * p2);
jstring NewStringUTF(JNIEnv * env, const char * p1);
jsize GetStringUTFLength(JNIEnv * env, jstring p1); /* 165 */
const jbyte * GetStringUTFChars(JNIEnv * env, jstring p1, jboolean * p2);
void ReleaseStringUTFChars(JNIEnv * env, jstring p1, const char * p2);
jsize GetArrayLength(JNIEnv * env, jarray p1);
jobjectArray NewObjectArray(JNIEnv * env, jsize p1, jclass p2, jobject p3);
jobject GetObjectArrayElement(JNIEnv * env, jobjectArray p1, jsize p2); /* 170 */
void SetObjectArrayElement(JNIEnv * env, jobjectArray p1, jsize p2, jobject p3); 
jbooleanArray NewBooleanArray(JNIEnv * env, jsize p1);
jbyteArray NewByteArray(JNIEnv * env, jsize p1);
jcharArray NewCharArray(JNIEnv * env, jsize p1);
jshortArray NewShortArray(JNIEnv * env, jsize p1); /* 175 */
jintArray NewIntArray(JNIEnv * env, jsize p1);
jlongArray NewLongArray(JNIEnv * env, jsize p1);
jfloatArray NewFloatArray(JNIEnv * env, jsize p1);
jdoubleArray NewDoubleArray(JNIEnv * env, jsize p1);
jboolean * GetBooleanArrayElements(JNIEnv * env, jbooleanArray p1, jboolean * p2); /* 180 */
jbyte * GetByteArrayElements(JNIEnv * env, jbyteArray p1, jboolean * p2);
jchar * GetCharArrayElements(JNIEnv * env, jcharArray p1, jboolean * p2);
jshort * GetShortArrayElements(JNIEnv * env, jshortArray p1, jboolean * p2);
jint * GetIntArrayElements(JNIEnv * env, jintArray p1, jboolean * p2);
jlong * GetLongArrayElements(JNIEnv * env, jlongArray p1, jboolean * p2); /* 185 */
jfloat * GetFloatArrayElements(JNIEnv * env, jfloatArray p1, jboolean * p2);
jdouble * GetDoubleArrayElements(JNIEnv * env, jdoubleArray p1, jboolean * p2);
void ReleaseBooleanArrayElements(JNIEnv * env, jbooleanArray p1, jboolean * p2, jint p3);
void ReleaseByteArrayElements(JNIEnv * env, jbyteArray p1, jbyte * p2, jint p3);
void ReleaseCharArrayElements(JNIEnv * env, jcharArray p1, jchar * p2, jint p3); /* 190 */
void ReleaseShortArrayElements(JNIEnv * env, jshortArray p1, jshort * p2, jint p3);
void ReleaseIntArrayElements(JNIEnv * env, jintArray p1, jint * p2, jint p3);
void ReleaseLongArrayElements(JNIEnv * env, jlongArray p1, jlong * p2, jint p3);
void ReleaseFloatArrayElements(JNIEnv * env, jfloatArray p1, jfloat * p2, jint p3);
void ReleaseDoubleArrayElements(JNIEnv * env, jdoubleArray p1, jdouble * p2, jint p3); /* 195 */
void GetBooleanArrayRegion(JNIEnv * env, jbooleanArray p1, jsize p2, jsize p3, jboolean * p4);
void GetByteArrayRegion(JNIEnv * env, jbyteArray p1, jsize p2, jsize p3, jbyte * p4);
void GetCharArrayRegion(JNIEnv * env, jcharArray p1, jsize p2, jsize p3, jchar * p4);
void GetShortArrayRegion(JNIEnv * env, jshortArray p1, jsize p2, jsize p3, jshort * p4);
void GetIntArrayRegion(JNIEnv * env, jintArray p1, jsize p2, jsize p3, jint * p4); /* 200 */
void GetLongArrayRegion(JNIEnv * env, jlongArray p1, jsize p2, jsize p3, jlong * p4);
void GetFloatArrayRegion(JNIEnv * env, jfloatArray p1, jsize p2, jsize p3, jfloat * p4);
void GetDoubleArrayRegion(JNIEnv * env, jdoubleArray p1, jsize p2, jsize p3, jdouble * p4);
void SetBooleanArrayRegion(JNIEnv * env, jbooleanArray p1, jsize p2, jsize p3, const jboolean * p4);
void SetByteArrayRegion(JNIEnv * env, jbyteArray p1, jsize p2, jsize p3, const jbyte * p4); /* 205 */
void SetCharArrayRegion(JNIEnv * env, jcharArray p1, jsize p2, jsize p3, const jchar * p4);
void SetShortArrayRegion(JNIEnv * env, jshortArray p1, jsize p2, jsize p3, const jshort * p4);
void SetIntArrayRegion(JNIEnv * env, jintArray p1, jsize p2, jsize p3, const jint * p4);
void SetLongArrayRegion(JNIEnv * env, jlongArray p1, jsize p2, jsize p3, const jlong * p4);
void SetFloatArrayRegion(JNIEnv * env, jfloatArray p1, jsize p2, jsize p3, const jfloat * p4); /* 210 */
void SetDoubleArrayRegion(JNIEnv * env, jdoubleArray p1, jsize p2, jsize p3, const jdouble * p4);
jint RegisterNatives(JNIEnv * env, jclass p1, const JNINativeMethod * p2, jint p3);
jint UnregisterNatives(JNIEnv * env, jclass p1);
jint MonitorEnter(JNIEnv * env, jobject p1);
jint MonitorExit(JNIEnv * env, jobject p1); /* 215 */
jint GetJavaVM(JNIEnv * env, JavaVM ** p1);
void GetStringRegion(JNIEnv * env, jstring p1, jsize p2, jsize p3, jchar * p4);
void GetStringUTFRegion(JNIEnv * env, jstring p1, jsize p2, jsize p3, char * p4);
void * GetPrimitiveArrayCritical(JNIEnv * env, jarray p1, jboolean * p2);
void ReleasePrimitiveArrayCritical(JNIEnv * env, jarray p1, void * p2, jint p3); /* 220 */
const jchar * GetStringCritical(JNIEnv * env, jstring p1, jboolean * p2);
void ReleaseStringCritical(JNIEnv * env, jstring p1, const jchar * p2);
jweak NewWeakGlobalRef(JNIEnv * env, jobject p1);
void DeleteWeakGlobalRef(JNIEnv * env, jweak p1);
jboolean ExceptionCheck(JNIEnv * env); /* 225 */
jobject NewDirectByteBuffer(JNIEnv * env, void * p1, jlong p2);
void * GetDirectBufferAddress(JNIEnv * env, jobject p1);
jlong GetDirectBufferCapacity(JNIEnv * env, jobject p1);
jobjectRefType GetObjectRefType(JNIEnv * env, jobject p1);

/*
jint GetVersion(JNIEnv * env);

jsize GetStringUTFLength(JNIEnv *env, jstring string);
const jbyte * GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy);
void ReleaseStringUTFChars(JNIEnv * env, jstring string, const char * utf);
void GetStringUTFRegion(JNIEnv *env, jstring str, jsize start, jsize len, char *buf);
jstring NewStringUTF(JNIEnv *env, const char *bytes);

jsize GetArrayLength(JNIEnv *env, jarray array);
void GetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);
void SetByteArrayRegion(JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf);  
void GetCharArrayRegion(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
void SetCharArrayRegion(JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf);
void GetIntArrayRegion(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);
void SetIntArrayRegion(JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf);

jchar *GetCharArrayElements(JNIEnv *env, jcharArray array, jboolean *isCopy);
void ReleaseCharArrayElements(JNIEnv *env, jcharArray array, jchar *elems, jint mode);
jint *GetIntArrayElements(JNIEnv *env, jintArray array, jboolean *isCopy);
void ReleaseIntArrayElements(JNIEnv *env, jintArray array, jint *elems, jint mode);

jclass GetObjectClass(JNIEnv *env, jobject obj);
jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig);

jboolean GetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID); 
void SetBooleanField(JNIEnv *env, jobject obj, jfieldID fieldID, jboolean value); 
jlong GetLongField(JNIEnv *env, jobject obj, jfieldID fieldID); 
jint GetIntField(JNIEnv *env, jobject obj, jfieldID fieldID);
void SetIntField(JNIEnv *env, jobject obj, jfieldID fieldID, jint value);
jobject GetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID);

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
jint GetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID);
void SetStaticIntField(JNIEnv *env, jclass clazz, jfieldID fieldID, jint value);

jclass FindClass(JNIEnv *env, const char *name);

jintArray NewIntArray(JNIEnv *env, jsize length);
jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement);
void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject value);
void DeleteLocalRef(JNIEnv *env, jobject localRef);

jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig);
void CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...); 
void CallVoidMethodA(JNIEnv *env, jobject obj, jmethodID methodID, jvalue * args);
void CallIntMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...);
void CallStaticVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...);

jthrowable ExceptionOccurred(JNIEnv *env);
void ExceptionDescribe(JNIEnv *env);
void ExceptionClear(JNIEnv *env);
jint ThrowNew(JNIEnv *env, jclass clazz, const char *message);

jint MonitorEnter(JNIEnv *env, jobject obj);
jint MonitorExit(JNIEnv *env, jobject obj);

void * GetPrimitiveArrayCritical(JNIEnv *env, jarray array, jboolean *isCopy);
void ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array, void *carray, jint mode);

jbyte *GetByteArrayElements(JNIEnv *env, jbyteArray array, jboolean *isCopy);
void ReleaseByteArrayElements(JNIEnv *env, jbyteArray array, jbyte *elems, jint mode);
jbyteArray NewByteArray(JNIEnv *env, jsize length);
jlongArray NewLongArray(JNIEnv *env, jsize length);
jlong *GetLongArrayElements(JNIEnv *env, jlongArray array, jboolean *isCopy);
void ReleaseLongArrayElements(JNIEnv *env, jlongArray array, jlong *elems, jint mode);
void GetLongArrayRegion(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);
void SetLongArrayRegion(JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf);

jboolean CallBooleanMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...); 

void SetObjectField(JNIEnv *env, jobject obj, jfieldID fieldID, jobject value);
jobject CallObjectMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...); 
jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index);
void SetShortField(JNIEnv *env, jobject obj, jfieldID fieldID, jshort value); 
*/
