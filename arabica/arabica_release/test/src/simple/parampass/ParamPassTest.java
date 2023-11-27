package simple.parampass;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;

import fixture.RobustaWatcher;

public class ParamPassTest extends TestCase 
{
	private int test;

	private native int pass1(int a);
	private native int pass2(int a, int b);
	private native int pass3(int a, int b, int c);
	private native double passprims(byte b, char c, double d, float f, int i, long l);
	private native String passobjtypes(Object o, String s);
	private native boolean returnboolean();
	private native byte returnbyte();
	private native char returnchar();
	private native short returnshort();
	private native int returnint();
	private native long returnlong();
	private native float returnfloat();
	private native double returndouble();
	private native Object returnjobject();

	public static final int NUM_COMPILE_ITERATIONS=1;

	//static native method checks
	private static native int passstatic(int a);

	@Before
	public void warmup()
	{	
		for(int i=0; i<10; i++)
		{
			pass1(42);
		}
   	}

	@Test
	public void testPass1()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(42, pass1(42));
		}
	}

	@Test
	public void testPass2()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(84, pass2(42, 42));
		}
	}

	@Test
	public void testPass3()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(126, pass3(42,42,42));
		}
	}

	@Test
	public void testPassPrims()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
		    assertEquals(4326.283180118408, passprims((byte)4, 'J', (double) 3.14159, (float) 3.14159f, 42, 4200), 0);
		}
	}

	@Test
	public void testPassObjTypes()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertTrue("Hello".equals(passobjtypes(this, "Hello")));
		}
	}

	@Test
	public void testPassStatic()
	{
		assertEquals(42, ParamPassTest.passstatic(42));
	}

	@Test
	public void testReturnBoolean()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(returnboolean(), true);
		}
	}

	@Test
	public void testReturnByte()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(returnbyte(), (byte) 0xFF);
		}
	}

	@Test
	public void testReturnChar()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(returnchar(), 'J');
		}
	}

	@Test
	public void testReturnShort()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(returnshort(), (short) 0xFFFF);
		}
	}

	@Test
	public void testReturnInt()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
			assertEquals(returnint(), (int) 0xFFFFFFFF);
		}
	}

	@Test
	public void testReturnLong()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
		    assertEquals((0xAFFFFFFFFL), returnlong());
		}
	}

    /*
	@Test
	public void testReturnFloat()
	{
		//does not work for some reason
		assertEquals(returnfloat(), 3.14159);
	}
    */
	@Test
	public void testReturnDouble()
	{
		for(int i=0; i<NUM_COMPILE_ITERATIONS; i++)
		{
		    assertEquals(3.14159, returndouble(), 0);
		}
	}

	static 
	{
		System.loadLibrary("ParamPass");
	}
}
