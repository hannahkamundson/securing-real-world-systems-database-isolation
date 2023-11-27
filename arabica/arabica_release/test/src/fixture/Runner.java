package fixture;

import org.punit.runner.SoloRunner;
import org.punit.convention.JUnitAnnotationConvention;
import org.punit.reporter.stream.file.FileLogger;

import java.text.DateFormat;
import java.text.SimpleDateFormat;

import fixture.watcher.SandWatcher;
import fixture.watcher.UpdateCountWatcher;

import java.io.PrintWriter;

public class Runner {
    public static void main(String[] args) {
        /*
         * Users can use either <code>SoloRunner</code> or <code>ConcurrentRunner</code>
         * to run any test classes and test suites. The reporters can be
         * configured by <code>runner.addEventListener(EventListener e)</code>,
         * including Console(default)/File/Image/PDF.
         * 
         * The result can be found in ./result/ folder.
         */
	//System.out.println("Running the benchmark with " + RunParameters.WARMUP_ITERATIONS + " warmup iterations and " + RunParameters.JNI_FUNCTION_ITERATIONS + " test iterations.");

	SoloRunner runner = new SoloRunner();

	//determine the output file based on the date, and whether it was sandboxed or not
	DateFormat dateFormat = new SimpleDateFormat("yyyy_MM_dd-HH_mm_ss_");	
	java.util.Date date = new java.util.Date();
	runner.addEventListener(new FileLogger("./result/" + dateFormat.format(date) + args[0] + ".txt"));
		
        // runner = new ConcurrentRunner();
        //runner.addEventListener(new FileLogger());
        
	//runner.methodRunner().addWatcher(new SandWatcher());
	//runner.methodRunner().addWatcher(new UpdateCountWatcher());

	runner.setConvention(new JUnitAnnotationConvention());
        runner.run(AllTestSuite.class);
    }
}
