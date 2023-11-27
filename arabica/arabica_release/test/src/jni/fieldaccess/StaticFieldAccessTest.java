package jni.fieldaccess;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class StaticFieldAccessTest extends TestCase 
{
	private static int si=100;

	private native void accessField();

	@Before
	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			accessField();
			si = 100;
		}
   	}

	@Test
	public void testAccessField()
	{	
		for(int i=0; i<RunParameters.JNI_FUNCTION_ITERATIONS; i++)
		{
			accessField();
			assertEquals(si, 200);
			si = 100;
		}
	}

	static 
	{
		System.loadLibrary("StaticFieldAccess");
	}
}
