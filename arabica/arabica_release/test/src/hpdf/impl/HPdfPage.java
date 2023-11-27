package hpdf.impl;

public class HPdfPage
{
	int hpage;

	private native void HPDFSetWidth(int hpage, float value);
	private native void HPDFSetHeight(int hpage, float value);
	private native int HPDFGetHeight(int hpage);
	private native int HPDFGetWidth(int hpage);
	private native int HPDFCreateDestination(int hpage);
	private native void HPDFBeginText(int hpage);
	private native void HPDFSetFontAndSize(int hpage, int handle, float size);
	private native void HPDFMoveTextPos(int hpage, float x, float y);
	private native void HPDFShowText(int hpage, String text);
	private native void HPDFEndText(int hpage);
	private native void HPDFDrawImage(int hpage, int img, int x, int y, int width, int height);
	private native void HPDFSetRGBFill(int hpage, float r, float g, float b);
	//private native void show_description(int hpage, float x, float y, String text);
	private native void HPDFSetTextMatrix(int hpage, float val1, float val2, float val3, float val4, float xpos, float ypos);
	private native void HPDFTextOut(int hpage, float x, float y, String text);
	private native void HPDFSetCharSpace(int hpage, float space);
	private native void HPDFSetWordSpace(int hpage, float space);

	public HPdfPage(int hpage)
	{
		this.hpage = hpage;
	}	

	public void setWidth(float value)
	{
		HPDFSetWidth(hpage, value);
	}

	public void setHeight(float value)
	{
		HPDFSetHeight(hpage, value);
	}

	public float getHeight()
	{
		return HPDFGetHeight(hpage);
	}

	public float getWidth()
	{
		return HPDFGetWidth(hpage);
	}

	public HPdfDestination createDestination()
	{
		int hdest;
		
		hdest = HPDFCreateDestination(hpage);

		return new HPdfDestination(hdest);
	}

	public void beginText()
	{
		HPDFBeginText(hpage);
	}

	public void setFontAndSize(HPdfFont font, float size)
	{
		HPDFSetFontAndSize(hpage, font.getHandle(), size);
	}

	public void moveTextPos(float x, float y)
	{
		HPDFMoveTextPos(hpage, x, y);
	}

	public void showText(String text)
	{
		HPDFShowText(hpage, text);
	}

	public void endText()
	{
		HPDFEndText(hpage);
	}

	public void drawImage(HPdfImage himg, int x, int y, String text)
	{
		HPDFDrawImage(hpage, himg.getHandle(), x, y, himg.getWidth(), himg.getHeight());
	}

	public void setRGBFill(float r, float g, float b)
	{
		HPDFSetRGBFill(hpage, r, g, b);
	}

	/*public void showDescription(float x, float y, String text)
	{
		show_description(hpage, x, y, text);
	}*/

	public void setTextMatrix(float val1, float val2, float val3, float val4, float xpos, float ypos)
	{
		HPDFSetTextMatrix(hpage, val1, val2, val3, val4, xpos, ypos);
	}

	public void textOut(float x, float y, String text)
	{
		HPDFTextOut(hpage, x, y, text);
	}

	public void setCharSpace(float space)
	{
		HPDFSetCharSpace(hpage, space);
	}

	public void setWordSpace(float space)
	{
		HPDFSetWordSpace(hpage, space);
	}

	static
	{
		System.loadLibrary("pdf");
	}
}
