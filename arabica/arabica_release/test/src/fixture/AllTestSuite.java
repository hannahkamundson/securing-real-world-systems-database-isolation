package fixture;

import org.punit.type.TestSuite;

public class AllTestSuite implements TestSuite 
{
    public Class<?>[] testSuite() 
    {
        return new Class[] 
	    {
		//simple.blank.BlankTest.class,
		//simple.helloworld.HelloWorldTest.class
		//simple.parampass.ParamPassTest.class,
		//jni.jniwrap.JNIWrapTest.class,
		//jni.fieldaccess.StaticFieldAccessTest.class,
		//jni.array.ArrayTest.class,
		//jni.objectarray.ObjectArrayTest.class,
		//jni.callbacks.CallbacksTest.class,
		//jni.exceptions.CatchThrowTest.class,
		//jni.threading.MonitorCounterTest.class,
		//jni.fieldaccess.FieldAccessTest.class,
		//zlib2.GzipTest.class,
		math.StrictMathTest.class,
		//hpdf.TextPdfTest.class,
		//jpeg.JpegCompressTest.class
	    };
    }
}
