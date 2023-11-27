#include "robusta_jni.h"

//the jvm parameter should be a JVM pointer, but since 
//this is only a temporary hack, we're just going to
//leave it as a void pointer to make things simpler.
void * JNU_GetEnv(void *jvm, int version)
{
	return GetGlobalJNIEnv();
}

void JNU_ThrowNullPointerException(JNIEnv * env, int num)
{
	//eventually map this to a system call
}

void JNU_ThrowByName(JNIEnv *env, char * type, char * buf)
{
	//eventually map this to a system call
}
