package jni.jniwrap;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;

import fixture.RunParameters;

public class JNIWrapTest extends TestCase 
{
	private String test = "ABCD";
	private int [] test2 = {0, 1, 2, 3};
	private byte [] test3 = {0, 1, 2, 3};
	private char [] test4 = {'a', 'b', 'c', 'd'};
	private long [] test5 = {1, 2, 3, 4};
	private Object temp = new Object();

	private native int testGetVersion();
	private native int testJNIChecks(String test, int [] test2, byte [] test3, char [] test4, long [] test5, Object temp);

	public static void main(String [] args)
	{
		JNIWrapTest jwt = new JNIWrapTest();
		jwt.testTestGetVersion();
	}

	@Before
	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			testGetVersion();
		}
   	}

	@Test
	public void testTestGetVersion()
	{	
		for(int i=0; i<RunParameters.JNI_FUNCTION_ITERATIONS; i++)
		{
			assertEquals(0x00010006, testGetVersion());
		}
	}

	@Test
	public void testTestJNIChecks()
	{
		testJNIChecks(test, test2, test3, test4, test5, temp);
	}

	static 
	{
		System.loadLibrary("JNIWrap");
	}
}
