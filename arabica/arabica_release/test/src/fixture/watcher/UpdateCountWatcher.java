package fixture.watcher;
import fixture.RobustaWatcher;

import org.punit.watcher.AbstractWatcher;

public class UpdateCountWatcher extends AbstractWatcher
{
    public int update_count;

    public void start() 
    {
	RobustaWatcher.resetUpdateCount();
    }

    public void stop() 
    {
	update_count = RobustaWatcher.getUpdateCount();
    }
    
    public double value() 
    {
    	return update_count;
    }

    public String punitName() {
        return ":"; //$NON-NLS-1$
    }

    public String unit() {
        return " updates"; //$NON-NLS-1$ 
    }
	
}
