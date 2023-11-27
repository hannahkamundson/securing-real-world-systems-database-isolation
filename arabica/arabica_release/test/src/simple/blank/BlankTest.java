package simple.blank;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.runner.SoloRunner;
import fixture.RunParameters;

public class BlankTest extends TestCase 
{
    public native void blank();

    @Test
    public void testBlankNativeMethod()
    {
	blank();
    }

    static 
    {
	System.loadLibrary("Blank");
    }
}
