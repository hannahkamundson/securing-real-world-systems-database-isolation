class FieldAccess {
  public int f_pub = 1013;
  private int f_prv = 789;

  public long lng_pub = 0xAFFFFFFFFL;
  public boolean bln_pub = true;

  
  public static A a = new A();

  private native void accessFields(A a);
  public static void main(String args[]) {
    FieldAccess c = new FieldAccess();
    c.accessFields(a);
  }
  static {
    System.loadLibrary("FieldAccess");
  }
}

class A {
  public int a_pub = 134;
  private int a_prv = 13434;
}
