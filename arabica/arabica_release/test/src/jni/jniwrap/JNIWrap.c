#include "JNIWrap.h"

JNIEXPORT jint JNICALL Java_jni_jniwrap_JNIWrapTest_testGetVersion (JNIEnv * env, jobject obj)
{
	/*printf("[Native Sandboxed] testJNICall: The JNIEnv pointer is: %p\n", env);
	fflush(stdout);

	printf("[JNIWrap.c: testJNICall:] env = %p\n", env);
	printf("[JNIWrap.c: testJNICall:] (*env) = %p\n", (*env));
	printf("[JNIWrap.c: testJNICall:] (*env)->Get... = %p\n", (*env)->GetStringUTFChars);*/
	
	//printf("[JNIWrap.c: testGetVersion:] should be 10006: %x\n", (*env)->GetVersion(env));

	return (*env)->GetVersion(env);

	//check(((*env)->GetVersion(env) == 0x10006), "GetVersion() check failed");
	//printf("[JNIWrap.c: testGetVersion:] Finished checks\n");
	//fflush(stdout);
}

JNIEXPORT jint JNICALL Java_jni_jniwrap_JNIWrapTest_testJNIChecks(JNIEnv * env, jobject obj, jstring test, jintArray test2, jbyteArray test3, jcharArray test4, jlongArray test5, jobject temp)
{
	jint n_test2[4];
	jbyte n_test3[4];
	jchar n_test4[4];
	jlong n_test5[4];

	//should succeed
	(*env)->GetStringUTFLength(env, test);

	//should fail
	//(*env)->GetStringUTFLength(env, temp);

	//should succeed	
	(*env)->GetArrayLength(env, test2);

	//should fail
	//(*env)->GetArrayLength(env, temp);

	//should succeed	
	(*env)->GetIntArrayRegion(env, test2, 0, 4, n_test2);

	//should fail
	//(*env)->GetIntArrayRegion(env, temp, 0, 4, n_test2);
	
	//should succeed
	(*env)->GetByteArrayRegion(env, test3, 0, 4, n_test3);

	//should fail
	//(*env)->GetByteArrayRegion(env, temp, 0, 4, n_test3);
	
	//should succeed
	(*env)->GetCharArrayRegion(env, test4, 0, 4, n_test4);

	//should fail
	//(*env)->GetCharArrayRegion(env, temp, 0, 4, n_test4);
	
	//should succeed
	(*env)->GetLongArrayRegion(env, test5, 0, 4, n_test5);

	//should fail
	//(*env)->GetLongArrayRegion(env, temp, 0, 4, n_test5);
}

void check(int isTrue, char * reason)
{
	if(!isTrue)
	{
		printf("[JNIWrap.c: check:] Check failed: %s\n", reason);
	}
} 
