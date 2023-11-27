#include <stdio.h>
#include <jni.h>
#include "Blank.h"

JNIEXPORT void JNICALL
Java_simple_blank_BlankTest_blank(JNIEnv *env, jobject obj)
{
  jint i, j;
  for (i = 0; i < 20000; i++)
    for (j = 0; j < 20000; j++) ;

}
