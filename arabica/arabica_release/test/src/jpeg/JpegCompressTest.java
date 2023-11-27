package jpeg;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;
import fixture.RobustaWatcher;

public class JpegCompressTest extends TestCase 
{
	public native void compress();

	@Before
	public void warmup()
	{

   	}

	@Test
	public void testCompress()
	{
	    int i;
	    for (i = 0; i < 10; i++)
		compress();
	}

	@After
	public void cooldown()
	{
		
	}

	static 
	{
		System.loadLibrary("jpegt");
	}
}
