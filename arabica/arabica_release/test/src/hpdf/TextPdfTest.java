package hpdf;

import hpdf.impl.HPdfDoc;
import hpdf.impl.HPdfFont;
import hpdf.impl.HPdfPage;
import hpdf.impl.HPdfDestination;
import hpdf.impl.HPdfImage;

import junit.framework.TestCase;
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;
import org.junit.After;

public class TextPdfTest 
{
	//private native int hello();

	@Before
	public void warmup()
	{
		
	}

	@Test
	public void CreateTextPdf()
	{
		HPdfDoc pdf = new HPdfDoc();

		pdf.setCompressionMode(HPdfDoc.HPDF_COMP_ALL);

		HPdfFont font = pdf.getFont("Helvetica", null);

		HPdfPage page = null;
		
		//HPdfImage img = new HPdfImage(pdf, "./rgb.jpg");
		//HPdfImage img2 = new HPdfImage(pdf, "./gray.jpg");

		for(int i=0; i<1000; i++)
		{
			page = pdf.addPage();
			page.setHeight((float)500.00);
			page.setWidth((float)650.00);
		
			HPdfDestination dst = page.createDestination();

			dst.setXYZ(0, page.getHeight(), 1);
			pdf.setOpenAction(dst);

			page.beginText();
			page.setFontAndSize(font, 20);
			page.moveTextPos(220, page.getHeight() - 70);
			page.showText("JpegDemo");
			page.endText();

			page.setFontAndSize(font, 12);
		}
		//page.drawImage(img, 70, (int) page.getHeight() - 410, "24bit color image");
		//page.drawImage(img2, 340, (int) page.getHeight() - 410, "8bit grayscale image");

		//pdf.saveToFile("./result/rgb.pdf");
	}

	/*static
	{
		System.loadLibrary("pdf");
	}*/
}
