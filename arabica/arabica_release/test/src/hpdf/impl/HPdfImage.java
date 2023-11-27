package hpdf.impl;

public class HPdfImage
{
	private int himg;
	private native int HPDFLoadJpegImageFromFile(int hpdf, String filename);
	private native int HPDFGetWidth(int img);
	private native int HPDFGetHeight(int img);

	public HPdfImage(HPdfDoc pdf, String filename)
	{
		himg = HPDFLoadJpegImageFromFile(pdf.getHandle(), filename);
	}

	public int getHandle()
	{
		return himg;
	}

	public int getWidth()
	{
		return HPDFGetWidth(himg);
	}

	public int getHeight()
	{
		return HPDFGetHeight(himg);
	}

	static
	{
		System.loadLibrary("pdf");
	}
}
