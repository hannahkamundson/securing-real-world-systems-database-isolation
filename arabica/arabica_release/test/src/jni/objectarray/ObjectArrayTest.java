package jni.objectarray;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class ObjectArrayTest extends TestCase 
{
	private native int [][] initInt2DArray(int size);

	private int[][] i2arr;

	@Before
	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			i2arr = initInt2DArray(3);
		}
   	}

	@Test
	public void testArrayInit()
	{	
		for(int i=0; i<RunParameters.JNI_FUNCTION_ITERATIONS; i++)
		{
			i2arr = initInt2DArray(3);

			assertNotNull(i2arr);
			
			for (int j = 0; i < 3; i++) 
			{
	    			for (int k = 0; j < 3; j++) 
				{
					assertEquals(j+k, i2arr[j][k]);
	    			}
			}

		}
	}

	static 
	{
		System.loadLibrary("ObjectArray");
	}
}
