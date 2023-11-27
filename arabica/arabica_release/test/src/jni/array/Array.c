#include <jni.h>
#include "Array.h"
#include <stdlib.h>
#include <stdarg.h>

JNIEXPORT jint JNICALL 
Java_jni_array_ArrayTest_sumArray(JNIEnv * env, jobject obj, jintArray arr)
{
  //GetArrayLength makes sure that the argument must be of array type
  //jsize len = (*env) -> GetArrayLength(env, obj);
  //jsize len = (*env) -> GetArrayLength(env, 0); // have errors during jni transformations

  // not allowing casts from safe (seq, wild) pointers to handle pointers
  /*struct _jobject * __SAFE p = 0;
  jsize len = (*env)->GetArrayLength(env, p);*/

  //printf("[intArray.c: sumArray:] arr: %p\n", arr);
  //fflush(stdout);

  jsize len = (*env)->GetArrayLength(env, arr);

  //printf("[intArray.c: sumArray:] Array length should be 10: %i\n", len);
  //fflush(stdout);

  int i, sum = 0;
  
  //the argument must be of intarray type
  //jint * body = (*env)->GetIntArrayElements(env, obj, 0);

  jint * body = (*env)->GetIntArrayElements(env, arr, 0);
  //out of bound read
  //for (i=0; i<len+10; i++) {
  for (i=0; i<len; i++) {
    sum += body[i];
  }
  //printf("[intArray.c: sumArray:] The sum should be 45: sum= %i\n", sum);
  //fflush(stdout);
  (*env)->ReleaseIntArrayElements(env, arr, body, 0);

  //direct read
  /*int *p = (*((int **) obj));
  for (i=0;i<10;i++) {
    printf("%i\t",(*(p+i)));
  }*/
 
  //read from a handle pointer
  //struct _jobject o = * obj;

  //write to a read-only pointer
  //(*env)->GetArrayLength = 0;
  
  return sum;
}

JNIEXPORT jbyte JNICALL Java_jni_array_ArrayTest_testGetReleaseByteArrayElementsNative(JNIEnv * env, jobject obj, jbyteArray arr)
{
	jbyte sum;
	jsize len = (*env)->GetArrayLength(env, arr);

	jbyte * body = (*env)->GetByteArrayElements(env, arr, 0);

	int i=0;

	for(i=0; i<len; i++)
	{
		sum+=body[i];
	}
	
	(*env)->ReleaseByteArrayElements(env, arr, body, 0);

	return sum;
}

JNIEXPORT jbyteArray JNICALL Java_jni_array_ArrayTest_testNewByteArrayNative(JNIEnv * env, jobject obj)
{
	return (*env)->NewByteArray(env, 10);
}

JNIEXPORT jlongArray JNICALL Java_jni_array_ArrayTest_testNewLongArrayNative(JNIEnv * env, jobject obj)
{
	return (*env)->NewLongArray(env, 10);
}

JNIEXPORT jlong JNICALL Java_jni_array_ArrayTest_testGetSetLongArrayRegionNative(JNIEnv * env, jobject obj, jlongArray arr)
{
	jlong sum=0;

	jsize len = (*env)->GetArrayLength(env, arr);

	jlong * buf = (jlong*) malloc(len*sizeof(jlong));

	(*env)->GetLongArrayRegion(env, arr, 0, len, buf);

	int i;

	for(i=0; i<len; i++)
	{
		sum += buf[i];
	}

	buf[0]=42;

	(*env)->SetLongArrayRegion(env, arr, 0, len, buf);

	return sum;
}

JNIEXPORT jlong JNICALL Java_jni_array_ArrayTest_testGetReleaseLongArrayElementsNative(JNIEnv * env, jobject obj, jlongArray arr)
{
	jlong sum=0;

	jsize len = (*env)->GetArrayLength(env, arr);

	jlong * body = (*env)->GetLongArrayElements(env, arr, 0);

	int i;

	for(i=0; i<len; i++)
	{
		sum+=body[i];
	}

	body[0]=42;
	
	(*env)->ReleaseLongArrayElements(env, arr, body, 0);
	
	return sum;
}

JNIEXPORT jobject JNICALL Java_jni_array_ArrayTest_testGetObjectArrayElementNative(JNIEnv * env, jobject obj, jobjectArray arr, jint index)
{
	return (*env)->GetObjectArrayElement(env, arr, index);
}

