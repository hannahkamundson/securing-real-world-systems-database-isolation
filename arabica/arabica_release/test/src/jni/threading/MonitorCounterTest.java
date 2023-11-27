package jni.threading;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class MonitorCounterTest extends TestCase 
{
	public static final int NUM_THREADS=5;
	public static final int NUM_INCREMENTS=10;

	private MonitorCounter mc[] = new MonitorCounter[NUM_THREADS];

	public static void main(String [] args)
	{
		MonitorCounterTest mct = new MonitorCounterTest();

		mct.testMonitorCounter();	
	}

	private void initializeThreads()
	{
		//initialize threads
		for(int i=0; i<NUM_THREADS; i++)
		{
			mc[i] = new MonitorCounter(NUM_INCREMENTS);
		}
	}

	/*@Before
	public void warmup()
	{		
		initializeThreads();
	
		//call print 10 times to warm up
		//for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		for(int i=0; i<0; i++)
		{
			//start all threads
			for(int j=0; j<NUM_THREADS; j++)
			{
				mc[j].start();
			}

			//join all threads
			for(int j=0; j<NUM_THREADS; j++)
			{
				try
				{
					mc[j].join();
				}
				catch(InterruptedException e)
				{

				}
			}
			MonitorCounter.resetCounter();
			//reinitialize threads
			initializeThreads();
		}
   	}*/

	@Test
	public void testMonitorCounter()
	{	
		initializeThreads();

		//NOTE: we only run this test once because we can't re-initialize the threads
		//within this test
		//start all threads
		for(int j=0; j<NUM_THREADS; j++)
		{
				mc[j].start();
		}

		//join all threads
		for(int j=0; j<NUM_THREADS; j++)
		{
			try
			{
				mc[j].join();
			}
			catch(InterruptedException e)
			{
				//force a test failure!
				assertTrue(false);
			}
		}

		assertEquals(NUM_INCREMENTS*NUM_THREADS, MonitorCounter.getCounter());
		//System.out.println("MonitorCounter.getCounter: " + MonitorCounter.getCounter());
	}


}
