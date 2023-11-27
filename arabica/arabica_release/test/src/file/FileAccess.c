#include "FileAccess.h"
#include <stdio.h>
#include <fcntl.h>

JNIEXPORT jint JNICALL Java_file_FileAccessTest_testFileRead(JNIEnv * env, jobject obj, jstring str)
{
	const jbyte * c_str = (*env)->GetStringUTFChars(env, str, NULL);
	int dest=0;

	FILE *fp;

	if((fp=open(c_str, O_RDONLY))==NULL)
	{
		return -1;
	}

	fscanf(fp, "%i", &dest);

	(*env)->ReleaseStringUTFChars(env, str, c_str);

	close(fp);

	return dest;
}

JNIEXPORT jint JNICALL Java_file_FileAccessTest_testFileWrite(JNIEnv * env, jobject obj, jstring str)
{
	const jbyte * c_str = (*env)->GetStringUTFChars(env, str, NULL);	
	int num_bytes;

	FILE * fp;

	if((fp=fopen(c_str, "a"))==NULL)
	{
		return -1;	
	}

	//printf("write fp: %p\n", fp);
	//fflush(stdout);	

	(*env)->ReleaseStringUTFChars(env, str, c_str);
	
	num_bytes = fprintf(fp, "%i", 42);
	
	fclose(fp);
	
	return num_bytes;
}

JNIEXPORT jint JNICALL Java_file_FileAccessTest_testFileReadF(JNIEnv * env, jobject obj, jstring str)
{
	const jbyte * c_str = (*env)->GetStringUTFChars(env, str, NULL);
	int dest=0;

	FILE *fp;

	if((fp=fopen(c_str, "r"))==NULL)
	{
		return -1;
	}

	fscanf(fp, "%i", &dest);

	(*env)->ReleaseStringUTFChars(env, str, c_str);

	fclose(fp);

	return dest;
}

JNIEXPORT jint JNICALL Java_file_FileAccessTest_testFileWriteF(JNIEnv * env, jobject obj, jstring str)
{
	const jbyte * c_str = (*env)->GetStringUTFChars(env, str, NULL);	
	int num_bytes;

	FILE * fp;

	if((fp=fopen(c_str, "w"))==NULL)
	{
		return -1;	
	}

	//printf("write fp: %p\n", fp);
	//fflush(stdout);	

	(*env)->ReleaseStringUTFChars(env, str, c_str);
	
	num_bytes = fprintf(fp, "%i", 42);
	
	fclose(fp);
	
	return num_bytes;
}

JNIEXPORT jint JNICALL Java_file_FileAccessTest_testFileReadWrite(JNIEnv * env, jobject obj, jstring str)
{
	const jbyte * c_str = (*env)->GetStringUTFChars(env, str, NULL);	
	int num_bytes;
	int dest=0;

	FILE * fp;

	if((fp=fopen(c_str, "a"))==NULL)
	{
		printf("File open failed\n");
		return -1;	
	}

	//printf("write fp: %p\n", fp);
	//fflush(stdout);	

	(*env)->ReleaseStringUTFChars(env, str, c_str);
	
	num_bytes = fprintf(fp, "%i ", 42);
	
	rewind(fp);
	
	fscanf(fp, "%i ", &dest);

	fclose(fp);
	
	return (num_bytes + dest);
}
