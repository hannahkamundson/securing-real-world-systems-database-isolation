#include "MonitorCounter.h"

JNIEXPORT void JNICALL Java_jni_threading_MonitorCounter_counterloop(JNIEnv * env, jobject obj, jint iters)
{
	//printf("[Native Sandbox] env: %p, obj: %p, iters: %p\n", env, obj, iters);
	int i;
	jint counter;

	/* Get a reference to obj's class */
	jclass cls = (*env)->GetObjectClass(env, obj);

	//get the static field ID for the counter
	jfieldID fid = (*env)->GetStaticFieldID(env, cls, "counter", "I");

	for(i=0; i<iters; i++)
	{
		if ((*env)->MonitorEnter(env, obj) != JNI_OK) 
		{
			//error handling (just return so the test fails
			return;
 		}

		//increment the counter
		counter = (*env)->GetStaticIntField(env, cls, fid);

		counter++;

		(*env)->SetStaticIntField(env, cls, fid, counter);

		if ((*env)->MonitorExit(env, obj) != JNI_OK) 
		{
			//error handling (just return so the test fails)
			printf("[MonitorCounter.c: counterloop:] ****FAIL**** Monitor exit failed\n");
			fflush(stdout);
			return;
		};
	}
}
