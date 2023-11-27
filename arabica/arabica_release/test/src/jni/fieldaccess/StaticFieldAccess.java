class StaticFieldAccess {
    private static int si;
 
    private native void accessField(AnotherStaticFieldAccess a);
    public static void main(String args[]) {
	AnotherStaticFieldAccess a = new AnotherStaticFieldAccess();
	StaticFieldAccess c = new StaticFieldAccess();
	StaticFieldAccess.si = 100;
	c.accessField(a);
	System.out.println("In Java:");
	System.out.println("Should be 200: " + si);
    }
    static {
	System.loadLibrary("StaticFieldAccess");
    }
}

class AnotherStaticFieldAccess 
{
	private static int asi;

	public AnotherStaticFieldAccess()
	{
		asi = 42;
	}
}
