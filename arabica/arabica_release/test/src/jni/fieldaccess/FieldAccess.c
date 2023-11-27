#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include "FieldAccess.h"

JNIEXPORT jboolean JNICALL Java_jni_fieldaccess_FieldAccessTest_accessFields
  (JNIEnv *env, jobject obj , jobject obj2)
{
  jfieldID fid;
  jint a_pub,a_prv, f_pub, f_prv;

  // access the other class
  /*jclass cls = (*env)->GetObjectClass(env, obj2);

  printf("[FieldAccess.c: accessFields] cls: %p\n", cls);
  fflush(stdout);

  fid = (*env)->GetFieldID(env, cls, "f_pub", "I");
  if (fid == 0) {
    return;
  }
  a_pub = (*env)->GetIntField(env, obj2, fid);
  printf("a_pub=%i\n",a_pub);

  fid = (*env)->GetFieldID(env, cls, "a_prv", "I");
  if (fid == 0) {
    return;
  }
  a_prv = (*env)->GetIntField(env, obj2, fid);
  printf("a_prv=%i\n",a_prv);*/

  // access self
  
  //printf("[FieldAccess.c: accessFields] obj: %p cls: %p\n", obj, cls);
  //fflush(stdout);


  return 1;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkGetIntField (JNIEnv *env, jobject obj)
{
	jfieldID fid;
	jint f_pub, f_prv;

	jclass cls = (*env)->GetObjectClass(env, obj);
	if(cls==0)
	{
		return -3;
	}

	fid = (*env)->GetFieldID(env, cls, "f_pub", "I");
	if (fid == 0) 
	{
    		return -2;
	}

	f_pub = (*env)->GetIntField(env, obj, fid);

	fid = (*env)->GetFieldID(env, cls, "f_prv", "I");

	if (fid == 0) 
	{
		return -1;
	}

 	 f_prv = (*env)->GetIntField(env, obj, fid);

	return f_pub+f_prv;
}

/*JNIEXPORT jboolean JNICALL Java_jni_fieldaccess_FieldAccessTest_checkGetByteArrayRegion(JNIEnv *env, jobject obj)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env,cls, "bln_pub", "Z"); 
  	bln_pub = (*env)->GetBooleanField(env, obj, fid);
 	
	
}*/


JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkGetLongField (JNIEnv *env, jobject obj)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	if(cls==0)
	{
		return -2;
	}

	jfieldID fid = (*env)->GetFieldID(env,cls, "lng_pub", "J");
	if (fid == 0) 
	{
		return -1;
	}

 	jlong lng_pub = (*env)->GetLongField(env, obj, fid);

	return lng_pub;
}


JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkGetBooleanField (JNIEnv *env, jobject obj)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env,cls, "bln_pub", "Z");
	if (fid == 0) 
	{
		return -1;
	}

  	jboolean bln_pub = (*env)->GetBooleanField(env, obj, fid);
 	
	if(bln_pub != 1)
	{
		return bln_pub;
	}

	return 1;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkGetObjectField (JNIEnv *env, jobject obj)
{
	jint a_pub;
	jclass cls = (*env)->GetObjectClass(env, obj);
	if (cls == 0) 
	{
		return -5;
	}

	jfieldID fid = (*env)->GetFieldID(env,cls, "obj_pub", "Ljni/fieldaccess/A;");
	if (fid == 0) 
	{
		return -4;
	}

  	jobject obj_pub = (*env)->GetObjectField(env, obj, fid);
	if (obj_pub == 0)
	{
		return -3;
	}

  	jclass obj_pub_cls = (*env)->GetObjectClass(env, obj_pub);
	if (obj_pub_cls == 0)
	{
		return -2;
	}

  	fid = (*env)->GetFieldID(env, obj_pub_cls, "a_pub", "I");
	if (fid == 0)
	{
		return -1;
	}

  	a_pub = (*env)->GetIntField(env, obj_pub, fid);

	return a_pub;

	//return obj_pub;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkGetByteArrayRegion (JNIEnv *env, jobject obj, jbyteArray bytearr, jint len)
{
	int i;
	int sum=0;
	jbyte * buf = (jbyte *) malloc(len * sizeof(jbyte));
	(*env)->GetByteArrayRegion(env, bytearr, 0, len, buf); 
	for(i=0; i<len; i++)
	{
		sum+=buf[i];
	}
	return sum;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkSetObjectField (JNIEnv *env, jobject obj, jobject value)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env,cls, "strfld", "Ljava/lang/String;");
	if (fid == 0) 
	{
		return -1;
	}

	(*env)->SetObjectField(env, obj, fid, value);

	return 1;

}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkSetIntField (JNIEnv *env, jobject obj, jint value)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env,cls, "setme", "I");
	if (fid == 0) 
	{
		return -1;
	}

  	(*env)->SetIntField(env, obj, fid, value);

	return 1;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkSetShortField (JNIEnv *env, jobject obj, jshort value)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env,cls, "shortfld", "S");
	if (fid == 0) 
	{
		return -1;
	}

	(*env)->SetShortField(env, obj, fid, value);

	return 1;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkSetBooleanField (JNIEnv *env, jobject obj, jboolean value)
{
	jclass cls = (*env)->GetObjectClass(env, obj);
	jfieldID fid = (*env)->GetFieldID(env, cls, "bln_pub", "Z");
	printf("checkSetBooleanField: env: %p, obj: %p, cls: %p, fid: %p, value: %d\n", env, obj, cls, fid, value);
	fflush(stdout);
	if (fid == 0) 
	{
		return -1;
	}

  	(*env)->SetBooleanField(env, obj, fid, value);
	return 1;
}

JNIEXPORT jint JNICALL Java_jni_fieldaccess_FieldAccessTest_checkSetByteArrayRegion (JNIEnv *env, jobject obj, jbyteArray bytearr, jint len)
{
	int i;
	int sum=0;
	jbyte * buf = (jbyte *) malloc(len * sizeof(jbyte));
	buf[0]=0;
	buf[1]=1;
	buf[2]=2;
	(*env)->SetByteArrayRegion(env, bytearr, 0, len, buf); 

	return 1;
}
