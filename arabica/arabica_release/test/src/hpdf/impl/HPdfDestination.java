package hpdf.impl;

public class HPdfDestination
{
	int hdest;

	private native void HPDFSetXYZ(int hdest, float left, float top, float zoom);

	public HPdfDestination(int hdest)
	{
		this.hdest = hdest;
	}

	public void setXYZ(float left, float top, float zoom)
	{
		HPDFSetXYZ(hdest, left, top, zoom);
	}

	public int getHandle()
	{
		return hdest;
	}

	static
	{
		System.loadLibrary("pdf");
	}
}
