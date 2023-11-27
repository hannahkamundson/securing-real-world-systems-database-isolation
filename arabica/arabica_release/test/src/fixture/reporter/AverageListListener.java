package fixture.reporter;

import org.punit.events.VanillaEventListener;
import org.punit.watcher.TimeWatcher;
import org.punit.watcher.Watcher;

import java.util.ListIterator;
import java.lang.reflect.Method;
import java.util.List;
import java.util.LinkedList;

public class AverageListListener extends VanillaEventListener
{
	private TimeWatcher timewatcher;
	
	private LinkedList<MethodAverage> averagelist = new LinkedList<MethodAverage>();

	public AverageListListener()
	{
		timewatcher = new TimeWatcher();
	}

	public void onMethodStart(Method method, Object testInstance, Object[] params) 
	{
		timewatcher.start();	
	}

	public void onMethodEnd(Method method, Object testInstance, Object[] params, Throwable t, List<Watcher> Watchers) 
	{
		timewatcher.stop();

		averagelist.add(new MethodAverage(method.getName(), timewatcher.value()));
	}

	public String result()
	{
		MethodAverage mavg;
		StringBuffer result = new StringBuffer();

		ListIterator<MethodAverage> iter = averagelist.listIterator(0);

		while(iter.hasNext())
		{
			mavg = iter.next();
			result.append(mavg.toString() + "\n");
		}

		return result.toString();
	}

	private class MethodAverage implements Comparable
	{
		private String name;
		private double val;

		public MethodAverage(String name, double val)
		{
			this.name=name;
			this.val=val;
		}

		public int compareTo(Object o)
		{
			MethodAverage ma = (MethodAverage) o;

			return this.name.compareTo(ma.name);
		}		

		@Override
		public String toString()
		{
			return "[" + name + "]" + " - " + val;
		}
	}
}
