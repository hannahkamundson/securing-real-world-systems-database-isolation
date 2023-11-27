package jni.exceptions;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class CatchThrowTest extends TestCase 
{
	private native int doit() throws IllegalArgumentException;

	private void callback() throws NullPointerException 
	{
		throw new NullPointerException("CatchThrow.callback");
    	}

	@Before 
	public void warmup()
	{
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			try
			{
				//doit();
			}
			catch(IllegalArgumentException e)
			{

			}
		}
   	}

	@Test (expected=IllegalArgumentException.class)
	public void testException()
	{	
		for(int i=0; i<RunParameters.JNI_FUNCTION_ITERATIONS; i++)
		{
			doit();
		}
	}

	public static void main(String [] args)
	{
		SoloRunner runner = new SoloRunner();
		runner.setConvention(new JUnitAnnotationConvention());
		runner.run(CatchThrowTest.class);
		runner.run(jni.callbacks.CallbacksTest.class);
	}

	static 
	{
		System.loadLibrary("CatchThrow");
	}
}
