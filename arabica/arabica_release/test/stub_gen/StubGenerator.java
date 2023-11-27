import java.io.PrintWriter;
import java.io.FileWriter;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * This is the stub library generator for Arabica
 * 
 * @author Mengtao Sun
 */
public class StubGenerator {
	
	private final PrintWriter pw;
	private final BufferedReader br;
	private final String lib;
	private final boolean em;	// em stands for "extern mark"
	private final boolean cm;	// cm stands for "common mark"
	private final String dst;
	private final ArrayList<String> headerList;
	private final ArrayList<String> methodList;
	private final ArrayList<String> returnTypeList;
	private final ArrayList<ArrayList<String>> argList;
	private final ArrayList<Integer> floatList;	
	
	// Access flag for the native method
	private int access_flag;

	private static void usage(String reason) {
		if (reason != null) {
			System.out.println(reason);
		}
		System.out.println(
				"Usage: java " + StubGenerator.class.getName() + " <config file> [Options]\n"
				+ "where options include: \n"
				+ " -help\t\t\tshow help information");
		System.exit(-1);
	}
	
	/**
	 * Constructor for the class
	 * 
	 * @param args: the output file as a PrintWriter
	 */
	private StubGenerator(PrintWriter a, BufferedReader b, boolean c, boolean d, String e, ArrayList<String> f, String g) {
		this.pw = a;
		this.br = b;
		this.em = c;
		this.cm = d;
		this.lib = e;
		this.headerList = f;
		this.dst = g;
		this.methodList = new ArrayList<String>();
		this.returnTypeList = new ArrayList<String>();
		this.argList = new ArrayList<ArrayList<String>>();
		this.floatList = new ArrayList<Integer>();
		
		// By default, the method is non-static
		this.access_flag = 33026;
	}
	
	/**
	 * Main method
	 * 
	 * @param args: the command line arguments
	 */
	public static void main(String[] args) {
		String stubName = null;
		String srcName = null;
		String libName = null;
		ArrayList<String> headerName = new ArrayList<String>();
		String configName = null;
		int i = 0;
		if (args.length != 1 || args[i].equals("-help")) {
			usage(null);
		} else {
			// There exists only one arg, which should be the config file name
			configName = args[0];
			
			// Begin to interpret the config file
			try {
				BufferedReader br = new BufferedReader(new FileReader(configName));
				String s, clearLine;
				int counter = 1;
				boolean externMark = false, commonMark = false;
				while ((s = br.readLine()) != null) {
					// Eliminate all spaces
					clearLine = s.replaceAll("\\s", "");
					
					// Comment line starts with #
					if (clearLine.startsWith("#") || clearLine.length() == 0)
						continue;
					else if (clearLine.startsWith("src=")) {
						srcName = clearLine.substring(4);
					} else if (clearLine.startsWith("dst=")) {
						stubName = clearLine.substring(4);
					} else if (clearLine.startsWith("lib=")) {
						libName = clearLine.substring(4);
					} else if (clearLine.startsWith("header=")) {
						headerName = parseHeaderFiles(clearLine.substring(7));
					} else if (clearLine.startsWith("extern=")) {
						if (clearLine.charAt(7) == '1')
							externMark = true;
					} else if (clearLine.startsWith("common=")) {
						if (clearLine.charAt(7) == '1')
							commonMark = true;
					}
					else {
						System.out.println("Illegal configuration file.");
						System.out.println("Error in line " + counter + ".");
						System.exit(-1);
					}
					counter++;
				}
				
				if (srcName == null) {
					System.out.println("You must specify the path to the source file in the configuration file.");
					System.exit(-1);
				}
				if (stubName == null) {
					System.out.println("You must specify the path to the generated file in the configuration file.");
					System.exit(-1);
				}
				if (libName == null) {
					System.out.println("You must specify the path to the true sandboxed native library in the configuration file.");
					System.exit(-1);
				}
				
				// Generate the stub code here
				BufferedReader tbr = new BufferedReader(new FileReader(srcName));
				BufferedWriter bw = new BufferedWriter(new FileWriter(stubName));
				PrintWriter tpw = new PrintWriter(bw);
				StubGenerator sg = new StubGenerator(tpw, tbr, externMark, commonMark, libName, headerName, stubName);
				
				sg.genStub();
				boolean state = tpw.checkError();
				if (state == true) throw new IOException();
				tpw.close();

				System.out.println("DONE.");
			} catch (FileNotFoundException fnfe) {
				//System.out.println("Can not find the configuration file!");
				fnfe.printStackTrace();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
	}
	
	/**
	 * Parse the header file list in the configuration file
	 * 
	 * @param The string contains the header files
	 * @return An array list with all header file names
	 */
	private static ArrayList<String> parseHeaderFiles(String flist) {
		ArrayList<String> headers = new ArrayList<String>();
		String s;
		int startIndex = 1;
		int endIndex;
		
		if (flist.startsWith("[")) {
			while ((endIndex = flist.indexOf(',', startIndex)) != -1) {
				s = flist.substring(startIndex, endIndex);
				headers.add(s);
				startIndex = endIndex + 1;
			}
			// There is still one header file left
			endIndex = flist.indexOf(']', startIndex);
			if (endIndex == -1) {
				System.out.println("The header files must be in []!");
				System.exit(-1);
			}
			s = flist.substring(startIndex, endIndex);
			headers.add(s);
		} else {
			System.out.println("The header files must be in []!");
			System.exit(-1);
		}
		return headers;
	}
	
	/**
	 * Generate the stub library source code
	 */
	private void genStub() throws IOException{
		genHeaders();
		genStubMethods();
	}
	
	/**
	 * Generate the header part of the stub code
	 * (including include statements and
	 * global variables - library name and symbol name)
	 * 
	 * @throws IOException
	 */
	private void genHeaders() throws IOException{
		// Include statements in any stub library should be the same pattern
		pw.println("/* DO NOT EDIT THIS FILE ENLESS REALLY NECESSARY - it is machine generated. */");
		pw.println();
		pw.println("#include <stdio.h>");
		pw.println("#include <stdlib.h>");
		pw.println("#include <assert.h>");
		
		// Print out user defined header files
		Iterator<String> headerItr = headerList.iterator();
		while (headerItr.hasNext())
			pw.println("#include \"" + headerItr.next() +"\"");
		
		pw.println("#include \"librobusta.h\"");
		
		/**
		 * If the common mark is true, generate the header file here
		 * and also include it
		 */
		
		if (cm == true) {
			pw.println("#include \"common.h\"");
			
			String fullpath = dst.substring(0, dst.lastIndexOf('/') + 1) + "common.h";
			BufferedWriter bw = new BufferedWriter(new FileWriter(fullpath));
			PrintWriter cpw = new PrintWriter(bw);
			
			cpw.println("extern void * _handle;");
			cpw.println("extern char * _libName;");
			cpw.println("extern int _sid;");
			cpw.close();
		}
		pw.println();
		
		// Variables for the library
		/**
		 * If the extern mark is true, we only declare the variables here.
		 * Otherwise, define them.
		 */
		if (em == true) {
			pw.println("extern void * _handle;");
			pw.println("extern char * _libName;");
			pw.println("extern int _sid;");
		}
		else {
			pw.println("void * _handle = NULL;");
			pw.println("char * _libName = \"" + lib + "\";");
			pw.println("int _sid = -1;");
		}
		
		// Variables for the symbols
		parseNativeMethods();
		pw.println();
		
		// Extern for global natp list
		pw.println("extern LIST * gl_natp_list;");
		pw.println();
	}
	
	/**
	 * Generate the stub methods for each native methods
	 */
	private void genStubMethods() {
		Iterator<String> methodItr = methodList.iterator();
		Iterator<String> returnItr = returnTypeList.iterator();
		Iterator<ArrayList<String>> argItr = argList.iterator();
		
		while (methodItr.hasNext() && returnItr.hasNext() && argItr.hasNext()) {
			// Method head
			String returnType = returnItr.next();
			String methodName = methodItr.next();
			ArrayList<String> argumentList = argItr.next();
			Iterator<String> argumentItr = argumentList.iterator();
			pw.print("JNIEXPORT " + returnType + " JNICALL " + methodName + " (");
			if (argumentItr.hasNext())
				pw.print(argumentItr.next());
			if (argumentItr.hasNext())
				do {
					pw.print(", " +argumentItr.next());
				} while (argumentItr.hasNext());
			pw.println(") {");
			
			// Method content
			pw.println("\tif (_handle == NULL) {");
			pw.println("\t\tptsb retv = loadLib(env, _libName);");
			pw.println("\t\t_handle = retv.pointer;");
			pw.println("\t\t_sid = retv.sb;");
			pw.println("\t}");
			pw.println("\tif (_addr_" + methodName + " == NULL && _handle != NULL)");
			pw.println("\t\t_addr_" + methodName + " = (void *) loadSym(_handle, _symname_" + methodName + ", _sid);");
			pw.println("\tassert(_addr_" + methodName + " != NULL);");
			pw.println("\tassert(gl_natp_list != NULL);");
			pw.println();
			pw.println("\tnpitem * s = (npitem *) ListFirst(gl_natp_list);");
			pw.println("\tassert(s != NULL);");
			pw.println("\tNATP_PTR stub_natp = NULL;");
			pw.println("\tdo {");
			pw.println("\t\tif (s->env == env) {");
			pw.println("\t\t\tstub_natp = s->natp[_sid];");
			pw.println("\t\t\tbreak;");
			pw.println("\t\t}");
			pw.println("\t} while((s = (npitem *) ListNext(gl_natp_list)) != NULL);");
			pw.println();
			// create a natp if it's NULL
			pw.println("\tif (stub_natp == NULL) {");
			pw.println("\t\tstub_natp = _Z17robusta_init_natpv();");
			pw.println("\t\ts = (npitem *) malloc(sizeof(npitem));");
			pw.println("\t\tif (s == NULL || stub_natp == NULL) {");
			pw.println("\t\t\tprintf(\"STOP! Fail to allocate new npitem or natp!\\n\");");
			pw.println("\t\t\twhile (1) ;");
			pw.println("\t\t}");
			pw.println("\t\ts->env = env;");
			pw.println("\t\tint i;");
			pw.println("\t\tfor (i = 0; i < NUM_SANDBOX; i++) {");
			pw.println("\t\t\tif (i != _sid) s->natp[i] = _Z17robusta_init_natpv();");
			pw.println("\t\t\telse s->natp[i] = stub_natp;");
			pw.println("\t\t}");
			pw.println("\t\tListAppend(gl_natp_list, s);");
			pw.println("\t}");
			
			int cpWord = calcArgSize(argumentList);
			Iterator<String> tempItr = argumentList.iterator();
			
			if (!floatList.isEmpty()) {
				// Generate the int array for of the float arg positions
				Iterator<Integer> floatItr = floatList.iterator();
				/**
				 * I don't understand the reason but if the array is not
				 * large enough, the test program simply crashes.
				 */
				int m = floatList.size() + 10;
				pw.println("\tint mark[" + m + "];");
				for (int i = 0; i < floatList.size(); i++)
					pw.println("\tmark[" + i + "] = " + floatItr.next() + ";");
				pw.println("\tint temp;");
				pw.println("\tfor (temp = " + floatList.size() + "; temp < " + m + "; temp++)");
				pw.println("\t\tmark[temp] = -1;");
				//pw.println("\tmark[" + floatList.size() + "] = -1;");
				pw.println();
				
				/** 
				 * For now, we consider all functions with float args
				 * returning double * (Actually this is not quite right...)
				 * Maybe refine it later...
				 */
				//TODO: Here is not quite right...
				
				pw.print("\tjdouble * result = _Z17call_in_float_argPv(mark, stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", ");
				printArgs(tempItr);
				pw.println(");");
				pw.println("\treturn * result;");
				floatList.clear();
				
			}
			
			else if (returnType.equals("void")) {
				pw.print("\t_Z7call_inPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", ");
				printArgs(tempItr);
				pw.println(");");
				
			}
			else {
				if (returnType.equals("jlong")) {
					pw.print("\tlong long * result = _Z12call_in_longPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", ");
					printArgs(tempItr);
					pw.println(");");
					pw.println("\treturn * result;");
				}
				else if (returnType.equals("jdouble") || returnType.equals("jfloat")) {
					pw.print("\t" + returnType + " * result = _Z13call_in_floatPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", ");
					printArgs(tempItr);
					pw.println(");");
					pw.println("\treturn * result;");
				}
				else {
					pw.print("\t" + returnType + " result = (" + returnType + ") _Z7call_inPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", ");
					printArgs(tempItr);
					pw.println(");");
					pw.println("\treturn result;");
				}
			}
			pw.println("}");
			pw.println();
		}
	}


	/**
	 * Find all native methods in the source file
	 * and store the symbol names in the array.
	 * 
	 * @throws IOException
	 */
	private void parseNativeMethods() throws IOException {
		String line = br.readLine();
		String clearLine;
		do {
			// Eliminate all spaces in the line
			clearLine = line.replaceAll("\\s", "");
			if (!clearLine.startsWith("JNIEXPORT"))
				continue;
			else {
				// 1. Parse the return type
				// startIndex and endIndex are used to mark the substring we need
				
				int startIndex = clearLine.lastIndexOf("JNIEXPORT");
				// move the cursor to the end of JNIEXPORT
				startIndex = startIndex + 9;
				int endIndex = clearLine.lastIndexOf("JNICALL");
				if (endIndex > startIndex) {
					// Here we should have found the return type
					String returnType = clearLine.substring(startIndex, endIndex);
					returnTypeList.add(returnType);
				}
				else if (endIndex == -1) {
					// "JNICALL" is on the next line
					// So return type is the remaining part of this line
					String returnType = clearLine.substring(startIndex);
					returnTypeList.add(returnType);
					
					// Read the next line
					line = br.readLine();
					clearLine = line.replaceAll("\\s", "");
				}
				
				// 2. Parse the method name
				startIndex = clearLine.lastIndexOf("JNICALL");

				if (startIndex == -1)
					continue;
				// move the cursor to the end of JNICALL
				startIndex = startIndex + 7;
				
				/**
				 * It is possible that the function name or arg list,
				 * or arg list appears on the next line
				 * 
				 * For example, people sometimes write:
				 * JNIEXPORT void JNICALL func_name(param)
				 * 
				 * And sometimes:
				 * JNIEXPORT void JNICALL
				 * func_name(param)
				 * 
				 * And sometimes:
				 * JNIEXPORT void JNICALL func_name
				 * 	(param)
				 * 
				 * And sometimes:
				 * JNIEXPORT void JNICALL func_name(param1,
				 * 	param2,
				 * 	param3)
				 * 
				 * For simplicity, we only take these three into consideration.
				 */
				
				if (startIndex >= clearLine.length()) {
					// Case 2
					// Read the next line
					line = br.readLine();
					clearLine = line.replaceAll("\\s", "");
					startIndex = 0;
				}
				
				endIndex = clearLine.indexOf('(');
				if (endIndex > startIndex) {
					// Here we should have found the method name
					String methodName = clearLine.substring(startIndex, endIndex);
					methodList.add(methodName);
				}
				
				else if (endIndex == -1) {
					// Case 3
					// Find the method name first
					endIndex = clearLine.length();
					while (clearLine.charAt(endIndex - 1) == ' ' || clearLine.charAt(endIndex - 1) == '\t')
						endIndex--;
					String methodName = clearLine.substring(startIndex, endIndex);
					methodList.add(methodName);
					// Read in the next line
					line = br.readLine();
					clearLine = line.replaceAll("\\s", "");
				}
				
				// 3. Parse the argument list
				// We use line rather than clearLine to parse the arg list
				
				// Move the cursor to the beginning of the arg list
				ArrayList<String> currList = new ArrayList<String>();
				startIndex = line.indexOf('(') + 1;
				while (line.charAt(startIndex) == ' ')
					startIndex++;
				// endIndex points to the first comma after the type
				// Since this is a native method, it at least has two arguments (env and obj),
				// so we definitely can find the first comma
				endIndex = line.indexOf(',', startIndex);
				do {
					if (endIndex > startIndex) {
						// We have found the argument (including type and name)
						String argRecord = line.substring(startIndex, endIndex);
						// Deal with name conflict
						currList.add(argRecord);
					}
					
					// Adjust to the next argument
					// Notice that it is possible for args to be in different lines
					startIndex = endIndex + 1;
					if (startIndex >= line.length()) {
						line = br.readLine();
						startIndex = endIndex = 0;
						while (line.charAt(startIndex) == ' ' || line.charAt(startIndex) == '\t')
							startIndex++;
					}
					while (line.charAt(startIndex) == ' ' || line.charAt(startIndex) == '\t')
						startIndex++;
					if (line.charAt(startIndex) == '\n' || line.charAt(startIndex) == '\r') {
						// This line has terminated, so read the next line
						line = br.readLine();
						startIndex = endIndex = 0;
						while (line.charAt(startIndex) == ' ' || line.charAt(startIndex) == '\t')
							startIndex++;
					}
					endIndex = line.indexOf(',', startIndex);
				} while (endIndex != -1);
				
				// Here we have reached the end of the arg list
				endIndex = line.indexOf(')', startIndex) - 1;
				while (line.charAt(endIndex) == ' ' || line.charAt(endIndex) == '\t')
					endIndex--;
				// The last argument
				String argRecord = line.substring(startIndex, ++endIndex);
				currList.add(argRecord);
				
				// Add the current list to the argList as a single record
				argList.add(currList);
			}
		} while ((line = br.readLine()) != null);
		genSymVars();
		genPolicy();
	}

	/**
	 * Use the methodList to generate symbol variables.
	 * 
	 * In order not to have duplicated names, we use the symbol name as
	 * part of the variable name
	 */
	
	private void genSymVars() {
		String symName;
		Iterator<String> it = methodList.iterator();
		while (it.hasNext()) {
			symName = it.next();
			pw.println("static void * _addr_" + symName + " = NULL;");
			pw.println("static char * _symname_" + symName + " = \"" + symName + "\";");
		}
	}
	
	/**
	 * Automatically write to (or generate if doesn't
	 * exist) a policy file which simply lists all native
	 * method names within the library.
	 * 
	 * This policy file will be used in Jinn's inspection,
	 * specifically in Native Method Bind event to decide
	 * whether the native method should be checked or not.
	 * 
	 * @throws IOException 
	 */
	
	private void genPolicy() throws IOException {
		// Write to the end of the file
		String currUsrDir = System.getProperties().getProperty("user.home");
		BufferedWriter bw = new BufferedWriter(new FileWriter(currUsrDir + "/.stub.policy", true));
		PrintWriter stubPrinter = new PrintWriter(bw);
		
		// We also need a reader to eliminate duplicates
		BufferedReader br = new BufferedReader(new FileReader(currUsrDir + "/.stub.policy"));
		String name, currLine, refine;
		Iterator<String> it = methodList.iterator();
		Next: while (it.hasNext()) {
			name = it.next();
			
			/**
			 * In order to make it convenient to compare with the class
			 * signature in Robusta, we refine the name a little:
			 * 
			 * 1. Delete the beginning "Java_"
			 * 2. Replace the '_' with '/'
			 * 
			 * After the refinement, the policy file will keep a list
			 * of method names which look like:
			 * simple/helloworld/HelloWorldTest/print
			 * (class signature + method name, use '/'s as separators...
			 */
			
			refine = name.substring(5).replace('_', '/');
			while ((currLine = br.readLine()) != null) {
				if (currLine.equals(refine))
					continue Next;
			}

			// No duplicates here, write the name to the end of the file.
			stubPrinter.println(refine);
		}
		stubPrinter.close();
		br.close();
	}
	
	
	/**
	 * Calculate the total number of words according to the arg list
	 * The JNI types are defined as follows:
	 * 
	 * 9 primitives types:
	 * jboolean, jbyte, jchar, jshort, jint, jlong, jfloat, jdouble, jsize
	 * 
	 * 15 reference types:
	 * jobject, jclass, jstring, jarray, jbooleanArray, jbyteArray,
	 * jcharArray, jshortArray, jintArray, jlongArray, jfloatArray,
	 * jdoubleArray, jobjectArray, jthrowable, jweak
	 * 
	 * 2 identifiers: jmethodID, jfieldID
	 * 
	 * @param args
	 * @return the number of words in total
	 */
	
	private int calcArgSize(ArrayList<String> args) {
		String currArg;
		int word = 0;
		int counter = 0;
		Iterator<String> argItr = args.iterator();
		while (argItr.hasNext()) {
			currArg = argItr.next();
			
			if (currArg.startsWith("JNIEnv")) {
				counter++;
				continue;
			}
			else if (currArg.contains("*")) {
				// All pointers have the same size
				word += 1;
				counter++;
			}
			else if (currArg.startsWith("jboolean")
					|| currArg.startsWith("jbyte")
					|| currArg.startsWith("jchar")
					|| currArg.startsWith("jshort")
					|| currArg.startsWith("jint")
					|| currArg.startsWith("jfloat")
					|| currArg.startsWith("jsize")
					|| currArg.startsWith("jstring")
					|| currArg.startsWith("jobject")
					|| currArg.startsWith("jclass")
					|| currArg.startsWith("jarray")
					|| currArg.startsWith("jmethodID")
					|| currArg.startsWith("jfieldID")
					|| currArg.startsWith("jlongArray")
					|| currArg.startsWith("jdoubleArray")) {
				/**
				 * starting with jboolean includes jboolean and jbooleanArray
				 * and so on and so forth
				 * 
				 * jlong and jdouble are special cases
				 */
				word += 1;
				
				// Set the access flag
				if (currArg.startsWith("jclass") && counter == 1) {
					// This indicates a static native method
					access_flag = 33034;
				}
				else if (currArg.startsWith("jobject") && counter == 1) {
					// This indicates a non-static native method
					access_flag = 33026;
				}
				
				// Check for float argument
				if (currArg.startsWith("jfloat") && !currArg.startsWith("jfloatArray"))
					/**
					 * We consider the first argument after env and obj (or cls)
					 * as argument number 1. So the actual number we stored in the list
					 * is counter - 1.
					 */
					floatList.add(counter - 1);
				counter++;
			}
			else if (currArg.startsWith("jlong")
					|| currArg.startsWith("jdouble")) {
				word += 2;
				counter++;
			}
			else {
				System.out.println("Illegal type in argument: " + currArg);
				System.exit(-1);
			}
		}
		return word;
	}
	
	/**
	 * Print out all the arguments for call_in or call_in_float
	 * 
	 * @param An iterator of the argument list, the list must start with env and obj (or cls)
	 * @return void (just print out)
	 */
	private void printArgs(Iterator<String> it) {
		String argRec = "", argName;
		int startIndex, endIndex;
		/** 
		 * For every record in the ArrayList, we have both its type and name
		 * e.g. jint a
		 * What we need is its name only
		 * 
		 * Note that the argRec could be something like:
		 * JNIEnv * env , (Note all spaces. No space at the beginning)
		 * We should deal with the spaces carefully to find exactly the name
		 */
		
		if (it.hasNext())
			argRec = it.next();
		startIndex = argRec.indexOf(' ');
		// Ignore all stars and spaces
		while (argRec.charAt(startIndex + 1) == '*' || argRec.charAt(startIndex + 1) == ' ')
			startIndex++;
		endIndex = argRec.lastIndexOf(' ');
		
		if (startIndex >= endIndex)
			argName = argRec.substring(startIndex + 1);
		else
			argName = argRec.substring(startIndex + 1, endIndex);
		pw.print(argName);
		
		while (it.hasNext()) {
			pw.print(", ");
			argRec = it.next();
			startIndex = argRec.indexOf(' ');
			while (argRec.charAt(startIndex + 1) == '*' || argRec.charAt(startIndex + 1) == ' ')
				startIndex++;
			endIndex = argRec.lastIndexOf(' ');
			
			if (startIndex >= endIndex)
				argName = argRec.substring(startIndex + 1);
			else
				argName = argRec.substring(startIndex + 1, endIndex);
			pw.print(argName);
		}
	}
}
