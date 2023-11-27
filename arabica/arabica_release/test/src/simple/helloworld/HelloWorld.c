#include <jni.h>
#include "HelloWorld.h"

JNIEXPORT void JNICALL Java_simple_helloworld_HelloWorldTest_print (JNIEnv *env, jobject obj)
{
  printf("[Native Sandbox] Hello world from a sandboxed library!!\n");
  fflush(stdout);
}
