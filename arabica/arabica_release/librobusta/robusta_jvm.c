/*
 * Copyright 2013, SoS Laboratory, Lehigh University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of SoS Laboratory, Lehigh University nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// G.T.
// This file is copied from src/trusted/service_runtime/sel_main.c
// with some adaptation

/*
 * Copyright 2008, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL).
 */
#include "RobustaWatcher.h"
#include "librobusta.h"
#include "robusta_common.h"
#include "librobusta/dynlink2/sel_ldr_dl.h"
#include "robusta_jni_checks.h"
#include "stub_policy.h"
#include <pthread.h>

// Native Client imports
#include "native_client/src/include/portability.h"
#include "native_client/src/include/portability_io.h"

// Standard library imports
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>

// Native Client imports
#include "native_client/src/shared/imc/nacl_imc_c.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"

#include "native_client/src/trusted/platform/nacl_log.h"
#include "native_client/src/trusted/platform/nacl_sync.h"
#include "native_client/src/trusted/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/platform_qualify/nacl_os_qualify.h"

#include "native_client/src/trusted/service_runtime/env_cleanser.h"
#include "native_client/src/trusted/service_runtime/expiration.h"
#include "native_client/src/trusted/service_runtime/gio.h"
#include "native_client/src/trusted/service_runtime/nacl_app.h"
#include "native_client/src/trusted/service_runtime/nacl_all_modules.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"
#include "native_client/src/trusted/service_runtime/nacl_ldt.h"

#define WORD_SIZE 4
#define HANDLE_TABLE_SIZE 100

// NUM_SANDBOX is defined in librobusta.h

static int handle_table[HANDLE_TABLE_SIZE];

#define PTR_ALIGN_MASK  ((sizeof(void *))-1)
#define RTLD_LAZY	0x00001	/* Lazy function call binding.  */

#define BLAT(t, v) do { \
    *(t *) p = (t) v; p += sizeof(t); \
  } while (0);

/* may redefine main() to install a hook */
#if defined(HAVE_SDL)
#include <SDL.h>
#endif

int verbosity = 0;

// mes310 multiple sandboxes
static struct NaClApp                *nap[NUM_SANDBOX];
static struct NaClAppThread	     *global_natp[NUM_SANDBOX];
pthread_mutex_t global_natp_locks[NUM_SANDBOX];
// pthread_mutex_t global_natp_lock = PTHREAD_MUTEX_INITIALIZER;

struct GioMemoryFileSnapshot  gf;

// current number of occupied sandboxes
static int cur_sb = 0;

//these routines deal with manipulating the handle table...
void initHandleTable()
{
	int i;
	for(i=0; i<HANDLE_TABLE_SIZE; i++)
	{
		handle_table[i]=0;
	}
}

//returns the slot if one is available, -1 otherwise
int storeHandle(int handle)
{	
	int i;
	
	for(i=0; i<HANDLE_TABLE_SIZE; i++)
	{
		if(handle_table[i]==0)
		{
			//found a slot for this handle
			handle_table[i]=handle;
			return 1;
		}
	}
	return 0;
}

//returns the slot for the handle, -1 if it is not sandboxed
int isHandleSandboxed(int handle)
{
	int i;
	for(i=0; i<HANDLE_TABLE_SIZE; i++)
	{
		if(handle_table[i]==handle)
		{
			//found this slot's handle, return true
			return 1;
		}
	}
	return 0;
}

void initStubPolicies() {
  readStubPolicies();
}

//Duplicate a name_comparator for Robusta 2.0
int name_comparator(void * item1, void * item2)
{
	//item1 is a robusta_jmethodID node, item2 is an ordinary jmethodID
	Stub_Policy * node = (Stub_Policy *) item1;
	char * method_name = (char *) item2;

	//printf("[robusta_jvm.c: name_comparator:] node->class_name: %s, class_name: %s\n", node->class_name, class_name);

	//returns 1 if the names match, 0 otherwise
	return !strcmp(method_name, node->method_name);
}

int jvm_JNIvars_valid = 0;

// Initialize all JNI variables so that they are accessible. This is stuff like exceptions, securitymanager, etc
void robusta_initJNIvars(JNIEnv * env)
{
	NaClLog(0, "[robusta_jvm.c: initJNIvars:] env: %p\n", env);

	accessControlException = (*env)->FindClass(env, "java/security/AccessControlException");

	//In Java 2, the AccessController backs the SecurityManager, so we call it directly to save overhead
	access_controller = (*env)->FindClass(env, "java/security/AccessController");

	permission_manager = (*env)->FindClass(env, "java/lang/SecurityManager");

	permission_manager_ctor = (*env)->GetMethodID(env, permission_manager, "<init>", "()V");

	permission_manager_inst = (*env)->NewObject(env, permission_manager, permission_manager_ctor);

	//get the checkPermission method reference
	check_permission = (*env)->GetStaticMethodID(env, access_controller, "checkPermission", "(Ljava/security/Permission;)V");

	ck_perm = (*env)->GetMethodID(env, permission_manager, "checkPermission", "(Ljava/security/Permission;)V");

	//get the UseSNLPermission class reference
	runtime_permission = (*env)->FindClass(env, "java/lang/RuntimePermission");

	//get the constructorID
	runtime_permission_ctor = (*env)->GetMethodID(env, runtime_permission, "<init>", "(Ljava/lang/String;)V");

	file_permission = (*env)->FindClass(env, "java/io/FilePermission");

	file_permission_ctor = (*env)->GetMethodID(env, file_permission, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");

	#ifdef SANDBOX_MEASUREMENT
		robusta_watcher = (*env)->FindClass(env, "fixture/RobustaWatcher");

		update_sand = (*env)->GetStaticMethodID(env, robusta_watcher, "updateSand", "(I)V"); 
	#endif

	jvm_JNIvars_valid = 1;
}

// Duplicate a method for stub policy referencing
int robusta_methodUsesStubLibraries(char * method_name) {
  NaClLog(1, "[robusta_jvm.c: methodUsesStubLibraries] Checking policy file for: %s\n", method_name);
  Stub_Policy * node = ListSearch(stub_policies, &name_comparator, method_name);
  return (node != NULL);
}

void * lsym(void * handle, const char * symbolname, int sid)
{
	void * addr;
	size_t size;

	// stack layout that dlsym expects:
	//        | return addr | hanhandledle | symbol ptr | the actual symbol | ...
	//        ^ 
	//        |
	//       esp

	//acquire a lock for the global natp
	pthread_mutex_lock( &global_natp_locks[sid] );
  
	uintptr_t usr_esp = NaClUserToSys(nap[sid],
					  global_natp[sid]->user.esp);

	// calculate the size of the things that need to be pushed into the stack
	size = strlen(symbolname) + 1; // the filename string
	size += sizeof(char *); // the pointer to the symbol
	size += sizeof(void *); // handle
	size += sizeof(int *); // a dummy return address
	size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK;

	//TODO: should check for stack overflow
	usr_esp = usr_esp - size;
	global_natp[sid]->user.esp = NaClSysToUser(global_natp[sid]->nap,
						   usr_esp);

	char *p = (char *) usr_esp;
	char *strp = p + sizeof(int *) + sizeof(void *) + sizeof(char *);

	BLAT(int *, 0);
	BLAT(void *, handle);
	BLAT(char *, NaClSysToUser(nap[sid], (uintptr_t)strp));
	strcpy(strp, symbolname);

	addr = (void *) setjmp(global_natp[sid]->snapshot);
	
	if(addr == 0)
	{
		// invoking dlsym in the sandbox
		NaClLog(0,"[Robusta.c: lsym] Jumping into the sandbox at addr %p\n", nap[sid]->dlSymWrap);
		NaClStartThreadInApp(global_natp[sid],
				     (int) nap[sid]->dlSymWrap);
	}

	//mes310 satisfy new outofjail
	addr = (void *)(*((int *) addr));
	
	//check to see if the symbol was not found
	if((int)addr == -1)
	{
		//the JVM expects a 0 address to represent an unknown symbol
		addr=0;
	}

	//TODO: free the space on the global natp stack
	
	// release global natp lock
	pthread_mutex_unlock( &global_natp_locks[sid] );
	
	NaClLog(0,"[Robusta.c: lsym] Returning %p as the symbol address.\n", addr);

	return addr;
}

//this hook is called by share/native/java/lang/ClassLoader.c in the JVM
void * loadSym(void * handle, char * symbolname_param, int sid)
{
	//adjust for the g++/gcc calling convention inconsistency...
	//the actual stack address of this parameter is one off...

	//mes310
	//after moving hooks out of JVM, adjustment is no longer needed
	char * symbolname = symbolname_param;

	NaClLog(0, "[Robusta.c: loadSym:] Looking for symbol: %s in library %i in sandbox %d\n", symbolname, (int) handle, sid);

	if(handle != NULL && isHandleSandboxed((int)handle) && symbolname != NULL )
	{
		//this handle is sandboxed but the symbol might not be in this library, 
		//so we must differentiate between two cases
		//(0 means this handle is sandboxed, but that the symbol was not found in this library)
	  return lsym(handle, symbolname, sid);
	}
	return (void *) -1;
}

int llib(char * filename)
{
	//handle of the library loaded
	int handle;

	// current sandbox ID
	int c = cur_sb;

	size_t size;
  
  	// stack layout that dlopen expects:
	//        | return addr | filename ptr | flag | the actual filename | ...
	//        ^ 
	//        |
	//       esp
	

  
	uintptr_t usr_esp = NaClUserToSys(nap[c], global_natp[c]->user.esp);

	// calculate the size of the thingNaClOpenAclChecks that need to be pushed into the stack
	size = strlen(filename) + 1; // the filename string
	size += sizeof(int); // the flag parameter
	size += sizeof(char *); // the pointer to the filename 
	size += sizeof(int *); // a dummy return address; never used by dlopenWrapper
	size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK;

	// todo: should check for stack overflow
	//usr_esp = usr_esp - size;
	global_natp[c]->user.esp = NaClSysToUser(nap[c], usr_esp);

	char *p = (char *) usr_esp;
	char *strp = p + sizeof(int *) + sizeof(char *) + sizeof(int);

	BLAT(int *, 0);
	BLAT(char *, NaClSysToUser(nap[c], (uintptr_t)strp));
	BLAT(int, RTLD_LAZY);
	strcpy(strp, filename);

	/*
	int k;
	for (k=(int)usr_esp; k< ((int) usr_esp + size); k++) {
		printf("0x%x: 0x%.2x \n", k, *((char *) k));
	}
	*/

	handle = setjmp(global_natp[c]->snapshot);

	if(handle == 0)
	{
		// invoking dlopenWrapper in the sandbox
		NaClLog(0,"[robusta.c: llib] llib jumping to %p\n", nap[c]->dlOpenWrap);
		NaClStartThreadInApp(global_natp[c], (int) nap[c]->dlOpenWrap);
	}

	//mes310 satisfy the new outofjail
	handle = (* ((int *) handle));

	NaClLog(0,"[robusta.c: llib] Returned the handle: %i\n", handle);
	printf("[robusta.c: llib] Returned the handle: %i\n", handle);
	fflush(stdout);
	
	//store the handle in our local handle table so we can tell if we need
	//to do symbol lookup in the future.
	//TODO: this is currently ***NOT*** threadsafe
	//(eventually store this table in nap?)
	if(!storeHandle(handle))
	{
		printf("[robusta.c: llib ***FATAL***] Error storing sandbox handle: %i\n", handle);
	}

	return handle;
}

//leaves lib_name untouched...
jstring formatLibraryString(JNIEnv * env, char * lib_name)
{
	char * temp;
	int len;
	int c_lib_name_len;
	char * c_lib_name, * so_name;
	jstring j_lib_name;

	len = strlen(lib_name);

	//only copy past the last slash
	temp = strrchr(lib_name, '/');

	//printf("lib_name: %x temp: %x\n", lib_name, temp);

	len = (len-(temp - lib_name)) - 1;

	//printf("len: %i\n", len);

	so_name = malloc(len*sizeof(char)+1);

	len=len-6;
	strncpy(so_name, temp+4, len);

	so_name[len]='\0';

	//printf("Soname: %s\n", so_name);

	c_lib_name_len = strlen("loadSNL.") + len;

	//allocate enough space for the '\0'
	c_lib_name = malloc(c_lib_name_len*sizeof(char)+1);

	strcpy(c_lib_name, "loadSNL.");

	//move to the period
	temp = c_lib_name + strlen("loadSNL.");

	//copy the lib name into the string
	//(null-terminates)
	strncpy(temp, so_name, len+1);

	//printf("Result: %s\n", c_lib_name);

	//create the jstring...
	j_lib_name = (*env)->NewStringUTF(env, c_lib_name);

	free(so_name);
	free(c_lib_name);

	return j_lib_name;
}

//this hook is called by jdk/src/share/native/java/lang/ClassLoader.c in the JVM
ptsb loadLib(JNIEnv * env, char * filename)
{
	int handle;

	ptsb retval;

	// currently we only support a static number of sandboxes
	if (cur_sb >= NUM_SANDBOX) {
	  fprintf(stderr, "Error: all sandboxes are occupied.\n");
	  retval.pointer = (void *) -1;
	  retval.sb = -1;
	  return retval;
	}

	// this should never happen...
	if (cur_sb < 0) {
	  fprintf(stderr, "Error: unknown error.\n");
	  retval.pointer = (void *) -1;
	  retval.sb = -1;
	  return retval;
	}

	// acquire the corresponding global natp lock
	pthread_mutex_lock( &global_natp_locks[cur_sb] );

	//TODO: hack until I can find some other way...
	global_natp[cur_sb]->real_env = env;

	NaClLog(0,"[robusta.c: loadLib:] examining policy for library: %s\n", filename);
	//mes310
	// in order to accomodate libjava.so, we here delete the
	// detection for jvm_JNIvars_valid
	//if(filename != NULL && jvm_JNIvars_valid)
	if (filename != NULL)
	{	
		jstring j_lib_name = formatLibraryString(env, filename);

		// mes310 disable the hack??
		//Temporary hack--------------------------------------------------------------------
		/*
		const jbyte * temp = (*env)->GetStringUTFChars(env, j_lib_name, 0);

		if(!strcmp((char *) temp, "loadSNL.awt") || !strcmp((char *) temp, "loadSNL.mawt"))
		{
			//explicit deny hack for the awt library so the jpeg decoder works
			(*env)->ReleaseStringUTFChars(env, j_lib_name, (char*) temp);

			pthread_mutex_unlock( &global_natp_locks[cur_sb] );
			//return -1;
			retval.pointer = (void *) -1;
			retval.sb = -1;
			return retval;
		}
		(*env)->ReleaseStringUTFChars(env, j_lib_name, (char *) temp);
		*/
		//End of temporary hack-------------------------------------------------------------

		//call the constructor for UseSNLPermission

		//mes310
		//jobject runtime_permission_inst = (*env)->NewObject(env, runtime_permission, runtime_permission_ctor, j_lib_name);
	
		//NaClLog(0, "%p\n", runtime_permission_inst);
	
		//NaClLog(0, "Back from the constructor\n");

		//NaClLog(0, "Calling checkPermission: permission_manager_inst: %p ck_perm: %p runtime_permission_inst: %p\n", permission_manager_inst, ck_perm, runtime_permission_inst);

		//call the checkPermission method. Throws an exception if the policy is violated.
		//mes310
		//(*env)->CallStaticVoidMethod(env, access_controller, check_permission, runtime_permission_inst);

		//(*env)->CallVoidMethod(env, permission_manager_inst, ck_perm, runtime_permission_inst);
	
		//(*env)->CallStaticVoidMethod(env, access_controller, check_permission, runtime_permission_inst);

		jthrowable exc = (*env)->ExceptionOccurred(env);

		//NaClLog(0, "Back from checkPermission: %p\n", exc);

		if(!exc)
		{
			handle = llib(filename);

			retval.pointer = (void *) handle;
			retval.sb = cur_sb++;

			// release the global natp lock
			pthread_mutex_unlock(&global_natp_locks[cur_sb - 1]);

			return retval;

		}

		if((*env)->IsInstanceOf(env, exc, accessControlException))
		{
			NaClLog(0, "Not a sandboxed library.\n");
			(*env)->ExceptionClear(env);
		}
		else
		{
			//Unknown exception, die!
			//don't clear it so that we crash
			(*env)->ExceptionDescribe(env);
		}
	}
	
	//Release the global natp lock
	pthread_mutex_unlock( &global_natp_locks[cur_sb] );	

	retval.pointer = (void *) -1;
	retval.sb = -1;
	
	return retval;
}

// G.T.: changed main to loadNaclApp
int initSandbox() {

	// initialize the lock list first...
	int c;
	for (c = 0; c < NUM_SANDBOX; c++) {
	  pthread_mutex_init(&global_natp_locks[c], NULL);
	}

	char * env_verbosity;
	char const *const *envp;
	char *nacl_file = 0;
	struct GioMemoryFileSnapshot gf[NUM_SANDBOX];
	NaClErrorCode errcode;
	extern char **environ;
	struct NaClEnvCleanser filtered_env;

	#ifdef SANDBOX_MEASUREMENT
		robusta_watcher = NULL;
		update_sand= NULL;
	#endif

	//initialize the per-class sandbox policies
	//Sandbox policy is used in Robusta 1.0 while Stub policy is used in 2.0
	//initSandboxPolicies();
	initStubPolicies();

	//Initialize library handle table
	initHandleTable();
	
	file_desc_list = ListCreate();

	envp = (char const *const *) environ;

	// allocate space for each sandbox
	for (c = 0; c < NUM_SANDBOX; c++) {
	  nap[c] = malloc(sizeof (struct NaClApp));
	  if (nap[c] == NULL) {
	    fprintf(stderr, "Error while allocating space for sandbox %d\n", c);
	    return 1;
	  }
	}
	
	NaClAllModulesInit();

	//you can set the verbosity level here...
	//robusta_setverb
	//NaClLogSetVerbosity(LOG_FATAL);
	NaClLogSetVerbosity(0);

	if (NULL != (env_verbosity = getenv("NACLVERBOSITY"))) 
	{
    		int v = strtol(env_verbosity, (char **) 0, 0);
		NaClLogSetVerbosity(v);
	}
	else
	{
		NaClLogSetVerbosity(LOG_FATAL);
	}

	NaClLogDisableTimestamp();

	//debug mode for now...
	NaClInsecurelyBypassAllAclChecks();
        NaClIgnoreValidatorResult();

	//hard code nacl file for now...
//	nacl_file="/home/<username>/robustatrunk/MSNaCl/nacl-build/googleclient/native_client/install-stubout/lib/ld-linux.so.2";
    nacl_file="/home/hannah/Documents/Repos/nacl/build/linux/debian_bullseye_i386-sysroot/lib/i386-linux-gnu/ld-linux.so.2";
	
	// construct dl file snapshots
	for (c = 0; c < NUM_SANDBOX; c++) {
	  int temp = GioMemoryFileSnapshotCtor(&(gf[c]), nacl_file);
	  //if (GioMemoryFileSnapshotCtor(&(gf[c]), nacl_file) == 0) {
	  if (temp == 0) {
	    perror("sel_main");
	    fprintf(stderr, "Cannot open \"%s\" in sandbox %d.\n",
		    nacl_file, c);
	    return 1;
	  }
	}
	
	// construct app state in each sandbox
	for (c = 0; c < NUM_SANDBOX; c++) {
	  int temp = NaClAppCtor(nap[c]);
	  //if (!NaClAppCtor(nap[c])) {
	  if (!temp) {
	    fprintf(stderr,
		    "Error while constructing app state in sandbox %d\n", c);
	    goto done_file_dtor;
	  }
	}

	errcode = LOAD_OK;

	// load the dl file into each sandbox
	if (LOAD_OK == errcode) 
	{
	  for (c = 0; c < NUM_SANDBOX; c++) {
		errcode = NaClAppLoadDynFile((struct Gio *) (gf + c), nap[c]);
		if (LOAD_OK != errcode) 
		{
			nap[c]->module_load_status = errcode;
			fprintf(stderr, "Error while loading \"%s\" in sandbox %d: %s\n",
				nacl_file, c,
				NaClErrorString(errcode));
		}
	  }
	}

	//dl stuff
	int argc;
	char **argv;
	char *gv[2];
	argc = 2;
	gv[0] = "NaClMain";
    gv[1] = "/home/Documents/Repos/securing-real-world-systems-database-isolation/arabica/arabica_release/librobusta/dynlink2/tests/dynlink";
//	gv[1] = "/home/<username>/robustatrunk/librobusta/dynlink2/tests/dynlink";
	argv = gv;

	// Jumping into each sandbox..
	
	for (c = 0; c < NUM_SANDBOX; c++) {
	  if (LOAD_OK == errcode) {
	    //always print to stdout & stderror instead of a log file (for now)
	    errcode = NaClAppPrepareToLaunch(nap[c], 0, 1, 2);
	  }

	  NaClLog(1, "Jumping into the sandbox %d...\n", c);

	  fflush((FILE *) NULL);

	  NaClXMutexLock(&nap[c]->mu);
	  nap[c]->module_load_status = LOAD_OK;
	  NaClXCondVarBroadcast(&nap[c]->cv);
	  NaClXMutexUnlock(&nap[c]->mu);

	  NaClEnvCleanserCtor(&filtered_env);
	  if (!NaClEnvCleanserInit(&filtered_env, envp)) {
	    fprintf(stderr, "Filtering environment variables failed in sandbox %d\n", c);
	    NaClEnvCleanserDtor(&filtered_env);
	    goto done;
	  }

	  //jjs204
	  /*Test code to eliminate additional thread*/

	  //NaClExecApp is implemented in
	  //native_client/src/trusted/service_runtime/sel_ldr_standard.c

	  global_natp[c] = NaClExecApp(nap[c],
				    argc,
				    argv,
				    NaClEnvCleanserEnvironment(&filtered_env)); 

	  //initialize the IDs for checks
	  //initIDs(natp->real_env);
	}

	int i=0;

	done:
	//jjs204 temporarily here until I can decide what to do...
	i++;
	done_file_dtor:
	i++;

	return 0;

}

//creates a sandbox
//called from jdk/src/share/bin/java.c
void initRobusta () 
{
  //printf("Testing...2\n");
  initSandbox();
}

void lazy_allocate_natp(struct NaClAppThread * natp, int sid)
{
	NaClLog(1, "[robusta_jvm.c: lazy_allocate_natp] natp: %p\n", natp);
	
	//void * usr_esp;
	//void * usr_tdb;
	void * sys_tdb;
	uint16_t  gs;

	//acquire global_natp_lock
	pthread_mutex_lock( &global_natp_locks[sid] );
	
	void * result = (void *) setjmp(global_natp[sid]->snapshot);

	if(result == 0)
	{
	  //NaClSwitchToApp(global_natp, (int) nap->dynlink_lazy_allocate_tdb_stack);
	  NaClStartThreadInApp(global_natp[sid],
			    (int) nap[sid]->dynlink_lazy_allocate_tdb_stack);
	}

	//mes310 to satisfy the new outofjail
	result = (void *)(* ((int *) result));

	//convert the result to a sys address
	//result = (void *) NaClUserToSys(global_natp->nap, (uintptr_t) result);

	//these are both on the stack, so we must copy them out before releasing the global
	//natp
	//usr_tdb = (void *) result[0];
	//usr_esp = (void *) result[1];
	//tdb_size = result[2];

	/*printf("usr_tdb: %p\n", usr_tdb);
	printf("usr_esp: %p\n", usr_esp);
	printf("tdb_size: %i\n", tdb_size);*/

	//release global_natp_lock
	pthread_mutex_unlock( &global_natp_locks[sid] );

	//convert the usr_tdb to a sys_tdb
	//sys_tdb = (void *) NaClUserToSys(global_natp->nap, (uintptr_t) usr_tdb);
	//sys_tdb = (void *) NaClUserToSys(nap, nap->break_addr);

	sys_tdb = global_natp[sid]->base_addr;

	natp->base_addr=sys_tdb;

	//TODO: do checks here on the values... to make sure they are legitimate
	//these are the same as NaCl does during NaClCommonSysThread_Create
	

	gs = NaClLdtAllocateByteSelector(NACL_LDT_DESCRIPTOR_DATA,
                                   0,
                                   (void *) sys_tdb,
                                   4096);


	//NaClLog(0, "NaClAppThreadAllocSegCtor: sys_tdb: %p tdb_size: %i\n", sys_tdb, tdb_size);
 	NaClLog(1, "[robusta_jvm.c: lazy_allocate_natp:] esp %p gs 0x%02x\n", result, gs);

	
	NaClAppThreadCtorNoSpawn(natp, nap[sid],
				 0, 0, (uintptr_t) result, gs);

	//get the thread num
	NaClXMutexLock(&natp->nap->threads_mu);
  	natp->thread_num = NaClAddThreadMu(natp->nap, natp);
	NaClXMutexUnlock(&natp->nap->threads_mu);

	//TODO: should we allocate a separate false_env?
	natp->false_env = global_natp[sid]->false_env;
	//mes310 also get false_vm
	natp->false_vm = global_natp[sid]->false_vm;

	natp->field_id_list=ListCreate();
	natp->method_id_list=ListCreate();

	NaClLog(1, "[robusta_jvm.c: lazy_allocate_natp:] natp->method_id_list: %p, natp->field_id_list: %p\n", natp->method_id_list, natp->field_id_list);
}

void print_natp_fields(struct NaClAppThread * natp)
{
	NaClLog(0, "---------------------------------------\n");
	NaClLog(0, "natp = 0x%08"PRIxPTR"\n", (uintptr_t) natp);

	NaClLog(0, "natp->thread_num: %i\n", natp->thread_num);
	NaClLog(0, "natp->user.cs: 0x%02x\n", natp->user.cs);
  	NaClLog(0, "natp->user.fs: 0x%02x\n", natp->user.fs);
  	NaClLog(0, "natp->user.gs: 0x%02x\n", natp->user.gs);
  	NaClLog(0, "natp->user.ss: 0x%02x\n", natp->user.ss);
	NaClLog(0, "natp->user.esp: 0x%08x\n", natp->user.esp);
	NaClLog(0, "----------------------------------------\n");
}

// a special call_in for JNI_OnLoad
// here we don't need that many args because JNI_OnLoad always only
// has two arguments
long long _Z14call_in_onloadPv(struct NaClAppThread * natp, int sid, void * sandbox_meth_addr, void * vmenv, void * jnienv, void * reserved)
{
  printf("[call_in_onload] natp: %p, sid: %d, addr: %p, vm: %p, env: %p\n", natp, sid, sandbox_meth_addr, vmenv, jnienv);
  fflush(stdout);
  struct NaClAppThread natp_restore;

  size_t size=0;

  long long int * result_addr;
  long long int result;

  //determine if lazy allocation is necessary
  if(natp->base_addr==NULL)
    {
      lazy_allocate_natp(natp, sid);
    }
	
  //print_natp_fields(global_natp);
  //print_natp_fields(local_natp);	

  //save natp to the stack as we will modify its contents with this call in
  //note that we save case n=1, even though it's never really used

  memcpy(&natp_restore, natp, sizeof(struct NaClAppThread));

  //method_dispatch returns a pointer to a long long, which will be
  //interpreted by the result handler in the JVM as the true return
  //type (which may only be one word instead of two, but that's ok)

  result_addr = (long long int *) setjmp(natp->snapshot);

  if(result_addr == 0)
    {
      uintptr_t usr_esp = NaClUserToSys(nap[sid], natp->user.esp);

      //allocate space on the native stack for all the parameters
		
      size += sizeof(void *); //reserved pointer

      size += sizeof(void *); //JavaVM pointer

      size += sizeof(void *); // the dispatch address

      size += sizeof(int *);  // a dummy return address

      size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK; //alignment check 

      // todo: should check for stack overflow
      usr_esp = usr_esp - size;
      natp->user.esp = NaClSysToUser(nap[sid], usr_esp);

      //write the parameters into the allocated space on the native stack
      //(starting with the lowest memory address first...top of stack)
      char *p = (char *) usr_esp;
		
      //write a blank return address (we will never return here)		
      BLAT(int *, 0);

      //the actual jump address for the dispatcher to call
      BLAT(void *, sandbox_meth_addr);

      //save the actual JNIEnv to natp
      /*JNIEnv * env = (JNIEnv*) jnienv;
	printf("[robusta.c: call_in:] env = %p\n", env);
	printf("[robusta.c: call_in:] (*env) = %p\n", (*env));
	printf("[robusta.c: call_in:] &env->Get... = %p\n", &(*env)->GetStringUTFChars);
	printf("[robusta.c: call_in:] env->Get... = %p\n", (*env)->GetStringUTFChars);*/
      natp->real_env = jnienv;
      natp->real_vm = vmenv;

      //TODO: eventually move this to a call within the JVM (when the thread is created?)
      //this is needless and slow because it's a global variable that everyone can share?

      initJNICheckIDs(jnienv);

      //push the false JNIEnv we fabricated during dynlink main
      //printf("[robusta.c: call_in:] false_env: %p\n", natp->false_env);
      //printf("[robusta.c: call_in:] obj: 0x%x\n", (int)mirror);


      //BLAT(void *, (void *)natp->false_env);
      BLAT(void *, (void *)natp->false_vm);

      BLAT(void *, reserved);
		
      //copy the rest of the Java parameters to the native stack
      //(they are above us on the current stack)
      //printf("Copying %i bytes from address %x to address %x\n", java_param_bytes, (int) &java_params, (int) p);
      //memcpy(p, &java_params, java_param_bytes);
      
      NaClLog(0,
	      "[robusta_jvm.c: call_in:] Jumping to dispatch %p\n",
	      nap[sid]->method_dispatch);
      //NaClSwitchToApp(natp, (int) nap->method_dispatch);
      
      //before and after are defined in include/librobusta.h
#ifdef SANDBOX_MEASUREMENT
      gettimeofday(&before, NULL);
#endif
      
      NaClStartThreadInApp(natp, (int) nap[sid]->method_dispatch);
      
      fflush(stdout);
      
    }

#ifdef SANDBOX_MEASUREMENT
  gettimeofday(&after, NULL);
  
  updateTime(jnienv, &before, &after);
#endif

  result = *(result_addr);

  //restore original natp structure
  memcpy(natp, &natp_restore, sizeof(struct NaClAppThread));

  NaClLog(0,"[robusta_jvm.c: call_in:] Returned from dispatch...returning to the JVM...\n");

  return result;
}

long long _Z7call_inPv(struct NaClAppThread * natp, int sid, int num_java_param_words, int access_flags, void * sandbox_meth_addr, void * jnienv, void * mirror, void * java_params)
{
  int java_param_bytes;
  struct NaClAppThread natp_restore;

  size_t size = 0;

  long long int * result_addr;
  long long int result;

  //determine if lazy allocation is necessary
  if(natp->base_addr == NULL)
    {
      lazy_allocate_natp(natp, sid);
    }

  //print_natp_fields(global_natp);
  //print_natp_fields(local_natp);	

  //save natp to the stack as we will modify its contents with this call in
  //note that we save case n=1, even though it's never really used

  memcpy(&natp_restore, natp, sizeof(struct NaClAppThread));

  //method_dispatch returns a pointer to a long long, which will be
  //interpreted by the result handler in the JVM as the true return
  //type (which may only be one word instead of two, but that's ok)

  result_addr = (long long int *) setjmp(natp->snapshot);
  
  if(result_addr == 0)
    {
      uintptr_t usr_esp = NaClUserToSys(nap[sid], natp->user.esp);

      //allocate space on the native stack for all the parameters

      //in order to know the actual parameter count, check the access flags
      //If the method is static, then the parameter count does not include
      //the calling class reference
      //if the method is not static, then the parameter count *includes* the
      //calling object reference
      //printf("The access flags are: %x\n", access_flags);
      if(access_flags & JVM_ACC_STATIC)
	{
	  java_param_bytes= WORD_SIZE * num_java_param_words;
	}
      else
	{
	  java_param_bytes= WORD_SIZE * (num_java_param_words-1); 
	}	
		
      NaClLog(0, "[robusta_jvm.c: call_in:] Pushing %i bytes to the native stack.\n", java_param_bytes);
		
      size += sizeof(void *);//the mirror (calling) object (class if static)

      size += sizeof(void *); //JNIEnv pointer

      size += sizeof(void *); // the dispatch address

      size += sizeof(int *); // a dummy return address

      size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK; //alignment check 

      // todo: should check for stack overflow
      usr_esp = usr_esp - size;
      natp->user.esp = NaClSysToUser(nap[sid], usr_esp);

      //write the parameters into the allocated space on the native stack
      //(starting with the lowest memory address first...top of stack)
      char *p = (char *) usr_esp;
		
      //write a blank return address (we will never return here)		
      BLAT(int *, 0);

      //the actual jump address for the dispatcher to call
      BLAT(void *, sandbox_meth_addr);

      //save the actual JNIEnv to natp
      /*JNIEnv * env = (JNIEnv*) jnienv;
	printf("[robusta.c: call_in:] env = %p\n", env);
	printf("[robusta.c: call_in:] (*env) = %p\n", (*env));
	printf("[robusta.c: call_in:] &env->Get... = %p\n", &(*env)->GetStringUTFChars);
	printf("[robusta.c: call_in:] env->Get... = %p\n", (*env)->GetStringUTFChars);*/
      natp->real_env = jnienv;

      //TODO: eventually move this to a call within the JVM (when the thread is created?)
      //this is needless and slow because it's a global variable that everyone can share?

      initJNICheckIDs(jnienv);

      //push the false JNIEnv we fabricated during dynlink main
      //printf("[robusta.c: call_in:] false_env: %p\n", natp->false_env);
      //printf("[robusta.c: call_in:] obj: 0x%x\n", (int)mirror);

      BLAT(void *, (void *)natp->false_env);

      BLAT(void *, mirror);
		
      //copy the rest of the Java parameters to the native stack
      //(they are above us on the current stack)
      //printf("Copying %i bytes from address %x to address %x\n", java_param_bytes, (int) &java_params, (int) p);
      memcpy(p, &java_params, java_param_bytes);

      //print the contents of the parameters going onto the native stack
      int * temp;
      int i;
      temp = ((int *) &java_params) -1;
      NaClLog(0, "[robusta_jvm.c: call_in:] Source-----------------------\n");
      
      for(i=0; i<10; i++)
	{
	  NaClLog(0, "%i: %x\n", i, *temp);
	  temp++;
	}
      
      NaClLog(0, "[robusta_jvm.c: call_in:] Destination--------------------\n");
      temp = ((int *) p)-10;
      for(i=0; i<20; i++)
	{
	  NaClLog(0, "%i: %x\n",i, *temp);
	  temp++;
	}

      NaClLog(0,
	      "[robusta_jvm.c: call_in:] Jumping to dispatch %p\n",
	      nap[sid]->method_dispatch);
      //NaClSwitchToApp(natp, (int) nap->method_dispatch);

      //before and after are defined in include/librobusta.h
#ifdef SANDBOX_MEASUREMENT
      gettimeofday(&before, NULL);
#endif

      NaClStartThreadInApp(natp, (int) nap[sid]->method_dispatch);

      fflush(stdout);
      
    }

#ifdef SANDBOX_MEASUREMENT
  gettimeofday(&after, NULL);
  
  updateTime(jnienv, &before, &after);
#endif


  result = *(result_addr);

  //restore original natp structure
  memcpy(natp, &natp_restore, sizeof(struct NaClAppThread));

  NaClLog(0,"[robusta_jvm.c: call_in:] Returned from dispatch...returning to the JVM...\n");

  return result;
}


//mes310
//duplicate a call_in for long type
long long * _Z12call_in_longPv(struct NaClAppThread * natp, int sid, int num_java_param_words, int access_flags, void * sandbox_meth_addr, void * jnienv, void * mirror, void * java_params)
{
  int java_param_bytes;
  struct NaClAppThread natp_restore;

  size_t size = 0;

  long long int * result_addr;
  long long int result;

  //determine if lazy allocation is necessary
  if(natp->base_addr==NULL)
    {
      lazy_allocate_natp(natp, sid);
      fflush(stdout);
    }

	
  //print_natp_fields(global_natp);
  //print_natp_fields(local_natp);	

  //save natp to the stack as we will modify its contents with this call in
  //note that we save case n=1, even though it's never really used

  memcpy(&natp_restore, natp, sizeof(struct NaClAppThread));

  //method_dispatch returns a pointer to a long long, which will be
  //interpreted by the result handler in the JVM as the true return
  //type (which may only be one word instead of two, but that's ok)

  result_addr = (long long int *) setjmp(natp->snapshot);

  if(result_addr == 0)
    {
      uintptr_t usr_esp = NaClUserToSys(nap[sid], natp->user.esp);

      //allocate space on the native stack for all the parameters

      //in order to know the actual parameter count, check the access flags
      //If the method is static, then the parameter count does not include
      //the calling class reference
      //if the method is not static, then the parameter count *includes* the
      //calling object reference
      //printf("The access flags are: %x\n", access_flags);
      if(access_flags & JVM_ACC_STATIC)
	{
	  java_param_bytes= WORD_SIZE * num_java_param_words;
	}
      else
	{
	  java_param_bytes= WORD_SIZE * (num_java_param_words-1); 
	}	
		
      NaClLog(0, "[robusta_jvm.c: call_in:] Pushing %i bytes to the native stack.\n", java_param_bytes);
		
      size += sizeof(void *);//the mirror (calling) object (class if static)

      size += sizeof(void *); //JNIEnv pointer

      size += sizeof(void *); // the dispatch address

      size += sizeof(int *); // a dummy return address

      size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK; //alignment check 

      // todo: should check for stack overflow
      usr_esp = usr_esp - size;
      natp->user.esp = NaClSysToUser(nap[sid], usr_esp);

      //write the parameters into the allocated space on the native stack
      //(starting with the lowest memory address first...top of stack)
      char *p = (char *) usr_esp;
		
      //write a blank return address (we will never return here)		
      BLAT(int *, 0);

      //the actual jump address for the dispatcher to call
      BLAT(void *, sandbox_meth_addr);

      //save the actual JNIEnv to natp
      /*JNIEnv * env = (JNIEnv*) jnienv;
	printf("[robusta.c: call_in:] env = %p\n", env);
	printf("[robusta.c: call_in:] (*env) = %p\n", (*env));
	printf("[robusta.c: call_in:] &env->Get... = %p\n", &(*env)->GetStringUTFChars);
	printf("[robusta.c: call_in:] env->Get... = %p\n", (*env)->GetStringUTFChars);*/
      natp->real_env = jnienv;

      //TODO: eventually move this to a call within the JVM (when the thread is created?)
      //this is needless and slow because it's a global variable that everyone can share?
      initJNICheckIDs(jnienv);

      //push the false JNIEnv we fabricated during dynlink main
      //printf("[robusta.c: call_in:] false_env: %p\n", natp->false_env);
      //printf("[robusta.c: call_in:] obj: 0x%x\n", (int)mirror);


      BLAT(void *, (void *)natp->false_env);

      BLAT(void *, mirror);
		
      //copy the rest of the Java parameters to the native stack
      //(they are above us on the current stack)
      //printf("Copying %i bytes from address %x to address %x\n", java_param_bytes, (int) &java_params, (int) p);
      memcpy(p, &java_params, java_param_bytes);

      //print the contents of the parameters going onto the native stack
      int * temp;
      int i;
      temp = ((int *) &java_params) -1;
      NaClLog(0, "[robusta_jvm.c: call_in:] Source-----------------------\n");
      
      for(i=0; i<10; i++)
	{
	  NaClLog(0, "%i: %x\n", i, *temp);
	  temp++;
	}
      
      NaClLog(0, "[robusta_jvm.c: call_in:] Destination--------------------\n");
      temp = ((int *) p)-10;
      for(i=0; i<20; i++)
	{
	  NaClLog(0, "%i: %x\n",i, *temp);
	  temp++;
	}
      
      NaClLog(0,
	      "[robusta_jvm.c: call_in:] Jumping to dispatch %p\n",
	      nap[sid]->method_dispatch);
      
      //NaClSwitchToApp(natp, (int) nap->method_dispatch);
      
      //before and after are defined in include/librobusta.h
#ifdef SANDBOX_MEASUREMENT
      gettimeofday(&before, NULL);
#endif
      
      NaClStartThreadInApp(natp, (int) nap[sid]->method_dispatch);
      
      fflush(stdout);
      
    }

#ifdef SANDBOX_MEASUREMENT
  gettimeofday(&after, NULL);
  
  updateTime(jnienv, &before, &after);
#endif


  result = *(result_addr);

  //restore original natp structure
  memcpy(natp, &natp_restore, sizeof(struct NaClAppThread));

  NaClLog(0,"[robusta_jvm.c: call_in:] Returned from dispatch...returning to the JVM...\n");

  return result_addr;
}


//mes310
//duplicate a call_in for floating point returns (without float arguments)
double* _Z13call_in_floatPv(struct NaClAppThread * natp, int sid, int num_java_param_words, int access_flags, void * sandbox_meth_addr, void * jnienv, void * mirror, void * java_params)
{
  int java_param_bytes;
  struct NaClAppThread natp_restore;

  size_t size=0;

  double * result_addr;
  double result;

  //determine if lazy allocation is necessary
  if(natp->base_addr==NULL)
    {
      lazy_allocate_natp(natp, sid);
      fflush(stdout);
    }

	
  //print_natp_fields(global_natp);
  //print_natp_fields(local_natp);	

  //save natp to the stack as we will modify its contents with this call in
  //note that we save case n=1, even though it's never really used

  memcpy(&natp_restore, natp, sizeof(struct NaClAppThread));

  //method_dispatch returns a pointer to a long long, which will be
  //interpreted by the result handler in the JVM as the true return
  //type (which may only be one word instead of two, but that's ok)

  result_addr = (double *) setjmp(natp->snapshot);

  if(result_addr == 0)
    {
      uintptr_t usr_esp = NaClUserToSys(nap[sid], natp->user.esp);

      //allocate space on the native stack for all the parameters

      //in order to know the actual parameter count, check the access flags
      //If the method is static, then the parameter count does not include
      //the calling class reference
      //if the method is not static, then the parameter count *includes* the
      //calling object reference
      //printf("The access flags are: %x\n", access_flags);
      if(access_flags & JVM_ACC_STATIC)
	{
	  java_param_bytes= WORD_SIZE * num_java_param_words;
	}
      else
	{
	  java_param_bytes= WORD_SIZE * (num_java_param_words-1); 
	}	
		
      NaClLog(0, "[robusta_jvm.c: call_in_float] Pushing %i bytes to the native stack.\n", java_param_bytes);
		
      size += sizeof(void *);//the mirror (calling) object (class if static)

      size += sizeof(void *); //JNIEnv pointer

      size += sizeof(void *); // the dispatch address

      size += sizeof(int *); // a dummy return address

      size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK; //alignment check 

      // todo: should check for stack overflow
      usr_esp = usr_esp - size;
      natp->user.esp = NaClSysToUser(nap[sid], usr_esp);

      //write the parameters into the allocated space on the native stack
      //(starting with the lowest memory address first...top of stack)
      char *p = (char *) usr_esp;
		
      //write a blank return address (we will never return here)		
      BLAT(int *, 0);

      //the actual jump address for the dispatcher to call
      BLAT(void *, sandbox_meth_addr);

      //save the actual JNIEnv to natp
      /*JNIEnv * env = (JNIEnv*) jnienv;
	printf("[robusta.c: call_in:] env = %p\n", env);
	printf("[robusta.c: call_in:] (*env) = %p\n", (*env));
	printf("[robusta.c: call_in:] &env->Get... = %p\n", &(*env)->GetStringUTFChars);
	printf("[robusta.c: call_in:] env->Get... = %p\n", (*env)->GetStringUTFChars);*/
      natp->real_env = jnienv;

      //TODO: eventually move this to a call within the JVM (when the thread is created?)
      //this is needless and slow because it's a global variable that everyone can share?
      initJNICheckIDs(jnienv);

      //push the false JNIEnv we fabricated during dynlink main
      //printf("[robusta_jvm.c: call_in_float] false_env: %p\n", natp->false_env);
      //printf("[robusta.c: call_in:] obj: 0x%x\n", (int)mirror);


      BLAT(void *, (void *)natp->false_env);

      BLAT(void *, mirror);
		
      //copy the rest of the Java parameters to the native stack
      //(they are above us on the current stack)
      //printf("[robusta_jvm.c: call_in_float] Copying %i bytes from address %x to address %x\n", java_param_bytes, (int) &java_params, (int) p);
      memcpy(p, &java_params, java_param_bytes);

      //mes310
      //print the contents of the parameters going onto the native stack and also adjust the stack if there are
      //float arguments
      int * temp;
      int i;
      temp = ((int *) &java_params) -1;
      NaClLog(0, "[robusta_jvm.c: call_in_float] Source-----------------------\n");
      
      for(i=0; i<10; i++)
	{
	  NaClLog(0, "%i: %x\n", i, *temp);
	  temp++;
	}
      
      NaClLog(0, "[robusta_jvm.c: call_in_float] Destination--------------------\n");
      temp = ((int *) p)-10;
      for(i=0; i<20; i++)
	{
	  NaClLog(0, "%i: %x\n",i, *temp);
	  temp++;
	}
      
      NaClLog(0,
	      "[robusta_jvm.c: call_in_float] Jumping to dispatch %p\n",
	      nap[sid]->method_dispatch_float);
      
      //NaClSwitchToApp(natp, (int) nap->method_dispatch);
      
      //before and after are defined in include/librobusta.h
#ifdef SANDBOX_MEASUREMENT
      gettimeofday(&before, NULL);
#endif
      
      NaClStartThreadInApp(natp, (int) nap[sid]->method_dispatch_float);
      
      fflush(stdout);
      
    }

#ifdef SANDBOX_MEASUREMENT
  gettimeofday(&after, NULL);
  
  updateTime(jnienv, &before, &after);
#endif


  result = *(result_addr);

  //restore original natp structure
  memcpy(natp, &natp_restore, sizeof(struct NaClAppThread));

  NaClLog(0,"[robusta_jvm.c: call_in:] Returned from dispatch...returning to the JVM...\n");

  return result_addr;
}


//mes310
//duplicate a call_in for floating point returns (with float arguments)
double* _Z17call_in_float_argPv(int * mark, struct NaClAppThread * natp, int sid, int num_java_param_words, int access_flags, void * sandbox_meth_addr, void * jnienv, void * mirror, void * java_params)
{
  int java_param_bytes;
  struct NaClAppThread natp_restore;

  size_t size=0;

  double * result_addr;
  double result;

  //determine if lazy allocation is necessary
  if(natp->base_addr==NULL)
    {
      lazy_allocate_natp(natp, sid);
      fflush(stdout);
    }

	
  //print_natp_fields(global_natp);
  //print_natp_fields(local_natp);	

  //save natp to the stack as we will modify its contents with this call in
  //note that we save case n=1, even though it's never really used

  memcpy(&natp_restore, natp, sizeof(struct NaClAppThread));

  //method_dispatch returns a pointer to a long long, which will be
  //interpreted by the result handler in the JVM as the true return
  //type (which may only be one word instead of two, but that's ok)

  result_addr = (double *) setjmp(natp->snapshot);

  if(result_addr == 0)
    {
      uintptr_t usr_esp = NaClUserToSys(nap[sid], natp->user.esp);

      //allocate space on the native stack for all the parameters

      //in order to know the actual parameter count, check the access flags
      //If the method is static, then the parameter count does not include
      //the calling class reference
      //if the method is not static, then the parameter count *includes* the
      //calling object reference
      //printf("The access flags are: %x\n", access_flags);
      if(access_flags & JVM_ACC_STATIC)
	{
	  java_param_bytes= WORD_SIZE * num_java_param_words;
	}
      else
	{
	  java_param_bytes= WORD_SIZE * (num_java_param_words-1); 
	}	
		
      NaClLog(0, "[robusta_jvm.c: call_in_float] Pushing %i bytes to the native stack.\n", java_param_bytes);
		
      size += sizeof(void *);//the mirror (calling) object (class if static)

      size += sizeof(void *); //JNIEnv pointer

      size += sizeof(void *); // the dispatch address

      size += sizeof(int *); // a dummy return address

      size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK; //alignment check 

      // todo: should check for stack overflow
      usr_esp = usr_esp - size;
      natp->user.esp = NaClSysToUser(nap[sid], usr_esp);

      //write the parameters into the allocated space on the native stack
      //(starting with the lowest memory address first...top of stack)
      char *p = (char *) usr_esp;
		
      //write a blank return address (we will never return here)		
      BLAT(int *, 0);

      //the actual jump address for the dispatcher to call
      BLAT(void *, sandbox_meth_addr);

      //save the actual JNIEnv to natp
      /*JNIEnv * env = (JNIEnv*) jnienv;
	printf("[robusta.c: call_in:] env = %p\n", env);
	printf("[robusta.c: call_in:] (*env) = %p\n", (*env));
	printf("[robusta.c: call_in:] &env->Get... = %p\n", &(*env)->GetStringUTFChars);
	printf("[robusta.c: call_in:] env->Get... = %p\n", (*env)->GetStringUTFChars);*/
      natp->real_env = jnienv;

      //TODO: eventually move this to a call within the JVM (when the thread is created?)
      //this is needless and slow because it's a global variable that everyone can share?
      initJNICheckIDs(jnienv);

      //push the false JNIEnv we fabricated during dynlink main
      //printf("[robusta_jvm.c: call_in_float] false_env: %p\n", natp->false_env);
      //printf("[robusta.c: call_in:] obj: 0x%x\n", (int)mirror);


      BLAT(void *, (void *)natp->false_env);

      BLAT(void *, mirror);
		
      //copy the rest of the Java parameters to the native stack
      //(they are above us on the current stack)
      //printf("[robusta_jvm.c: call_in_float] Copying %i bytes from address %x to address %x\n", java_param_bytes, (int) &java_params, (int) p);

      int * cursor = (int *) &java_params;
      int * cursor2;
      //printf("cursor: %d\n", *cursor);
      //Adjust the stack before copying
      //printf("mark: %p\n", mark);
      while (* mark != -1) {
	int m = * mark;
	//printf("*mark: %d\n", m);
	int i;
	for (i = 0; i < m; i++)
	  cursor++;
	//printf("cursor after moving: %d\n", *cursor);
	int times = java_param_bytes - m - 1;
	for (i = 0; i < times; i++) {
	  cursor2 = cursor;
	  float ffff;
	  if (i == 0) {
	    double tttt = * (double *) cursor2;
	    ffff = tttt;
	    //printf("The number in double: %f, in float: %f\n", tttt, ffff);
	  }
	  *cursor2 = *(++cursor);
	  if (i == 0) {
	    * (float *) cursor2 = ffff;
	  }
	}
	mark++;
	//printf("mark: %p\n", mark);
      }

      memcpy(p, &java_params, java_param_bytes);

      //mes310
      //print the contents of the parameters going onto the native stack
      int * temp;
      int i;
      temp = ((int *) &java_params) -1;
      NaClLog(0, "[robusta_jvm.c: call_in_float] Source-----------------------\n");
      //printf("[robusta_jvm.c: call_in_float] Source-----------------------\n");
      
      for(i=0; i<10; i++)
	{
	  NaClLog(0, "%i: %x\n", i, *temp);
	  //printf("%i: %p: %x\n", i, temp, *temp);
	  temp++;
	}
      
      NaClLog(0, "[robusta_jvm.c: call_in_float] Destination--------------------\n");
      //printf("[robusta_jvm.c: call_in_float] Destination--------------------\n");
      temp = ((int *) p)-10;
      for(i=0; i<20; i++)
	{
	  NaClLog(0, "%i: %x\n",i, *temp);
	  //printf("%i: %p: %x\n",i, temp, *temp);
	  temp++;
	}
      
      NaClLog(0,
	      "[robusta_jvm.c: call_in_float] Jumping to dispatch %p\n",
	      nap[sid]->method_dispatch_float);
      
      //NaClSwitchToApp(natp, (int) nap->method_dispatch);
      
      //before and after are defined in include/librobusta.h
#ifdef SANDBOX_MEASUREMENT
      gettimeofday(&before, NULL);
#endif
      
      NaClStartThreadInApp(natp, (int) nap[sid]->method_dispatch_float);
      
    }

#ifdef SANDBOX_MEASUREMENT
  gettimeofday(&after, NULL);
  
  updateTime(jnienv, &before, &after);
#endif

  result = *(result_addr);

  //restore original natp structure
  memcpy(natp, &natp_restore, sizeof(struct NaClAppThread));

  NaClLog(0,"[robusta_jvm.c: call_in:] Returned from dispatch...returning to the JVM...\n");

  return result_addr;
}



//mes310
//duplicate a call_in for normal returns (with float arguments)
long long _Z24call_in_normal_float_argPv(int * mark, struct NaClAppThread * natp, int sid, int num_java_param_words, int access_flags, void * sandbox_meth_addr, void * jnienv, void * mirror, void * java_params)
{
  int java_param_bytes;
  struct NaClAppThread natp_restore;

  size_t size=0;

  long long * result_addr;
  long long result;

  //determine if lazy allocation is necessary
  if(natp->base_addr==NULL)
    {
      lazy_allocate_natp(natp, sid);
      fflush(stdout);
    }

	
  //print_natp_fields(global_natp);
  //print_natp_fields(local_natp);	

  //save natp to the stack as we will modify its contents with this call in
  //note that we save case n=1, even though it's never really used

  memcpy(&natp_restore, natp, sizeof(struct NaClAppThread));

  //method_dispatch returns a pointer to a long long, which will be
  //interpreted by the result handler in the JVM as the true return
  //type (which may only be one word instead of two, but that's ok)

  result_addr = (long long *) setjmp(natp->snapshot);

  if(result_addr == 0)
    {
      uintptr_t usr_esp = NaClUserToSys(nap[sid], natp->user.esp);

      //allocate space on the native stack for all the parameters

      //in order to know the actual parameter count, check the access flags
      //If the method is static, then the parameter count does not include
      //the calling class reference
      //if the method is not static, then the parameter count *includes* the
      //calling object reference
      //printf("The access flags are: %x\n", access_flags);
      if(access_flags & JVM_ACC_STATIC)
	{
	  java_param_bytes= WORD_SIZE * num_java_param_words;
	}
      else
	{
	  java_param_bytes= WORD_SIZE * (num_java_param_words-1); 
	}	
		
      NaClLog(0, "[robusta_jvm.c: call_in_float] Pushing %i bytes to the native stack.\n", java_param_bytes);
		
      size += sizeof(void *);//the mirror (calling) object (class if static)

      size += sizeof(void *); //JNIEnv pointer

      size += sizeof(void *); // the dispatch address

      size += sizeof(int *); // a dummy return address

      size = (size + PTR_ALIGN_MASK) & ~PTR_ALIGN_MASK; //alignment check 

      // todo: should check for stack overflow
      usr_esp = usr_esp - size;
      natp->user.esp = NaClSysToUser(nap[sid], usr_esp);

      //write the parameters into the allocated space on the native stack
      //(starting with the lowest memory address first...top of stack)
      char *p = (char *) usr_esp;
		
      //write a blank return address (we will never return here)		
      BLAT(int *, 0);

      //the actual jump address for the dispatcher to call
      BLAT(void *, sandbox_meth_addr);

      //save the actual JNIEnv to natp
      /*JNIEnv * env = (JNIEnv*) jnienv;
	printf("[robusta.c: call_in:] env = %p\n", env);
	printf("[robusta.c: call_in:] (*env) = %p\n", (*env));
	printf("[robusta.c: call_in:] &env->Get... = %p\n", &(*env)->GetStringUTFChars);
	printf("[robusta.c: call_in:] env->Get... = %p\n", (*env)->GetStringUTFChars);*/
      natp->real_env = jnienv;

      //TODO: eventually move this to a call within the JVM (when the thread is created?)
      //this is needless and slow because it's a global variable that everyone can share?
      initJNICheckIDs(jnienv);

      //push the false JNIEnv we fabricated during dynlink main
      //printf("[robusta_jvm.c: call_in_float] false_env: %p\n", natp->false_env);
      //printf("[robusta.c: call_in:] obj: 0x%x\n", (int)mirror);


      BLAT(void *, (void *)natp->false_env);

      BLAT(void *, mirror);
		
      //copy the rest of the Java parameters to the native stack
      //(they are above us on the current stack)
      //printf("[robusta_jvm.c: call_in_float] Copying %i bytes from address %x to address %x\n", java_param_bytes, (int) &java_params, (int) p);

      int * cursor = (int *) &java_params;
      int * cursor2;
      //printf("cursor: %d\n", *cursor);
      //Adjust the stack before copying
      //printf("mark: %p\n", mark);
      while (* mark != -1) {
	int m = * mark;
	//printf("*mark: %d\n", m);
	int i;
	for (i = 0; i < m; i++)
	  cursor++;
	//printf("cursor after moving: %d\n", *cursor);
	int times = java_param_bytes - m - 1;
	for (i = 0; i < times; i++) {
	  cursor2 = cursor;
	  float ffff;
	  if (i == 0) {
	    double tttt = * (double *) cursor2;
	    ffff = tttt;
	    //printf("The number in double: %f, in float: %f\n", tttt, ffff);
	  }
	  *cursor2 = *(++cursor);
	  if (i == 0) {
	    * (float *) cursor2 = ffff;
	  }
	}
	mark++;
	//printf("mark: %p\n", mark);
      }

      memcpy(p, &java_params, java_param_bytes);

      //mes310
      //print the contents of the parameters going onto the native stack
      int * temp;
      int i;
      temp = ((int *) &java_params) -1;
      NaClLog(0, "[robusta_jvm.c: call_in_float] Source-----------------------\n");
      //printf("[robusta_jvm.c: call_in_float] Source-----------------------\n");
      
      for(i=0; i<10; i++)
	{
	  NaClLog(0, "%i: %x\n", i, *temp);
	  //printf("%i: %p: %x\n", i, temp, *temp);
	  temp++;
	}
      
      NaClLog(0, "[robusta_jvm.c: call_in_float] Destination--------------------\n");
      //printf("[robusta_jvm.c: call_in_float] Destination--------------------\n");
      temp = ((int *) p)-10;
      for(i=0; i<20; i++)
	{
	  NaClLog(0, "%i: %x\n",i, *temp);
	  //printf("%i: %p: %x\n",i, temp, *temp);
	  temp++;
	}
      
      NaClLog(0,
	      "[robusta_jvm.c: call_in_float] Jumping to dispatch %p\n",
	      nap[sid]->method_dispatch_float);
      
      //NaClSwitchToApp(natp, (int) nap->method_dispatch);
      
      //before and after are defined in include/librobusta.h
#ifdef SANDBOX_MEASUREMENT
      gettimeofday(&before, NULL);
#endif
      
      NaClStartThreadInApp(natp, (int) nap[sid]->method_dispatch);
      
    }

#ifdef SANDBOX_MEASUREMENT
  gettimeofday(&after, NULL);
  
  updateTime(jnienv, &before, &after);
#endif

  result = *(result_addr);

  //restore original natp structure
  memcpy(natp, &natp_restore, sizeof(struct NaClAppThread));

  NaClLog(0,"[robusta_jvm.c: call_in:] Returned from dispatch...returning to the JVM...\n");

  return result;
}



//this hook is called during JavaThread creation time
NATP_PTR _Z17robusta_init_natpv()
{
	struct NaClAppThread * natp = (NATP_PTR) malloc(sizeof(struct NaClAppThread));

	//we use this as the signal that we need to lazy initialize the natp
	natp->base_addr=NULL;

	// printf("[robusta_jvm.c: init_natp] natp: %p, esp: %p\n", natp, natp->user.esp);

	return natp;
}
