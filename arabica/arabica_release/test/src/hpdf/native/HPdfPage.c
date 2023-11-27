#include <jni.h>
#include "hpdf_types.h"
#include "HPdfPage.h"

JNIEXPORT jfloat JNICALL Java_hpdf_impl_HPdfPage_HPDFGetHeight(JNIEnv *env, jobject obj, jint hpage)
{
	return (jfloat) HPDF_Page_GetHeight_1(hpage);
}

JNIEXPORT jfloat JNICALL Java_hpdf_impl_HPdfPage_HPDFGetWidth(JNIEnv *env, jobject obj, jint hpage)
{
	return (jfloat) HPDF_Page_GetWidth_1(hpage);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetHeight(JNIEnv *env, jobject obj, jint hpage, jfloat value)
{
	//HPDF_Page_SetHeight(hpage, value);
	HPDF_Page_SetHeight_1(hpage, (int) value);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetWidth(JNIEnv *env, jobject obj, jint hpage, jfloat value)
{
	HPDF_Page_SetWidth_1(hpage, (int) value);
}

JNIEXPORT jint JNICALL Java_hpdf_impl_HPdfPage_HPDFCreateDestination(JNIEnv *env, jobject obj, jint hpage)
{
	return HPDF_Page_CreateDestination(hpage);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFBeginText(JNIEnv *env, jobject obj, jint hpage)
{
	HPDF_Page_BeginText(hpage);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetFontAndSize(JNIEnv *env, jobject obj, jint hpage, jint handle, jfloat size)
{
	HPDF_Page_SetFontAndSize_1(hpage, handle, (int) size);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFMoveTextPos(JNIEnv *env, jobject obj, jint hpage, jfloat x, jfloat y)
{
	HPDF_Page_MoveTextPos_1(hpage, (int) x, (int) y);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFShowText(JNIEnv *env, jobject obj, jint hpage, jstring text)
{
	jbyte * text_utf = (*env)->GetStringUTFChars(env, text, NULL);
	HPDF_Page_ShowText(hpage, text_utf);
	(*env)->ReleaseStringUTFChars(env, text, text_utf);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFEndText(JNIEnv *env, jobject obj, jint hpage)
{
	HPDF_Page_EndText(hpage);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFDrawImage(JNIEnv *env, jobject obj, jint hpage, jint img, jint x, jint y, jint width, jint height)
{
	HPDF_Page_DrawImage_1(hpage, img, x, y, width, height);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetRGBFill(JNIEnv *env, jobject obj, jint hpage, jfloat r, jfloat g, jfloat b)
{
	HPDF_Page_SetRGBFill_1(hpage, (double)r, (double)g, (double)b);
}

/*JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_show_description(JNIEnv *env, jobject obj, jint hpage, jfloat x, jfloat y, jstring text)
{
	jbyte * text_utf = (*env)->GetStringUTFChars(env, text, NULL);
	float fsize = HPDF_Page_GetCurrentFontSize (hpage);
    	HPDF_Font font = HPDF_Page_GetCurrentFont (hpage);
    	HPDF_RGBColor c = HPDF_Page_GetRGBFill (hpage);

    	//HPDF_Page_BeginText (hpage);
    	HPDF_Page_SetRGBFill (hpage, 0, 0, 0);
    	HPDF_Page_SetTextRenderingMode (hpage, HPDF_FILL);
    	HPDF_Page_SetFontAndSize (hpage, font, 10);
    	HPDF_Page_TextOut (hpage, x, y - 12, text_utf);
    	//HPDF_Page_EndText (hpage);

    	HPDF_Page_SetFontAndSize (hpage, font, fsize);
    	HPDF_Page_SetRGBFill (hpage, c.r, c.g, c.b);

	(*env)->ReleaseStringUTFChars(env, text, text_utf);
}*/

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetTextMatrix(JNIEnv *env, jobject obj, jint hpage, jfloat val1, jfloat val2, jfloat val3, jfloat val4, jfloat xpos, jfloat ypos)
{
	HPDF_Page_SetTextMatrix_1(hpage, (double)val1, (double)val2, (double)val3, (double)val4, (double)xpos, (double)ypos);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFTextOut(JNIEnv *env, jobject obj, jint hpage, jfloat x, jfloat y, jstring text)
{
	jbyte * text_utf = (*env)->GetStringUTFChars(env, text, NULL);
	HPDF_Page_TextOut_1(hpage, (int)x, (int)y, text_utf);
	(*env)->ReleaseStringUTFChars(env, text, text_utf);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetCharSpace(JNIEnv *env, jobject obj, jint hpage, jfloat space)
{
	HPDF_Page_SetCharSpace_1(hpage, (int)space);
}

JNIEXPORT void JNICALL Java_hpdf_impl_HPdfPage_HPDFSetWordSpace(JNIEnv *env, jobject obj, jint hpage, jfloat space)
{
	HPDF_Page_SetWordSpace_1(hpage, (int)space);
}

