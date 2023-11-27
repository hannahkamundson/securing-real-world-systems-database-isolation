#include <jni.h>
#include "HPdfImage.h"

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfImage_HPDFLoadJpegImageFromFile(JNIEnv *env, jobject obj, jint hpdf, jstring filename)
{
	jbyte * filename_utf = (*env)->GetStringUTFChars(env, filename, NULL);
	return HPDF_LoadJpegImageFromFile(hpdf, filename_utf);
	(*env)->ReleaseStringUTFChars(env, filename, filename_utf);
}

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfImage_HPDFGetWidth(JNIEnv *env, jobject obj, jint img)
{
	return HPDF_Image_GetWidth(img);
}

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfImage_HPDFGetHeight(JNIEnv *env, jobject obj, jint img)
{
	return HPDF_Image_GetHeight(img);
}
