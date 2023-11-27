package jni.fieldaccess;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class FieldAccessTest extends TestCase 
{
	public int f_pub = 1013;
  	private int f_prv = 789;

	public long lng_pub = 1234;
	public boolean bln_pub = true;
	public A obj_pub = new A();
	public int setme=0;
	public byte [] bytereg = { 0xA, 0xB, 0xC };

	public short shortfld=0;

	public String strfld = "FTL";

  	public static A a = new A();

	private native int checkGetIntField();
	private native int checkGetLongField();
	private native int checkGetBooleanField();
	private native int checkGetObjectField();
	private native int checkSetIntField(int value);
	private native int checkSetBooleanField(boolean bln);
	private native int checkSetObjectField(Object obj);
	private native int checkSetShortField(short sht);

	private native int checkGetByteArrayRegion(byte [] arr, int len);
	private native int checkSetByteArrayRegion(byte [] arr, int len);

	@Before
	public void warmup()
	{
		//call print 10 times to warm up
		/*for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			accessFields(a);
		
		}*/
   	}

	/*@Test
	public void testFieldAccess()
	{	
		for(int i=0; i<RunParameters.JNI_FUNCTION_ITERATIONS; i++)
		{
			assertEquals(true, accessFields(a));
		}
	}*/

	@Test
	public void testGetIntField()
	{
		assertEquals(1802, checkGetIntField());
	}

	@Test
	public void testGetLongField()
	{
		assertEquals(1234, checkGetLongField());
	}

	@Test
	public void testGetBooleanField()
	{
		assertEquals(1, checkGetBooleanField());
	}

	@Test
	public void testGetObjectField()
	{
		assertEquals(134, checkGetObjectField());
	}

	@Test
	public void testGetByteArrayRegion()
	{
		assertEquals(33, checkGetByteArrayRegion(bytereg, 3));
	}

	@Test
	public void testSetObjectField()
	{
		assertEquals(1, checkSetObjectField("FTW"));
		assertTrue("FTW".equals(strfld));
		strfld="FTL";
	}

	@Test
	public void testSetIntField()
	{
		assertEquals(1, checkSetIntField(42));
		assertEquals(42, setme);
		setme=0;
	}

	@Test
	public void testSetShortField()
	{
		assertEquals(1, checkSetShortField((short)42));
		assertEquals(42, shortfld);
		shortfld=0;
	}

	@Test
	public void testSetBooleanField()
	{
		assertEquals(1, checkSetBooleanField(false));
		assertEquals(false, bln_pub);
		bln_pub=true;
	}

	@Test
	public void testSetByteArrayRegion()
	{
		assertEquals(1,checkSetByteArrayRegion(bytereg,3));

		assertEquals(3,bytereg[0]+bytereg[1]+bytereg[2]);
		bytereg[0]=0xA;
		bytereg[1]=0xB;
		bytereg[2]=0xC;
	}

	static 
	{
		System.loadLibrary("FieldAccess");
	}
}

class A {
  public int a_pub = 134;
  private int a_prv = 13434;
}
