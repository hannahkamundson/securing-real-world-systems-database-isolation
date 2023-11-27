#include <jni.h>
#include "HPdfDoc.h"

/*JNIEXPORT jint JNICALL Java_hpdf_JpegPdfTest_hello(JNIEnv *env, jobject obj)
{
	printf("Hello!\n");
	//return (jint) HPDF_New(NULL, NULL, NULL, 0, 0);
	return 0;
}*/

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfDoc_HPDFNew(JNIEnv *env, jobject obj)
{
	jint temp = (jint) HPDF_New(NULL, NULL, NULL, 0, 0);
	return temp;
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfDoc_HPDFSetCompressionMode (JNIEnv *env, jobject obj, jint hpdf, jint mode)
{
	HPDF_SetCompressionMode(hpdf, mode);	
}

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfDoc_HPDFGetFont (JNIEnv *env, jobject obj, jint hpdf, jstring font_name, jstring encoding_name)
{
	jint temp;
	//convert Java strings to C-strings
	char * font_name_utf = (*env)->GetStringUTFChars(env, font_name, NULL);

	if(encoding_name != NULL)
	{
		char * encoding_name_utf = (*env)->GetStringUTFChars(env, encoding_name, NULL);
		temp = HPDF_GetFont(hpdf, font_name_utf, encoding_name_utf);
		(*env)->ReleaseStringUTFChars(env, encoding_name, encoding_name_utf);
	}
	else
	{
		temp = HPDF_GetFont(hpdf, font_name_utf, NULL);
	}

	(*env)->ReleaseStringUTFChars(env, font_name, font_name_utf);

	return temp;
}

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfDoc_HPDFAddPage (JNIEnv *env, jobject obj, jint hpdf)
{
	jint temp = HPDF_AddPage(hpdf);
	//printf("HPDF_New(hpdf:%x) => %x \n", hpdf, temp);
	return temp;
}

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfDoc_HPDFSetOpenAction (JNIEnv *env, jobject obj, jint hpdf, jint handle)
{
	return HPDF_SetOpenAction(hpdf, handle);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfDoc_HPDFSaveToFile (JNIEnv *env, jobject obj, jint hpdf, jstring file)
{
	jbyte * file_utf = (*env)->GetStringUTFChars(env, file, NULL);
	HPDF_SaveToFile(hpdf, file_utf);
	(*env)->ReleaseStringUTFChars(env, file, file_utf);
}
