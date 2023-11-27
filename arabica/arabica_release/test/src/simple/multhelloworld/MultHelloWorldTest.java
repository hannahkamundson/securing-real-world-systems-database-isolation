package simple.multhelloworld;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;
import fixture.RobustaWatcher;

public class MultHelloWorldTest extends TestCase 
{
	public native void print1();

	public native void print2();

	private int test;

 	@Before

	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			print1();
			print2();
		}
		System.out.println("Initializing sand...");
		RobustaWatcher.resetSand();
   	}

	@Test
	public void testMultHelloWorld()
	{
		print1();
		print2();
	}

	@After
	public void cooldown()
	{
		System.out.println("Sand:" + RobustaWatcher.getSand());
	}

	static 
	{
		System.loadLibrary("MultHelloWorld1");
		System.loadLibrary("MultHelloWorld2");
	}
}
