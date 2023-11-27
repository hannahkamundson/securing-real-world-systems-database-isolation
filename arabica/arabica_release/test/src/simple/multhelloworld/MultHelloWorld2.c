#include <jni.h>
#include "MultHelloWorld2.h"

JNIEXPORT void JNICALL
Java_simple_multhelloworld_MultHelloWorldTest_print2 (JNIEnv *env, jobject obj)
{
  printf("[Native Sandbox 2] Hello world from a sandboxed library in sandbox 2!\n");
  fflush(stdout);
}
