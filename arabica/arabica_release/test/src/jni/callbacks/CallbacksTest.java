package jni.callbacks;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

import java.io.InputStream;
import java.io.FileInputStream;

public class CallbacksTest extends TestCase 
{
	public static final String INPUT_FILE="./testfile.txt";

	private native int nativeMethod(int depth);
	private native int testCallIntMethodNative(byte[] arr, int base, int length);
	private native boolean testCallBooleanMethodNative(int val);
	private native Object testCallObjectMethodNative(Object obj);
	private native int testInputStreamCallbackNative(InputStream is, byte[] arr);

	@Before
	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			nativeMethod(0);
		}
   	}

 	public int callback(int depth, boolean f) 
 	{
    		if (depth < 5 && f) 
		{
      			//System.out.println("In Java, depth = " + depth + ", about to enter C");
      			return nativeMethod(depth + 1);
      			//System.out.println("In Java, depth = " + depth + ", back from C");
   		} 
		else
      			//System.out.println("In Java, depth = " + depth + ", limit exceeded");
      			return depth;
  	}

	public boolean booleanMethod(int val)
	{
		return (val==42);
	}

	public Object objectMethod(Object str)
	{
		return str;
	}

	@Test
	public void testCallbacks()
	{
		for(int i=0; i<RunParameters.JNI_FUNCTION_ITERATIONS; i++)
		{
			assertEquals(5,nativeMethod(0));
		}
	}

	//the callback function from the testCallIntMethodNative
	public int passByteArrIntInt(byte [] arr, int base, int length)
	{
		assertEquals(0xA, arr[0]);
		assertEquals(0,base);
		assertEquals(4, length);

		return 42;
	}

	public void testCallIntMethod()
	{

		byte [] arr = { 0xA, 0xB, 0xC, 0xD };
		assertEquals(42, testCallIntMethodNative(arr, 0, 4));
	}

	@Test
	public void testInputStreamCallback()
	{

		FileInputStream fis = null;
		try
		{
			fis = new FileInputStream(INPUT_FILE);
		}
		catch(Exception e)
		{

		}
	
		assertEquals(2, testInputStreamCallbackNative(fis, new byte[2]));
	}

	@Test
	public void testCallBooleanMethod()
	{

		assertTrue(testCallBooleanMethodNative(42));
	}

	@Test
	public void testObjectMethod()
	{

		assertTrue("Hello".equals(testCallObjectMethodNative("Hello")));
	}

	static 
	{
		System.loadLibrary("Callbacks");
	}
}
