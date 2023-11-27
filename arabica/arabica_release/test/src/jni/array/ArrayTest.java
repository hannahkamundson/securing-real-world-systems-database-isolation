package jni.array;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class ArrayTest extends TestCase 
{
	private native int sumArray(int arr[]);

	private native byte testGetReleaseByteArrayElementsNative(byte arr[]);
	private native byte[] testNewByteArrayNative();
	private native long[] testNewLongArrayNative();
	private native long testGetSetLongArrayRegionNative(long arr[]);
	private native long testGetReleaseLongArrayElementsNative(long arr[]);
	private native Object testGetObjectArrayElementNative(Object [] arr, int index);

	public static void main(String [] args)
	{
		ArrayTest at = new ArrayTest();

		//at.testGetSetLongArrayRegion();
	}
    /*
	@Test
	public void testSumArray()
	{	
		int arr[]=new int[10000];

		for (int i = 0; i < 10000; i++)
			arr[i] = i;
	
		for(int i=0; i<1; i++)
		{
			assertEquals(49995000, sumArray(arr));
		}
	}

	@Test
	public void testGetReleaseByteArrayElements()
	{
		byte arr[]={0x1, 0x2, 0x3};

		assertEquals(6, testGetReleaseByteArrayElementsNative(arr));
	}


	@Test
	public void testNewByteArray()
	{
		byte arr[] = testNewByteArrayNative();
		arr[0]=0xA;
		assertEquals(0xA, arr[0]);
	}

	@Test
	public void testNewLongArray()
	{
		long arr[] = testNewLongArrayNative();
		arr[0]=42;
		assertEquals(42, arr[0]);
	}

	@Test
	public void testGetSetLongArrayRegion()
	{
		long arr [] = new long[10];
		
		for(int i=0; i<10; i++)
		{
			arr[i]=i;
		}

		assertEquals(45, testGetSetLongArrayRegionNative(arr));

		assertEquals(42, arr[0]);
	}



	@Test
	public void testGetReleaseLongArrayElements()
	{
		long arr [] = new long[10];
		
		for(int i=0; i<10; i++)
		{
			arr[i]=i;
		}

		

		assertEquals(45, testGetReleaseLongArrayElementsNative(arr));

		assertEquals(42, arr[0]);
	}

*/

	@Test
	public void testGetObjectArrayElement()
	{
	    String [] arr = { "A", "B", "C" };
	    assertTrue("C".equals(testGetObjectArrayElementNative(arr, 2)));
	    //String[] arr = {"A"};
	    //assertTrue("A".equals(testGetObjectArrayElementNative(arr, 0)));
        }

	static 
	{
		System.loadLibrary("Array");
	}
}
