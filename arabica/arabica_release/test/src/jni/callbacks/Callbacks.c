#include <stdio.h>
#include <jni.h>
#include "Callbacks.h"

JNIEXPORT jint JNICALL 
Java_jni_callbacks_CallbacksTest_nativeMethod(JNIEnv *env, jobject obj, jint depth)
{
  jclass cls = (*env)->GetObjectClass(env, obj);
  jmethodID mid = (*env)->GetMethodID(env, cls, "callback", "(IZ)I");
  if (mid == 0) {
    return depth;
  }
  
  //printf("In C, depth = %d, about to enter Java\n", depth);
  //fflush(stdout);
  return (*env)->CallIntMethod(env, obj, mid, depth, 1);
  //printf("In C, depth = %d, back from Java\n", depth);
  //fflush(stdout);
}

JNIEXPORT jboolean JNICALL 
Java_jni_callbacks_CallbacksTest_testCallBooleanMethodNative(JNIEnv *env, jobject obj, jint val)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
  	jmethodID mid = (*env)->GetMethodID(env, cls, "booleanMethod", "(I)Z");

	if (mid == 0)
	{
    		return 0;
 	}


	return (*env)->CallBooleanMethod(env, obj, mid, val);

}

JNIEXPORT jobject JNICALL 
Java_jni_callbacks_CallbacksTest_testCallObjectMethodNative(JNIEnv *env, jobject obj, jobject val)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
  	jmethodID mid = (*env)->GetMethodID(env, cls, "objectMethod", "(Ljava/lang/Object;)Ljava/lang/Object;");

	if (mid == 0)
	{
    		return 0;
 	}


	return (*env)->CallObjectMethod(env, obj, mid, val);
}

JNIEXPORT jint JNICALL
Java_jni_callbacks_CallbacksTest_testCallIntMethodNative(JNIEnv *env, jobject obj, jbyteArray arr, jint base, jint length)
{
	jclass CallbacksTest = (*env)->GetObjectClass(env, obj);

	jmethodID passByteArrIntInt_id = (*env)->GetMethodID(env, CallbacksTest, "passByteArrIntInt", "([BII)I");

	return (*env)->CallIntMethod(env, obj, passByteArrIntInt_id, arr, 0, length);
}

JNIEXPORT jint JNICALL
Java_jni_callbacks_CallbacksTest_testInputStreamCallbackNative(JNIEnv *env, jobject obj, jobject is, jbyteArray arr)
{
	int ret, buflen = (*env)->GetArrayLength(env, arr);
	jclass InputStream_class = (*env)->GetObjectClass(env, is);
	
	jmethodID InputStream_readID = (*env)->GetMethodID(env, InputStream_class,
                                             "read", "([BII)I");
	
	ret = (*env)->CallIntMethod(env, is, InputStream_readID, arr, 0, buflen);

	if((*env)->ExceptionOccurred(env))
	{
		printf("Exception occurred!\n");
		(*env)->ExceptionDescribe(env);
	}

	return ret;
}
