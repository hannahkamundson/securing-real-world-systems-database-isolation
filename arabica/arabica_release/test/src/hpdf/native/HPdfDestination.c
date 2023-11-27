#include <jni.h>
#include "HPdfDestination.h"

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfDestination_HPDFSetXYZ(JNIEnv *env, jobject obj, jint hdest, jfloat left, jfloat top, jfloat zoom)
{
	HPDF_Destination_SetXYZ_1(hdest, (int) left, (int) top, (int) zoom);
}

