#include <jni.h>
#include <stdio.h>
#include "JNIOnload.h"

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM * vm, void * reserved)
{
  JNIEnv * env = NULL;
  printf("This is from JNI_OnLoad! vm: %p\n", vm);
  fflush(stdout);
  int result = (*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_2);
  printf("This is from JNI_OnLoad! result: %d, env: %p\n", result, env);
  printf("JNI_OK: %d\n", JNI_OK);
  printf("JNI_EDETACHED: %d\n", JNI_EDETACHED);
  printf("JNI_EVERSION: %d\n", JNI_EVERSION);
  fflush(stdout);
  return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL
Java_jni_onload_JNIOnloadTest_print (JNIEnv *env, jobject obj)
{
  printf("Hello world from print!\n");
  fflush(stdout);
}

