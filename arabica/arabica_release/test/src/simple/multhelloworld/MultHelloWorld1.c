#include <jni.h>
#include "MultHelloWorld1.h"

JNIEXPORT void JNICALL
Java_simple_multhelloworld_MultHelloWorldTest_print1 (JNIEnv *env, jobject obj)
{
  printf("[Native Sandbox 1] Hello world from a sandboxed library in sandbox 1!\n");
  fflush(stdout);
}
