package file;

import java.io.File;
import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class FileAccessTest extends TestCase 
{
	public static final String CUR_DIR_FILE_READ = "/home/<username>/robustatrunk/test/punit/testfile.txt";
	public static final String CUR_DIR_FILE_WRITE = "/home/<username>/robustatrunk/test/punit/testfile-write.txt";
	public static final String OTHER_DIR_FILE = "/home/<username>/robustatrunk/README";

	private native int testFileRead(String filename);
	private native int testFileWrite(String filename);
	private native int testFileReadWrite(String filename);

	private native int testFileReadF(String filename);
	private native int testFileWriteF(String filename);


	public static void main(String[] args)
	{
		FileAccessTest fat = new FileAccessTest();
		//fat.testReadCurrentDirectory();
		//fat.testWriteOther();
	}

	/*@Before
	public void warmup()
	{
		//call print 10 times to warm up
		for(int i=0; i<RunParameters.WARMUP_ITERATIONS; i++)
		{
			testGetVersion();
		}
   	}*/

	/*public void testReadCurrentDirectory()
	{
		assertEquals(42, testFileRead(CUR_DIR_FILE_READ));
	}*/

	@Test
	public void testReadCurrentDirectoryF()
	{
		assertEquals(42, testFileReadF(CUR_DIR_FILE_READ));
	}

	@Test
	public void testWriteCurrentDirectoryF()
	{		
		assertEquals(2, testFileWriteF(CUR_DIR_FILE_WRITE));
		assertEquals(42, testFileReadF(CUR_DIR_FILE_WRITE));

		File f = new File(CUR_DIR_FILE_WRITE);
		f.delete();
	}

	/*@Test
 	public void testReadWriteCurrentDirectory()
	{
		assertEquals(44, testFileReadWrite(CUR_DIR_FILE_WRITE));
	}*/

	@Test
	public void testReadOtherF()
	{
		assertEquals(42, testFileReadF(OTHER_DIR_FILE));
	}

	@Test
	public void testWriteOtherF()
	{
		assertEquals(2, testFileWriteF(OTHER_DIR_FILE));
	}

	/*@Test
	public void testReadWriteOther()
	{
		assertEquals(0, testFileReadWrite(OTHER_DIR_FILE));
	}*/

	static 
	{
		System.loadLibrary("FileAccess");
	}
}
