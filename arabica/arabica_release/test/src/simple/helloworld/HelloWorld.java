class HelloWorld 
{
	//public so we can find it from the benchmark driver
	public native void print();

	public static void main(String[] args) 
	{
		System.out.println("[Java] In the Java file...");
		HelloWorld hw = new HelloWorld();		

		hw.print();
	}

	static 
	{
		System.loadLibrary("HelloWorld");
	}
}
