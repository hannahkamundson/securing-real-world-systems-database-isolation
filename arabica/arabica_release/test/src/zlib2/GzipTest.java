package zlib2;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import org.punit.type.Parameterized;
import org.punit.type.Parameter;
import fixture.RunParameters;
import fixture.RobustaWatcher;

public class GzipTest implements Parameterized
//extends TestCase implements Parameterized
{
	private String [] args = new String[2];
		
        @Before
	public void warmup()
	{	
		args[0]="images/med_image.jpg";
		args[1]="16384";
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			gzip.main(args);
		}	
		RobustaWatcher.resetSand();
   	}
    /*
	@Test
	public void smallFile(ZipBufferSizeParameter param)
	{
		doTest("build.xml", param);
	}

	@Test
	public void mediumFile(ZipBufferSizeParameter param)
	{
		doTest("images/med_image.jpg", param);
	}
    */
        @Test
	public void largeFile(ZipBufferSizeParameter param)
	{
		doTest("images/very_larg.jpg", param);
	}

	/*@Test
	public void enormousFile(ZipBufferSizeParameter param)
	{
		doTest("iTunes64Setup.exe", param);
	}*/

	public void doTest(String file, ZipBufferSizeParameter param)
	{
		args[0]=file;
		args[1]=param.toString();
		gzip.main(args);
	}

	public void setUpAfterWatchers(Parameter param) throws Exception {}

	public void setUpBeforeWatchers(Parameter param) throws Exception {}

	public void tearDownAfterWatchers(Parameter param) throws Exception {}

	public void tearDownBeforeWatchers(Parameter param) throws Exception {}

	public Parameter[] parameters() 
	{
	    return new Parameter[] { new ZipBufferSizeParameter(512), new ZipBufferSizeParameter(1024), new ZipBufferSizeParameter(2048), new ZipBufferSizeParameter(4096), new ZipBufferSizeParameter(8192), new ZipBufferSizeParameter(16384), new ZipBufferSizeParameter(65536)};

	    //return new Parameter[] { new ZipBufferSizeParameter(2048) };

	}
    /*
	@Test
	public void testLargeZip512()
	{
		args[0]="med_image.jpg";
		args[1]="512";
		gzip.main(args);
	}

	@Test
	public void testEnormousZip8192()
	{
		args[0]="iTunes64Setup.exe";
		args[1]="8192";
		gzip.main(args);
	}

	@Test
	public void testVeryLargeZip1024()
	{
		args[0]="very_larg.jpg";
		args[1]="1024";
		gzip.main(args);
	}

	@Test
	public void testLargeZip1024()
	{
		args[0]="images/med_image.jpg";
		args[1]="16384";
		gzip.main(args);
	}

	@Test
	public void testLargeZip2048()
	{
		args[0]="med_image.jpg";
		args[1]="2048";
		gzip.main(args);
	}

	@Test
	public void testLargeZip4096()
	{
		args[0]="med_image.jpg";
		args[1]="4096";
		gzip.main(args);
	}


	@Test
	public void testLargeZip8192()
	{
		args[0]="med_image.jpg";
		args[1]="8192";
		gzip.main(args);
	}

	@Test
	public void testLargeZip16384()
	{
		args[0]="med_image.jpg";
		args[1]="16384";
		gzip.main(args);
	}

	@Test
	public void testLargeZip65536()
	{
		args[0]="med_image.jpg";
		args[1]="65536";
		gzip.main(args);
	}

	@Test
	public void testLargeZip131072()
	{
		args[0]="med_image.jpg";
		args[1]="131072";
		gzip.main(args);
	}

	@Test
	public void testLargeZip262144()
	{
		args[0]="med_image.jpg";
		args[1]="262144";
		gzip.main(args);
	}

    /*@Test
	public void testSmallZip1024()
	{
		args[0]="build.xml";
		args[1]="small";
		gzip.main(args);
	}

	@Test
	public void testSmallZip8192()
	{
		args[0]="build.xml";
		args[1]="medium";
		gzip.main(args);
	}

	@Test
	public void testSmallZip16384()
	{
		args[0]="build.xml";
		args[1]="large";
		gzip.main(args);
	}
    *//*
	@Test
	public void testMediumZip512()
	{
		args[0]="los.jpg";
		args[1]="512";
		gzip.main(args);
	}

	@Test
	public void testMediumZip1024()
	{
		args[0]="los.jpg";
		args[1]="1024";
		gzip.main(args);
	}

	@Test
	public void testMediumZip2048()
	{
		args[0]="los.jpg";
		args[1]="2048";
		gzip.main(args);
	}

	@Test
	public void testMediumZip4096()
	{
		args[0]="los.jpg";
		args[1]="4096";
		gzip.main(args);
	}

	@Test
	public void testMediumZip8192()
	{
		args[0]="los.jpg";
		args[1]="8192";
		gzip.main(args);
	}

	@Test
	public void testMediumZip16384()
	{
		args[0]="los.jpg";
		args[1]="16384";
		gzip.main(args);
	}

	@Test
	public void testMediumZip65536()
	{
		args[0]="los.jpg";
		args[1]="65536";
		gzip.main(args);
	}
*/

}

class ZipBufferSizeParameter implements Parameter 
{
	private int _count;
	
	ZipBufferSizeParameter(int count) 
	{
		_count = count;
	}
	
	public int getZipBufferSize() 
	{
		return _count;
	}

	public String toString()
	{
		return ""+_count;
	}
}
