/* gtan: changelog
   - changed #include "java_lang_StrictMath.h" to #include "StrictMath.h".
   - changed all method names from "*_java_lang_StrictMath_*" to
     "*_StrictMath_*".
 */
/*
 * @(#)StrictMath.c	1.55 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "jni.h"
#include "fdlibm.h"

#include "StrictMath.h"

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_cos(JNIEnv *env, jclass unused, jdouble d)
{
    //printf("cos(%f): %f\n", d, jcos((double)d));
    //fflush(stdout);
    return (jdouble) jcos((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_sin(JNIEnv *env, jclass unused, jdouble d)
{
    //printf("sin(%f): %f\n", d, jsin((double)d));
    //fflush(stdout);
    return (jdouble) jsin((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_tan(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jtan((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_asin(JNIEnv *env, jclass unused, jdouble d)
{
    //printf("asin(%f): %f\n", d, jasin((double)d));
    //fflush(stdout);
    return (jdouble) jasin((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_acos(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jacos((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_atan(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jatan((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_exp(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jexp((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_log(JNIEnv *env, jclass unused, jdouble d)
{
    //printf("log(%f): %f\n", d, jlog((double)d));
    //fflush(stdout);
    return (jdouble) jlog((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_log10(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jlog10((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_sqrt(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jsqrt((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_cbrt(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jcbrt((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_ceil(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jceil((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_floor(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jfloor((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_atan2(JNIEnv *env, jclass unused, jdouble d1, jdouble d2)
{
    return (jdouble) jatan2((double)d1, (double)d2);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_pow(JNIEnv *env, jclass unused, jdouble d1, jdouble d2)
{
    return (jdouble) jpow((double)d1, (double)d2);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_IEEEremainder(JNIEnv *env, jclass unused,
                                  jdouble dividend,
                                  jdouble divisor)
{
    return (jdouble) jremainder(dividend, divisor);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_cosh(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jcosh((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_sinh(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jsinh((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_tanh(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jtanh((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_hypot(JNIEnv *env, jclass unused, jdouble x, jdouble y)
{
    return (jdouble) jhypot((double)x, (double)y);
}



JNIEXPORT jdouble JNICALL
Java_math_StrictMath_log1p(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jlog1p((double)d);
}

JNIEXPORT jdouble JNICALL
Java_math_StrictMath_expm1(JNIEnv *env, jclass unused, jdouble d)
{
    return (jdouble) jexpm1((double)d);
}

