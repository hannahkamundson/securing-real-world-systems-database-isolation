package hpdf.impl;

public class HPdfFont
{
	private int hfont;

	public HPdfFont(int hfont)
	{
		this.hfont=hfont;
		//System.out.println("hfont: " + hfont);
	}

	public int getHandle()
	{
		return hfont;
	}
}
