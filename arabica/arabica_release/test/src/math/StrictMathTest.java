package math;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class StrictMathTest extends TestCase 
{
	@Before
	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			double d = StrictMath.pow(2, 4.5);
			d = StrictMath.log(100);
			d = StrictMath.sin(java.lang.Math.PI/2);
			d = StrictMath.cbrt(9);
		}
   	}

    // public volatile int j;

	@Test
	public void testMath()
	{	
		for(int i=0; i<20000; i++)
		{
		    //mes310
		    //for (j = 0; j < 1000; j++) ;
		    assertEquals(1.0, StrictMath.cos(0.0), 0);
		    assertEquals(1.0,StrictMath.sin(java.lang.Math.PI/2), 0);
		    assertEquals(java.lang.Math.PI/2, StrictMath.asin(1.0), 0);
		    assertEquals(0.0, StrictMath.acos(1), 0);
		    assertEquals(0.0, StrictMath.atan(0.0), 0);
		    assertEquals(1.0, StrictMath.exp(0), 0);
		    assertEquals(4.605170185988092,StrictMath.log(100.0), 0);
		    assertEquals(1.0, StrictMath.log10(10.0), 0);
		    assertEquals(2.0, StrictMath.sqrt(4.0), 0);
		    assertEquals(2.080083823051904,StrictMath.cbrt(9.0), 0);
		    assertEquals(3.0, StrictMath.ceil(2.489), 0);
		    assertEquals(2.0, StrictMath.floor(2.489), 0);
		    assertEquals(java.lang.Math.PI/2,StrictMath.atan2(1.0, 0.0), 0);
		    assertEquals(22.62741699796952,StrictMath.pow(2.0, 4.5), 0);
		}
		//StrictMath.IEEEremainder(jdouble dividend,jdouble divisor)
		//StrictMath.cosh(jdouble d)
		//StrictMath.sinh(jdouble d)
		//StrictMath.tanh(jdouble d)
		//StrictMath.hypot(jdouble x, jdouble y)
		//StrictMath.log1p(jdouble d)
		//StrictMath.expm1(jdouble d)
	}

	public void nonNative()
	{
		System.out.println("non-native function");
	}

	/*@Test
	public void testOneLoop()
	{
		for(int i=0; i<50000; i++)
		{
			assertEquals(22.62741699796952,StrictMath.pow(2.0, 4.5));
			assertEquals(1.0, StrictMath.cos(0.0));
		}
	}*/

	/*@Test
	public void testOne()
	{
		assertEquals(1.0, StrictMath.cos(0.0));
	}*/
}


/*public class StrictMathTest {
    public static void main(String[] args) {
	double d = StrictMath.pow(2, 4.5);
	System.out.println("2^4.5=" + d);
	d = StrictMath.log(100);
	System.out.println("log(100)=" + d);
	d = StrictMath.sin(java.lang.Math.PI/2);
	System.out.println("sin(90 degress)=" + d);
	d = StrictMath.cbrt(9);
	System.out.println("Cubic root (9)=" + d);
    }
}*/
