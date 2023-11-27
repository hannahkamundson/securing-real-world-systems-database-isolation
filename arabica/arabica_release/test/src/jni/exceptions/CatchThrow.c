// From JNI spec book p74

#include <jni.h>
#include "CatchThrow.h"
 
JNIEXPORT void JNICALL 
 Java_jni_exceptions_CatchThrowTest_doit(JNIEnv *env, jobject obj)
{
  jthrowable exc;
  jclass cls = (*env)->GetObjectClass(env, obj);
  jmethodID mid = 
    (*env)->GetMethodID(env, cls, "callback", "()V");
  if (mid == NULL) {
    return;
  }
  (*env)->CallVoidMethod(env, obj, mid);
  exc = (*env)->ExceptionOccurred(env);
  //printf("[Native Sandbox] Should now display a NullPointerException:\n");
  //fflush(stdout);
  if (exc) {
    /* We don't do much with the exception, except that
       we print a debug message for it, clear it, and 
       throw a new exception. */
    jclass newExcCls;
    //(*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    newExcCls = (*env)->FindClass(env, 
				  "java/lang/IllegalArgumentException");
    if (newExcCls == NULL) {
      /* Unable to find the exception class, give up. */
      return;
    }
       (*env)->ThrowNew(env, newExcCls, "thrown from C code");
  }
  //else (!exc) - in this case it won't throw an exception so the test will fail

}

