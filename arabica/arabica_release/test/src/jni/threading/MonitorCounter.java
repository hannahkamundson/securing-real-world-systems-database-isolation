package jni.threading;

public class MonitorCounter extends Thread
{
	private static int counter;
	private final int num_iter;

	private native void counterloop(int iters);

	public MonitorCounter(int num_iter)
	{
		this.num_iter=num_iter;
	}

	public void run()
	{
		counterloop(num_iter);
	}

	public static void resetCounter()
	{
		counter=0;
	}

	public static int getCounter()
	{
		return counter;
	}

	static 
	{
		System.loadLibrary("MonitorCounter");
	}
}
