package jni.onload;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;

import fixture.RunParameters;

public class JNIOnloadTest extends TestCase
{
    public native void print();

    @Test
    public void test()
    {
	print();
    }
    static
    {
	System.loadLibrary("JNIOnload");
    }
}
