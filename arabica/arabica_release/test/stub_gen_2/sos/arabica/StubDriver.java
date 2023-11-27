package sos.arabica;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;

public class StubDriver {

	private String mainClass;	// name of the main class (subfolder)
	private String lib;			// name of the native lib

	/**
	 * find the name of the main class before extracting
	 * the Jar package
	 * 
	 * @param file name
	 * @return nothing, but populate the field
	 */
	public void popMainClass (String jarName) throws IOException
	{
		System.out.println("Analyzing the jar file...");
		// get the name of the main class first because
		// we need to know how to run the program later
		JarFile jarfile = new JarFile(jarName);
		Manifest manifest = jarfile.getManifest();
		Attributes attrs = (Attributes) manifest.getMainAttributes();

		// find the Main-Class attribute
		for (Iterator<Object> it = attrs.keySet().iterator(); it.hasNext(); ) {
			// Get attribute name                                       
			Attributes.Name attrName = (Attributes.Name) it.next();
			if (attrName.toString().equals("Main-Class"))
				mainClass = attrs.getValue(attrName);
		}
		System.out.println("The main (entry) class of the Jar package: " + mainClass);
		System.out.println();
	}

	/**
	 * extract the Jar package to a subfolder
	 * named after its main class
	 * 
	 * @param file name
	 */
	public void extract (String jarName) throws IOException
	{
		System.out.println("Extracting the jar file...");
		System.out.println();
		JarFile jarfile = new JarFile(jarName);
		Enumeration<JarEntry> en = jarfile.entries();

		// new folder
		File sub = new File(mainClass);
		sub.mkdir();

		while (en.hasMoreElements()) {
			JarEntry file = (JarEntry) en.nextElement();
			File f = new File(sub + File.separator + file.getName());
			// if its a directory, create it
			if (file.isDirectory()) {
				f.mkdir();
				continue;
			}
			InputStream is = jarfile.getInputStream(file); // get the input stream
			FileOutputStream fos = new FileOutputStream(f);
			while (is.available() > 0) {  // write contents of 'is' to 'fos'
				fos.write(is.read());
			}
			fos.close();
			is.close();
		}
	}

	/**
	 * go over the subfolder
	 * and generate stub code for each class file
	 */
	public void gen () throws Exception
	{
		System.out.println("Generating stub code...");
		System.out.println();
		File dir = new File(mainClass);
		File[] files = dir.listFiles();
		int counter = 0;
		for (int i = 0; i < files.length; i++) {
			String fullname = files[i].getName();
			if (fullname.contains(".class")) {
				String classname = fullname.substring(0, fullname.indexOf('.'));
				StubGenerator sg = new StubGenerator(classname);
				File inter = sg.runJavap(dir);
				sg.processJavap(inter);
				if (sg.lib != null)
					lib = sg.lib;
				// generate the stub code based on the information above
				sg.genStub(mainClass, counter++);
				inter.deleteOnExit();
			}
		}
	}

	/**
	 * Use gcc to compile all stub code
	 * and generate the stub library
	 */
	public void compile () throws IOException
	{
		System.out.println("Compiling and linking...");
		// prepare the compilation command
		// TODO: we can provide a chance for users to provide
		// the path to their jdk
		String[] cmd = new String[7];
		cmd[0] = "gcc";
		cmd[1] = "-o";
		// cmd[2] is reserved for target .os file
		cmd[3] = "-c";
		cmd[4] = "-I/home/<username>/robustatrunk/include";
		cmd[5] = "-fPIC";
		// cmd[6] is reserved for the source file

		// compile all stub files
		File dir = new File(mainClass);
		File[] files = dir.listFiles();
		for (int i = 0; i < files.length; i++) {
			String name = files[i].getName();
			if (name.startsWith("stub-") && name.endsWith(".c")) {
				cmd[6] = name;
				cmd[2] = name.replaceFirst(".c", ".os");
				Process sub = Runtime.getRuntime().exec(cmd, null, dir);
				// a reader to read the output of the subprocess
				InputStreamReader isr = new InputStreamReader(sub.getInputStream());
				BufferedReader br = new BufferedReader(isr);
				String out;
				while ((out = br.readLine()) != null)
					System.out.println(out);
			}
		}
		
		// link objective files
		files = dir.listFiles();
		ArrayList<String> objs = new ArrayList<String>();
		for (int i = 0; i < files.length; i++) {
			String name = files[i].getName();
			if (name.startsWith("stub-") && name.endsWith(".os"))
				objs.add(name);
		}
		String[] linkcmd = new String[objs.size() + 5];
		linkcmd[0] = "gcc";
		linkcmd[1] = "-o";
		linkcmd[2] = "lib" + lib + ".so";
		linkcmd[3] = "/home/<username>/robustatrunk/librobusta/.libs/librobusta.so";
		linkcmd[4] = "-shared";
		for (int i = 0; i < objs.size(); i++)
			linkcmd[i + 5] = objs.get(i);
		
		Process sub = Runtime.getRuntime().exec(linkcmd, null, dir);
		// a reader to read the output of the subprocess
		InputStreamReader isr = new InputStreamReader(sub.getInputStream());
		BufferedReader br = new BufferedReader(isr);
		String out;
		while ((out = br.readLine()) != null)
			System.out.println(out);
		System.out.println();
	}
	
	/**
	 * Run the target program within Arabica
	 */
	public void runs () throws IOException
	{
		System.out.println("Begin the target program with native sandboxed...");
		System.out.println();
		// we still run in the subfolder
		File dir = new File(mainClass);
		String[] cmd = new String[4];
		cmd[0] = "java";
		cmd[1] = "-Djava.library.path=.";
		cmd[2] = "-agentpath:/home/<username>/robustatrunk/librobusta/.libs/librobusta.so";
		cmd[3] = mainClass;
		
		Process sub = Runtime.getRuntime().exec(cmd, null, dir);
		// a reader to read the output of the subprocess
		InputStreamReader isr = new InputStreamReader(sub.getInputStream());
		BufferedReader br = new BufferedReader(isr);
		String out;
		while ((out = br.readLine()) != null)
			System.out.println(out);
	}
	

	/**
	 * The main method
	 * 
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			// prompt the user
			System.out.println();
			System.out.println("=========================================");
			System.out.println("| Welcome to Arabica's Jar File Runner! |");
			System.out.println("|     by SoS Lab, Lehigh University     |");
			System.out.println("=========================================");
			System.out.println();
			System.out.println("Please input the name of the jar file: ");
			BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
			String jarName = in.readLine();

			if(!jarName.endsWith(".jar")) {
				System.out.println("File not in jar format.");
				System.exit(-1);
			}
			File file = new File(jarName);
			if (!file.exists()) {
				System.out.println("File does not exist.");
				System.exit(-1);
			}

			System.out.println();
			StubDriver sd = new StubDriver();
			sd.popMainClass(jarName);
			sd.extract(jarName);
			sd.gen();
			sd.compile();
			sd.runs();

		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
