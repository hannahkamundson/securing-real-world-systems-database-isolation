// From JNI spec book p45

#include <jni.h>
#include "StaticFieldAccess.h"

 JNIEXPORT void JNICALL 
 Java_jni_fieldaccess_StaticFieldAccessTest_accessField(JNIEnv *env, jobject obj)
{
  jfieldID fid;   /* store the field ID */
  jint si;
 
  /* Get a reference to obj's class */
  jclass cls = (*env)->GetObjectClass(env, obj);

  if(cls == NULL)
  {
	return;
  }

  //jclass other_cls = (*env)->GetObjectClass(env, other_obj);

  //printf("In C:\n");
 
  /* Look for the static field si in cls */
  fid = (*env)->GetStaticFieldID(env, cls, "si", "I");
  //other_fid = (*env)->GetStaticFieldID(env, other_cls, "asi", "I");

  if (fid == NULL) {
    return; /* field not found */
  }

  //test access control by passing a bad class
  //cls=NULL;

  /* Access the static field si */
  si = (*env)->GetStaticIntField(env, cls, fid);
  
  //printf("Should be 100: %d\n", si);
  if(si != 100)
  {
	return;
  }

  (*env)->SetStaticIntField(env, cls, fid, 200);

  //test access control by passing a bad fid which belongs to
  //another class
  //printf("Should fail:\n");
  //(*env)->SetStaticIntField(env, cls, other_fid, 43);
}

