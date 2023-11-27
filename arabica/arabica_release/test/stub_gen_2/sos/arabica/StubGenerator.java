package sos.arabica;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * This is the stub library generator v2.0 for Arabica
 * 
 * It is upgraded from v1.0 to generate stub code solely
 * based on Java class files. It functions upon javap command.
 * 
 * @author Mengtao Sun
 */

public class StubGenerator {

	// necessary information for the stub code
	private String javaClass;
	public String lib;   // name of the sandboxed native lib
	private boolean em;	// em stands for "extern mark"
	private final ArrayList<String> methodList;		// list of native method names
	private final ArrayList<String> returnTypeList;	// list of return types
	private final ArrayList<ArrayList<String>> argList;	// list of args
	private final ArrayList<Boolean> staticList;	// whether it is static or not
	private final ArrayList<Integer> floatList;		// list of float args (positions)
	String target;		// name of the class

	// the writer to write to the stub file
	private PrintWriter pw;	

	/**
	 * Constructor for the class
	 * 
	 * @param
	 * a: the output file as a PrintWriter
	 * b: the input file as a BufferedReader
	 * etc...
	 */
	public StubGenerator(String target) {
		this.pw = null;
		this.em = true;
		this.lib = null;
		this.methodList = new ArrayList<String>();
		this.returnTypeList = new ArrayList<String>();
		this.argList = new ArrayList<ArrayList<String>>();
		this.staticList = new ArrayList<Boolean>();
		this.floatList = new ArrayList<Integer>();
		this.target = target;
	}

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
	 * Take in a java type
	 * and find the corresponding JNI type
	 * 
	 * @param java type as a string
	 * @return its corresponding JNI type
	 */
	private String javaTypeToJNI (String jtype)
	{
		String jni;
		
		// primitive types
		if (jtype.equals("void"))
			jni = "void";
		else if (jtype.equals("boolean"))
			jni = "jboolean";
		else if (jtype.equals("byte"))
			jni = "jbyte";
		else if (jtype.equals("char"))
			jni = "jchar";
		else if (jtype.equals("short"))
			jni = "jshort";
		else if (jtype.equals("int"))
			jni = "jint";
		else if (jtype.equals("long"))
			jni = "jlong";
		else if (jtype.equals("float"))
			jni = "jfloat";
		else if (jtype.equals("double"))
			jni = "jdouble";
		
		// reference types
		else if (jtype.equals("java.lang.Object"))
			jni = "jobject";
		else if (jtype.equals("java.lang.Class"))
			jni = "jclass";
		else if (jtype.equals("java.lang.String"))
			jni = "jstring";
		else if (jtype.equals("java.lang.Object[]"))
			jni = "jobjectArray";
		else if (jtype.equals("boolean[]"))
			jni = "jbooleanArray";
		else if (jtype.equals("byte[]"))
			jni = "jbyteArray";
		else if (jtype.equals("char[]"))
			jni = "jcharArray";
		else if (jtype.equals("short[]"))
			jni = "jshortArray";
		else if (jtype.equals("int[]"))
			jni = "jintArray";
		else if (jtype.equals("long[]"))
			jni = "jlongArray";
		else if (jtype.equals("float[]"))
			jni = "jfloatArray";
		else if (jtype.equals("double[]"))
			jni = "jdoubleArray";
		else if (jtype.equals("java.lang.Throwable"))
			jni = "jthrowable";
		
		else {
			System.out.println("Illegal type: " + jtype);
			jni = null;
			System.exit(-1);
		}
		return jni;
	}

	/**
	 * Take in the first line output by javap
	 * and find the class name contained in it
	 * 
	 * @param the first line of the output of javap
	 * @return the name of the class (include package)
	 */
	private String parseClassName (String info)
	{
		// find the first occurrence of 'class'
		int begin = info.indexOf("class");
		// we pass through 'class', and we're now on the name we need
		begin = begin + 6;
		// find the end of the name then
		int end = info.indexOf(' ', begin);
		// return the class name
		return info.substring(begin, end);
	}

	/**
	 * All method signatures in javap's output is a single line
	 * this makes our life much easier
	 * 
	 * @param a method signature line from javap
	 * @return nothing, except populating those fields
	 */
	private void parseNativeMethodInfo (String info)
	{
		// 1) check it's static or not
		if (info.contains("static"))
			staticList.add(true);
		else
			staticList.add(false);

		// 2) find the return type
		// which locates right after the native keyword
		int begin = info.indexOf("native");
		begin = info.indexOf(' ', begin) + 1;
		int end = info.indexOf(' ', begin);
		String ret = info.substring(begin, end);
		
		// convert the java type to the JNI type
		String jniret = javaTypeToJNI(ret);
		returnTypeList.add(jniret);

		// 3) find the method name
		// which locates right after the return type
		begin = end + 1;
		end = info.indexOf('(', begin);
		String method = info.substring(begin, end);
		methodList.add(method);

		// 4) find the arg list
		// which located inside the parentheses
		ArrayList<String> args = new ArrayList<String>();
		String arg;
		begin = end + 1;
		end = info.indexOf(',', begin);
		// iterate until there's no comma any more
		while (end != -1) {
			// we have found the next comma, which indicates an arg
			arg = info.substring(begin, end);
			args.add(javaTypeToJNI(arg));
			// locate the next arg if possible
			// in the output of javap, there's always a space after the comma
			begin = end + 2;
			end = info.indexOf(',', begin);
		}
		// here we still have one last arg to include
		end = info.indexOf(')', begin);
		arg = info.substring(begin, end);
		if (arg.length() > 0)
			args.add(javaTypeToJNI(arg));
		// add this list to the global list of lists
		argList.add(args);
	}

	/**
	 * Find the name of the target library
	 * Since we need to deal with multiple lines,
	 * we take the Reader obj as an arg
	 * 
	 * @param a reader that reads the output of javap
	 * @return nothing, except populating the lib name field
	 */
	private void parseLibName (BufferedReader r) throws IOException
	{
		String line;
		// find the ldc instruction, which pushes the static string
		// onto the stack
		while (!((line = r.readLine()).contains("ldc"))
				|| !(line.contains("String"))) ;
		// locates to the beginning of the name
		// 7 is the length of the word "String" + 1 (space)
		int begin = line.indexOf("String") + 7;
		String name = line.substring(begin);
		lib = name;
	}


	/**
	 * Run the javap command under dir and put its output
	 * into a temporary file
	 * 
	 * @param the working directory
	 * @return the File object
	 */
	public File runJavap (File dir) throws Exception
	{
		// we have to run javap with '-c' option
		// because we need the lib's name in the static block
		String[] cmd = new String[4];
		cmd[0] = "javap";
		cmd[1] = "-c";
		cmd[2] = "-private";
		cmd[3] = target;

		Process sub = Runtime.getRuntime().exec(cmd, null, dir);
		// a reader to read the output of the subprocess
		InputStreamReader isr = new InputStreamReader(sub.getInputStream());
		BufferedReader br = new BufferedReader(isr);
		// a writer to write the output to a file
		File temp = new File("output.txt");
		PrintWriter pw = 
				new PrintWriter(new BufferedWriter(new FileWriter(temp)));
		String s;
		while ((s = br.readLine()) != null) {
			pw.println(s);
		}
		pw.close();

		//return the file handle to the caller
		return temp;
	}

	/**
	 * Analyze the output of javap and find all the information
	 * we need to generate stub code
	 * 
	 * @param the file keeping the output of javap
	 */
	public void processJavap (File output) throws IOException
	{
		BufferedReader br =
				new BufferedReader(new FileReader(output));

		// first find the line including the name of the class
		String line;
		while (!(line = br.readLine()).contains("class")) ;
		
		javaClass = parseClassName(line);

		// find the remaining information we need
		while ((line = br.readLine()) != null) {
			// we only pay attention to the native methods
			if (line.contains("native") && line.contains("(")
					&& line.contains(")"))
				parseNativeMethodInfo(line);

			// we also need to know the name of the library
			else if (line.contains("static {};")) {
				parseLibName(br);
			}
		}

		// close the stream
		br.close();
	}

	/**
	 * Use the methodList to generate symbol variables.
	 * 
	 * In order not to have duplicated names, we use the symbol name as
	 * part of the variable name
	 */

	private void genSymVars() {
		String symName;
		String symFullName;
		Iterator<String> it = methodList.iterator();
		while (it.hasNext()) {
			symName = it.next();
			// the symbol name should conform to JNI specification
			String classPrefix = javaClass.replace('.', '_');
			symFullName = "Java_" + classPrefix + "_" + symName;
			pw.println("static void * _addr_" + symName + " = NULL;");
			pw.println("static char * _symname_" + symName + " = \"" + symFullName + "\";");
		}
	}

	/**
	 * Generate the header part of the stub code
	 * (including include statements and
	 * global variables - library name and symbol name)
	 * 
	 * @param path: the sub folder
	 * @throws IOException
	 */
	private void genHeaders(String path) throws IOException{
		// Include statements in any stub library should be the same pattern
		pw.print("/* DO NOT EDIT THIS FILE ENLESS REALLY NECESSARY ");
		pw.println("- it is machine generated. */");
		pw.println();
		pw.println("#include <stdio.h>");
		pw.println("#include <stdlib.h>");
		pw.println("#include <assert.h>");

		pw.println("#include \"librobusta.h\"");

		// generate the header file here and also include it
		pw.println("#include \"common.h\"");

		// String fullpath = dst.substring(0, dst.lastIndexOf('/') + 1) + "common.h";
		String fullpath = path + "/common.h";
		BufferedWriter bw = new BufferedWriter(new FileWriter(fullpath));
		PrintWriter cpw = new PrintWriter(bw);

		cpw.println("extern void * _handle;");
		cpw.println("extern char * _libName;");
		cpw.println("extern int _sid;");
		cpw.close();

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
			pw.println("char * _libName = \"lib/lib" + lib + ".so\";");
			pw.println("int _sid = -1;");
		}

		// Variables for the symbols
		genSymVars();		
		pw.println();

		// Extern for global natp list and the global lock (defined in Arabica)
		pw.println("extern LIST * gl_natp_list;");
		pw.println("extern pthread_mutex_t gl_natp_list_lock;");
		pw.println();
	}

	/**
	 * Calculate the total number of words according to the arg list
	 * 
	 * We need to do a translation from Java types to JNI types.
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
	 * @param the list of argument types
	 * @return the number of words in total
	 */

	private int calcArgSize(ArrayList<String> args) {
		String currArg;

		// the list on the Java side doesn't include JNIEnv * and jclass/jobject
		// so we have to include the size of the cls/obj ourselves
		int word = 1;
		int counter = 0;
		Iterator<String> argItr = args.iterator();
		while (argItr.hasNext()) {
			currArg = argItr.next();

			if (currArg.contains("*")) {
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

				// Check for float argument
				if (currArg.startsWith("jfloat") && !currArg.startsWith("jfloatArray"))
					/**
					 * We consider the first argument after env and obj (or cls)
					 * as argument number 1. So the actual number we stored in the list
					 * is counter + 1.
					 */
					floatList.add(counter + 1);
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
	 * @param the name (char) of the last arg, the static bool
	 * @return void (just print out)
	 */
	private void printArgs(char last, boolean isStatic) {
		if (isStatic)
			pw.print("cls");
		else
			pw.print("obj");
		int count = last - 'a';
		char arg = 'a';
		for (int i = 0; i < count; i++)
			pw.print(", " + (arg++));
	}

	/**
	 * Generate the stub methods for each native methods
	 */
	private void genStubMethods() {
		Iterator<String> methodItr = methodList.iterator();
		Iterator<String> returnItr = returnTypeList.iterator();
		Iterator<ArrayList<String>> argItr = argList.iterator();
		Iterator<Boolean> staticItr = staticList.iterator();

		while (methodItr.hasNext() && returnItr.hasNext()
				&& argItr.hasNext() && staticItr.hasNext()) {
			String returnType = returnItr.next();
			String methodName = methodItr.next();
			ArrayList<String> argumentList = argItr.next();
			Iterator<String> argumentItr = argumentList.iterator();
			Boolean isStatic = staticItr.next();

			// get the access flag bits so that we can use them later
			String access_flag;
			if (isStatic)
				access_flag = "33034";
			else
				access_flag = "33026";

			// 1) Method head (signature)
			// we need the full name here which conforms to JNI spec
			String fullMethodName =
					"Java_" + javaClass.replace('.', '_') + "_" + methodName;
			pw.print("JNIEXPORT " + returnType + " JNICALL "
					+ fullMethodName + " (JNIEnv * env, ");
			if (isStatic)
				pw.print("jclass cls");
			else
				pw.print("jobject obj");

			// the argumentList only contains the types of the arguments
			// so we need to give names to them
			char v = 'a';
			if (argumentItr.hasNext())
				do {
					pw.print(", " + argumentItr.next() + " " + v++);
				} while (argumentItr.hasNext());
			pw.println(") {");

			// Method content
			pw.println("\tpthread_mutex_lock (&gl_natp_list_lock);");
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
			pw.println("\tpthread_mutex_unlock (&gl_natp_list_lock);");

			int cpWord = calcArgSize(argumentList);

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

				pw.print("\tjdouble * result = _Z17call_in_float_argPv(mark, stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", env, ");
				printArgs(v, isStatic);
				pw.println(");");
				pw.println("\treturn * result;");
				floatList.clear();

			}

			else if (returnType.equals("void")) {
				pw.print("\t_Z7call_inPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", env, ");
				printArgs(v, isStatic);
				pw.println(");");

			}
			else {
				if (returnType.equals("jlong")) {
					pw.print("\tlong long * result = _Z12call_in_longPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", env, ");
					printArgs(v, isStatic);
					pw.println(");");
					pw.println("\treturn * result;");
				}
				else if (returnType.equals("jdouble") || returnType.equals("jfloat")) {
					pw.print("\t" + returnType + " * result = _Z13call_in_floatPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", env, ");
					printArgs(v, isStatic);
					pw.println(");");
					pw.println("\treturn * result;");
				}
				else {
					pw.print("\t" + returnType + " result = (" + returnType + ") _Z7call_inPv(stub_natp, _sid, " + cpWord + ", " + access_flag + ", _addr_" + methodName + ", env, ");
					printArgs(v, isStatic);
					pw.println(");");
					pw.println("\treturn result;");
				}
			}
			pw.println("}");
			pw.println();
		}
	}

	/**
	 * Generate the stub library source code
	 */
	public void genStub(String path, int num) throws IOException{
		// get the writer to the stub file
		pw = new PrintWriter(new FileWriter(path + "/stub-" + num + ".c"));
		// if we're generating the first stub file,
		// set the extern mark to true
		if (num == 0)
			em = false;

		genHeaders(path);
		genStubMethods();
		pw.close();
	}
}
