package fixture.watcher;
import fixture.RobustaWatcher;

import org.punit.watcher.AbstractWatcher;

public class SandWatcher extends AbstractWatcher
{
    public int sand;

    private static final int _SCALE = 1000;
    
    public void start() 
    {
	RobustaWatcher.resetSand();
    }

    public void stop() 
    {
	sand = RobustaWatcher.getSand();
    }
    
    public double value() {
    	return ((double)sand)/_SCALE;
    }

    public String punitName() {
        return "Sand:"; //$NON-NLS-1$
    }

    public String unit() {
        return "ms"; //$NON-NLS-1$ 
    }
	
}
