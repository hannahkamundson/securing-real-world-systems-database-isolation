package hpdf.impl;

public class HPdfDoc
{
	//constants
	public static final int HPDF_COMP_ALL = 15;

	public static final boolean DEBUG = true;

	private int hpdf;

	//native methods

	private native int HPDFNew();	

	private native int HPDFSetCompressionMode(int hpdf, int mode);

	private native int HPDFGetFont(int hpdf, String font_name, String encoding_name);

	private native int HPDFAddPage(int hpdf);

	private native void HPDFSetOpenAction(int hpdf, int handle);

	private native void HPDFSaveToFile(int hpdf, String file);

	public HPdfDoc() 
	{
		hpdf = HPDFNew();
	}

	public int getHandle()
	{
		return hpdf;
	}

	public void setCompressionMode(int mode)
	{
		HPDFSetCompressionMode(hpdf, mode);
	}

	public HPdfFont getFont(String font_name, String encoding_name)
	{
		int hfont;

		hfont = HPDFGetFont(hpdf, font_name, encoding_name);
		return new HPdfFont(hfont);
	}

	public HPdfPage addPage()
	{
		int hpage;
		
		hpage = HPDFAddPage(hpdf);

		return new HPdfPage(hpage);
	}

	public void setOpenAction(HPdfDestination dst)
	{
		if(dst == null)
		{
			//error!
		}
		else
		{
			HPDFSetOpenAction(hpdf, dst.getHandle());	
		}
	}

	public void saveToFile(String file)
	{
		HPDFSaveToFile(hpdf, file);
	}

	static
	{
		System.loadLibrary("pdf");
	}

}
