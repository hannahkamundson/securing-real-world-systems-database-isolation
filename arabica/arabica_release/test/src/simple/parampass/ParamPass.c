
#include <jni.h>
#include "ParamPass.h"

JNIEXPORT jint JNICALL Java_simple_parampass_ParamPassTest_pass1 (JNIEnv *env, jobject obj, jint a)
{
	//printf("[Native sandboxed] The value should be 42: %i\n", a);
	//fflush(stdout);
	return a;
}

JNIEXPORT jint JNICALL Java_simple_parampass_ParamPassTest_pass2 (JNIEnv *env, jobject obj, jint a, jint b)
{
	//printf("[Native sandboxed] The values should be 42 42: %i %i\n", a, b);
	//fflush(stdout);
	return a+b;
}

JNIEXPORT jint JNICALL Java_simple_parampass_ParamPassTest_pass3 (JNIEnv *env, jobject obj, jint a, jint b, jint c)
{
	//printf("[Native sandboxed] The values should be 42 42 42: %i %i %i\n", a, b, c);
	//fflush(stdout);
	return a+b+c;
}

JNIEXPORT jdouble JNICALL Java_simple_parampass_ParamPassTest_passprims(JNIEnv *env, jobject obj, jbyte a, jchar b, jdouble c, jfloat d, jint e, jlong f)
{
	printf("[Native sandboxed] The values should be 4 J 3.14159 3.14159 42 4200: %i %c %f %f %i %li\n", a, b, c, d, e, f);
	fflush(stdout);
	return (jdouble) a + b + c + d + e + f;
}

JNIEXPORT jstring JNICALL Java_simple_parampass_ParamPassTest_passobjtypes (JNIEnv *env, jobject obj, jclass a, jstring b)
{
	//printf("[Native sandboxed] Should be reference values: %p %p\n", a, b);
	//fflush(stdout);
	return b;
}

JNIEXPORT jint JNICALL Java_simple_parampass_ParamPassTest_passstatic (JNIEnv *env, jclass cls, jint a)
{
	//printf("[Native sandboxed] Static native method - The value should be 42: %i\n", a);
	//fflush(stdout);
	return a;
}

JNIEXPORT jboolean JNICALL Java_simple_parampass_ParamPassTest_returnboolean (JNIEnv * env, jobject obj)
{
	jboolean test=1;
	return test;
}

JNIEXPORT jbyte JNICALL Java_simple_parampass_ParamPassTest_returnbyte (JNIEnv * env, jobject obj)
{
	jbyte test=0xFF;
	return test;
}

JNIEXPORT jchar JNICALL Java_simple_parampass_ParamPassTest_returnchar (JNIEnv * env, jobject obj)
{
	jchar test='J';
	return test;
}

JNIEXPORT jshort JNICALL Java_simple_parampass_ParamPassTest_returnshort (JNIEnv * env, jobject obj)
{
	jshort test=0xFFFF;
	return test;
}

JNIEXPORT jint JNICALL Java_simple_parampass_ParamPassTest_returnint (JNIEnv * env, jobject obj)
{
	jint test=0xFFFFFFFF;
	return test;
}

//when the return type is two bytes, the compiler returns the results in eax and edx
JNIEXPORT jlong JNICALL Java_simple_parampass_ParamPassTest_returnlong (JNIEnv * env, jobject obj)
{
	jlong test=(0xAFFFFFFFFL);
	printf("[Native] test: %lld\n", test);
	fflush(stdout);
	return test;
}

JNIEXPORT jfloat JNICALL Java_simple_parampass_ParamPassTest_returnfloat (JNIEnv *env, jobject obj)
{
	jfloat test = 3.14159;
	return test;
}

JNIEXPORT jdouble JNICALL Java_simple_parampass_ParamPassTest_returndouble (JNIEnv *env, jobject obj)
{
  jdouble test = (double) 3.14159;
  return test;
}

JNIEXPORT jobject JNICALL Java_simple_parampass_ParamPassTest_returnjobject(JNIEnv *env, jobject obj)
{
	return obj;
}

	/*int i;
	int * env_addr = (int*)&env;
	printf("env addr= %p\n", env_addr);
	for(i=-20; i<20; i++)
	{
		printf(" %i : %i\n", i, *((int*)env_addr+i) );
	}
	b='c';
	printf(" %i %c\n", a, b);*/
