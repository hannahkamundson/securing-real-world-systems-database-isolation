// This gzip calls revised SUN JDK's zip implementation.
// This revised JDK's implementation uses JNI to call the ccured C zlib library (version 1.1.4).
package zlib2;

import java.io.*;

import zlib2.zip.GZIPOutputStream;

class GZIPOutputStreamLevel extends GZIPOutputStream {
  public GZIPOutputStreamLevel(OutputStream out, int level, int size) throws IOException {
    super(out, size);
    def.setLevel(level);
  }
}

class gzip {
  public static void main(String args[]) {
    byte[] buf;
    try {
      // Create the GZIP output stream
      String outFilename = args[0]+".gz";
      GZIPOutputStreamLevel out; 
        //new GZIPOutputStreamLevel(new FileOutputStream(outFilename), 6);
    
      // Open the input file
      String inFilename = args[0];
      FileInputStream in = new FileInputStream(inFilename);
    
      // Transfer bytes from the input file to the GZIP output stream
      //byte[] buf = new byte[1024];
      //16384

      int size = Integer.parseInt(args[1]);
      out = new GZIPOutputStreamLevel(new FileOutputStream(outFilename), 6, size);
      buf= new byte[size];
	
      int len;

      while ((len = in.read(buf)) > 0) {
	  //System.out.println("len: " + len + "\n");      
	  out.write(buf, 0, len);
      }


      in.close();
    
      // Complete the GZIP file
      out.finish();
      out.close();
    } catch (IOException e) {
    }
  }
}
