class ParamPass 
{
	private int test;

	private native void pass1(int a);
	private native void pass2(int a, int b);
	private native void pass3(int a, int b, int c);
	private native void passprims(byte b, char c, double d, float f, int i, long l);
	private native void passobjtypes(Object o, String s);
	private native boolean returnboolean();
	private native byte returnbyte();
	private native char returnchar();
	private native short returnshort();
	private native int returnint();
	private native long returnlong();
	private native float returnfloat();
	private native double returndouble();
	private native Object returnjobject();

	//static native method checks
	private static native void passstatic(int a);


	public ParamPass(int test)
	{
		this.test=test;
	}

	public static void main(String[] args) 
	{
		System.out.println("[Java] In the Java file...");
		ParamPass hw = new ParamPass(42);
		ParamPass hw2;

		hw.pass1(42);
		hw.pass2(42, 42);
		hw.pass3(42, 42, 42);

		//check primitive parameter passing
		hw.passprims((byte)4, 'J', (double) 3.14159, (float) 3.14159, 42, 4200);

		hw.passobjtypes(hw, "Hello");

		ParamPass.passstatic(42);		

		//do return type checks...
		check(hw.returnboolean() == true, "Return boolean check failed");
		check((byte) hw.returnbyte() == ((byte) 0xFF), "Return byte check failed");
		check(hw.returnchar() == 'J', "Return char check failed");
		check((short) hw.returnshort() == ((short) 0xFFFF), "Return short check failed");
		check(hw.returnint() == 0xFFFFFFFF, "Return int check failed");
		long test = hw.returnlong();
		check(test == (0xAFFFFFFFFL), "The value returned should have been : (0xAFFFFFFFFL): " + Long.toHexString(test));
		check(hw.returnfloat() == 3.14159, "Return float check failed");
		check(hw.returndouble() == 3.14159, "Return double check failed");

		//do object return type checks
		hw2 = (ParamPass) hw.returnjobject();
		System.out.println("[Java] Checking java object return. Should be 42: " + hw2.test);
	}

	public static void check(boolean test, String errmsg)
	{
		if(!test)
		{
			System.out.println("[Java]" + errmsg);
		}
	}
	static 
	{
		System.loadLibrary("ParamPass");
	}
}
