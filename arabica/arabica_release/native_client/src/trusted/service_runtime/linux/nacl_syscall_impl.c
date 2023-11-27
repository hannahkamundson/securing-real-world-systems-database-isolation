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


#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <setjmp.h>
#include <jni.h>
#include <jvmti.h>
#include <classfile_constants.h>

#include "native_client/src/include/portability.h"

#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_conn_cap.h"
#include "native_client/src/trusted/desc/nacl_desc_imc.h"
#include "native_client/src/trusted/desc/nacl_desc_imc_bound_desc.h"
#include "native_client/src/trusted/desc/nacl_desc_imc_shm.h"
#include "native_client/src/trusted/desc/nacl_desc_io.h"

#include "native_client/src/trusted/platform/nacl_log.h"
#include "native_client/src/trusted/platform/nacl_sync_checked.h"
#include "native_client/src/trusted/platform/nacl_time.h"

#include "native_client/src/trusted/service_runtime/nacl_app_thread.h"
#include "native_client/src/trusted/service_runtime/nacl_closure.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"
#include "native_client/src/trusted/service_runtime/nacl_globals.h"
#include "native_client/src/trusted/service_runtime/nacl_memory_object.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_handlers.h"
#include "native_client/src/trusted/service_runtime/nacl_syscall_common.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_memory.h"

#include "native_client/src/trusted/service_runtime/include/bits/mman.h"
#include "native_client/src/trusted/service_runtime/include/bits/nacl_syscalls.h"
#include "native_client/src/trusted/service_runtime/include/machine/_types.h"
#include "native_client/src/trusted/service_runtime/include/sys/errno.h"
#include "native_client/src/trusted/service_runtime/include/sys/fcntl.h"
#include "native_client/src/trusted/service_runtime/include/sys/nacl_imc_api.h"
#include "native_client/src/trusted/service_runtime/include/sys/stat.h"
#include "native_client/src/trusted/service_runtime/include/sys/time.h"
#include "native_client/src/trusted/service_runtime/include/sys/unistd.h"

#include "native_client/src/trusted/service_runtime/linux/nacl_syscall_inl.h"

// gtan
#include "native_client/src/trusted/service_runtime/nacl_switch_to_app.h"

#if defined(HAVE_SDL)
# include "native_client/src/trusted/service_runtime/nacl_bottom_half.h"
#endif

//mes310 add necessary header files
#include <pwd.h>
#include <sys/utsname.h>
#include <dirent.h>
#include "include/librobusta.h"
#include "librobusta/robusta_jni_checks.h"
#include "librobusta/jinn/c2j_proxy.h"

#include "jvm.h"
#include "jni_util.h"

struct NaClSyscallTableEntry nacl_syscall[NACL_MAX_SYSCALLS] = {{0}};

static int32_t NotImplementedDecoder(struct NaClAppThread *natp) {
  return -NACL_ABI_ENOSYS;
}

//used for longjmp
extern jmp_buf snapshot;

static void NaClAddSyscall(int num, int32_t (*fn)(struct NaClAppThread *)) {
  if (nacl_syscall[num].handler != &NotImplementedDecoder) {
    NaClLog(LOG_FATAL, "Duplicate syscall number %d\n", num);
  }
  nacl_syscall[num].handler = fn;
}

/* ====================================================================== */

int32_t NaClSysNull(struct NaClAppThread *natp) {
  return 0;
}

int32_t NaClSysOpen(struct NaClAppThread  *natp,
                    char                  *pathname,
                    int                   flags,
                    int                   mode) {
  return NaClCommonSysOpen(natp, pathname, flags, mode);
}

int32_t NaClSysClose(struct NaClAppThread *natp,
                     int                  d) {
  return NaClCommonSysClose(natp, d);
}

int32_t NaClSysRead(struct NaClAppThread  *natp,
                    int                   d,
                    void                  *buf,
                    size_t                count) {
  return NaClCommonSysRead(natp, d, buf, count);
}

int32_t NaClSysWrite(struct NaClAppThread *natp,
                     int                  d,
                     void                 *buf,
                     size_t               count) {
  return NaClCommonSysWrite(natp, d, buf, count);
}

/* Warning: sizeof(nacl_abi_off_t)!=sizeof(off_t) on OSX */
int32_t NaClSysLseek(struct NaClAppThread *natp,
                     int                  d,
                     nacl_abi_off_t       offset,
                     int                  whence) {
  return NaClCommonSysLseek(natp, d, (off_t)offset, whence);
}

int32_t NaClSysIoctl(struct NaClAppThread *natp,
                     int                  d,
                     int                  request,
                     void                 *arg) {
  return NaClCommonSysIoctl(natp, d, request, arg);
}

int32_t NaClSysFstat(struct NaClAppThread *natp,
                     int                  d,
                     struct nacl_abi_stat *nasp) {
  return NaClCommonSysFstat(natp, d, nasp);
}

int32_t NaClSysStat(struct NaClAppThread *natp,
                    const char           *path,
                    struct nacl_abi_stat *nasp) {
  return NaClCommonSysStat(natp, path, nasp);
}

int32_t NaClSysGetdents(struct NaClAppThread  *natp,
                        int                   d,
                        void                  *buf,
                        size_t                count) {
  return NaClCommonSysGetdents(natp, d, buf, count);
}

int32_t NaClSysSysbrk(struct NaClAppThread  *natp,
                      void                  *new_break) {
  return NaClSetBreak(natp, (uintptr_t) new_break);
}

//mes310
//add a parameter to accomodate floating point method dispatch
int32_t NaClSysInitDynLinkWrappers(struct NaClAppThread * natp, void * dlopen, void * dlsym, void * method_dispatch, void * method_dispatch_float, void * dynlink_lazy_allocate_tdb_stack, void * safeExit)
{
	NaClLog(2, "Initializing wrapper addresses...\n dlOpen is at: %p dlSym is at: %p method_dispatch is at: %p \n", dlopen, dlsym, method_dispatch);
	natp->nap->dlOpenWrap=dlopen;
	natp->nap->dlSymWrap=dlsym;
	natp->nap->method_dispatch=method_dispatch;
	natp->nap->method_dispatch_float = method_dispatch_float; 
	natp->nap->dynlink_lazy_allocate_tdb_stack=dynlink_lazy_allocate_tdb_stack;
	natp->nap->safeExitWrap=safeExit;
	
	return 0;
}

//jjs204
int32_t NaClSysOutOfJail(struct NaClAppThread *natp, int fnum, int * retval)
{
    NaClLog(0, "[nacl_syscall_impl.c: SysOutOfJail:] Getting out of the sandbox, fnum=%i \n", fnum);
    //another pointer for JavaVM
    int * retval2 = NULL;
    switch (fnum) {
    case 1:
      //dlopenCont(natp);
      break;
    case 2:
      //dlsymCont(natp, retval);
      break;
    case 3:
      //callHelloCont(natp, (void (*)(void))(retval));
      break;
    case 5:
      //the return type is not really a long long, so just return an int
      //for simplicities sake.

      //another pointer for JavaVM
      retval2 = (int *) ((int) retval - 4);
      
      //convert retvals to system addresses
      retval = (int *) NaClUserToSys(natp->nap, (uintptr_t) retval);
      retval2 = (int *) NaClUserToSys(natp->nap, (uintptr_t) retval2);

      //longjmp(natp->snapshot, *(retval));
      longjmp(natp->snapshot, (int) retval);

      //never returns

      break;
    case 6:
      retval = (int *) NaClUserToSys(natp->nap, (uintptr_t) retval);

      //return the address of the return value (which sits on the system runtime stack)
      //it should be ok to use stack memory in this way, since the system runtime stack
      //is per-NaClAppThread
      //although this technique doesn't really follow traditional conventions.
      //We need to use the address of retval because the return type of longjmp is not 64 bits
      //int temp = 5;
      //retval = &temp;
      longjmp(natp->snapshot, (int) retval);

      //never returns
      break;

    default:
      _exit(0);
    }

    return 0;
}

int32_t NaClSysJNIEscape(struct NaClAppThread *natp, int fnum)
{
	//NOTE: currently, this syscall is NOT USED

	return 0;
}

int32_t NaClSysRobustaSleep(struct NaClAppThread * natp, int seconds)
{
	printf("n_s_i.c sleeping...");
	sleep(seconds);

	return 0;
}

//-------------------------------------------------------------------------------------------------
//mes310
//JavaVM Call Functions
int32_t NaClSysVM_Call_GetEnv(struct NaClAppThread * natp, JavaVM * false_vm, void ** penv, jint version)
{
  printf("[nacl: getenv] penv: %p\n", penv);
  fflush(stdout);
  // penv is usr addr, so first convert it to a sys addr
  void ** sys_penv = (void **) NaClUserToSys(natp->nap, (uintptr_t) penv);

  // here we really want the value that penv points to be the
  // false_env...
  *sys_penv = natp->false_env;

  // return JNI_OK
  return 0;
}

//-------------------------------------------------------------------------------------------------
//jjs204
//JNI Call Functions

//mes310 disable runtime type checking
//#ifdef RUNTIME_TYPE_CHECKING
//#undef RUNTIME_TYPE_CHECKING

// mes310
// we use a global indicating bit to decide whether to go through Jinn
// or not. Jinn's c2j proxies rely on JVMTI's init callback. We cannot
// use them until the event is sent (and bit is set in the event
// callback)

// 1
int32_t NaClSysJNI_Call_GetVersion(struct NaClAppThread *natp, JNIEnv * false_env)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetVersion(env);
  else
    return (int32_t) (*env)->GetVersion(env);
}

int32_t NaClSysJNI_Call_DefineClass(struct NaClAppThread *natp, JNIEnv *false_env, const char *name, jobject loader, const jbyte *buf, jsize bufLen)
{
  JNIEnv *env = natp->real_env;

  // name and buf are usr addr, so first convert it to a sys addr
  char *sys_name = (char *) NaClUserToSys(natp->nap, (uintptr_t) name);
  jbyte *sys_buf = (jbyte *) NaClUserToSys(natp->nap, (uintptr_t) buf);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_DefineClass(env, sys_name, loader, sys_buf, bufLen);
  else
    return (int32_t) (*env)->DefineClass(env, sys_name, loader, sys_buf, bufLen);
}

int32_t NaClSysJNI_Call_FindClass(struct NaClAppThread *natp, JNIEnv *false_env, const char *usr_name)
{
  JNIEnv * env = natp->real_env;

  //convert the name buffer addr to a system addr
  char * sys_name = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_name);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_FindClass(env, sys_name);
  else
    return (int32_t) (*env)->FindClass(env, sys_name);
}

int32_t NaClSysJNI_Call_FromReflectedMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject method)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_FromReflectedMethod(env, method);
  else
    return (int32_t) (*env)->FromReflectedMethod(env, method);
}

int32_t NaClSysJNI_Call_FromReflectedField(struct NaClAppThread *natp, JNIEnv *false_env, jobject field)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_FromReflectedField(env, field);
  else
    return (int32_t) (*env)->FromReflectedField(env, field);
}

// 6
int32_t NaClSysJNI_Call_ToReflectedMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass cls, jmethodID methodID, jboolean isStatic)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_ToReflectedMethod(env, cls, methodID, isStatic);
  else
    return (int32_t) (*env)->ToReflectedMethod(env, cls, methodID, isStatic);
}

int32_t NaClSysJNI_Call_GetSuperclass(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetSuperclass(env, clazz);
  else
    return (int32_t) (*env)->GetSuperclass(env, clazz);
}

int32_t NaClSysJNI_Call_IsAssignableFrom(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazza, jclass clazzb)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_IsAssignableFrom(env, clazza, clazzb);
  else
    return (int32_t) (*env)->IsAssignableFrom(env, clazza, clazzb);
}

int32_t NaClSysJNI_Call_ToReflectedField(struct NaClAppThread *natp, JNIEnv *false_env, jclass cls, jfieldID fieldID, jboolean isStatic)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_ToReflectedField(env, cls, fieldID, isStatic);
  else
    return (int32_t) (*env)->ToReflectedField(env, cls, fieldID, isStatic);
}

int32_t NaClSysJNI_Call_Throw(struct NaClAppThread *natp, JNIEnv *false_env, jthrowable obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_Throw(env, obj);
  else
    return (int32_t) (*env)->Throw(env, obj);
}

// 11
int32_t NaClSysJNI_Call_ThrowNew(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, const char *usr_message)
{
  JNIEnv * env = natp->real_env;

  char * sys_message = NULL;
  if (usr_message != NULL)
    sys_message = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_message);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_ThrowNew(env, clazz, sys_message);
  else
    return (int32_t) (*env)->ThrowNew(env, clazz, sys_message);
}

int32_t NaClSysJNI_Call_ExceptionOccurred(struct NaClAppThread *natp, JNIEnv *false_env)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_ExceptionOccurred(env);
  else
    return (int32_t) (*env)->ExceptionOccurred(env);
}

int32_t NaClSysJNI_Call_ExceptionDescribe(struct NaClAppThread *natp, JNIEnv *false_env)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_ExceptionDescribe(env);
  else
    (*env)->ExceptionDescribe(env);

  return 0;
}

int32_t NaClSysJNI_Call_ExceptionClear(struct NaClAppThread *natp, JNIEnv *false_env)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_ExceptionClear(env);
  else
    (*env)->ExceptionClear(env);
	
  return 0;
}

int32_t NaClSysJNI_Call_FatalError(struct NaClAppThread *natp, JNIEnv *false_env, const char *msg)
{
  JNIEnv * env = natp->real_env;

  // msg is usr addr, so first convert it to a sys addr
  char * sys_msg = (char *) NaClUserToSys(natp->nap, (uintptr_t) msg);

  if (bda_init_finish == 1)
    bda_c2j_proxy_FatalError(env, sys_msg);
  else
    (*env)->FatalError(env, sys_msg);

  return 0;
}

// 16
int32_t NaClSysJNI_Call_PushLocalFrame(struct NaClAppThread *natp, JNIEnv *false_env, jint capacity)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_PushLocalFrame(env, capacity);
  else
    return (int32_t) (*env)->PushLocalFrame(env, capacity);
}

int32_t NaClSysJNI_Call_PopLocalFrame(struct NaClAppThread *natp, JNIEnv *false_env, jobject result)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_PopLocalFrame(env, result);
  else
    return (int32_t) (*env)->PopLocalFrame(env, result);
}

int32_t NaClSysJNI_Call_NewGlobalRef(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewGlobalRef(env, obj);
  else
    return (int32_t) (*env)->NewGlobalRef(env, obj);
}

int32_t NaClSysJNI_Call_DeleteGlobalRef(struct NaClAppThread *natp, JNIEnv *false_env, jobject gref)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_DeleteGlobalRef(env, gref);
  else
    (*env)->DeleteGlobalRef(env, gref);

  return 0;
}

int32_t NaClSysJNI_Call_DeleteLocalRef(struct NaClAppThread *natp, JNIEnv *false_env, jobject localRef)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_DeleteLocalRef(env, localRef);
  else
    (*env)->DeleteLocalRef(env, localRef);

  return 0;
}

// 21
int32_t NaClSysJNI_Call_IsSameObject(struct NaClAppThread *natp, JNIEnv *false_env, jobject refa, jobject refb)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_IsSameObject(env, refa, refb);
  else
    return (int32_t) (*env)->IsSameObject(env, refa, refb);
}

int32_t NaClSysJNI_Call_NewLocalRef(struct NaClAppThread *natp, JNIEnv *false_env, jobject ref)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewLocalRef(env, ref);
  else
    return (int32_t) (*env)->NewLocalRef(env, ref);
}

int32_t NaClSysJNI_Call_EnsureLocalCapacity(struct NaClAppThread *natp, JNIEnv *false_env, jint capacity)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_EnsureLocalCapacity(env, capacity);
  else
    return (int32_t) (*env)->EnsureLocalCapacity(env, capacity);
}

int32_t NaClSysJNI_Call_AllocObject(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_AllocObject(env, clazz);
  else
    return (int32_t) (*env)->AllocObject(env, clazz);
}

int32_t NaClSysJNI_Call_NewObject(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr, result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count == -1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallVoidMethod:] FATAL - hidden_param_count == -1");
  }

  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = (int) bda_c2j_proxy_NewObject(env, clazz, methodID);
  else
    result = (int) (*env)->NewObject(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return result;
}

// 26
int32_t NaClSysJNI_Call_NewObjectV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewObjectV(env, clazz, methodID, args);
  else
    return (int32_t) (*env)->NewObjectV(env, clazz, methodID, args);
}

int32_t NaClSysJNI_Call_NewObjectA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewObjectA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->NewObjectA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_GetObjectClass(struct NaClAppThread *natp, JNIEnv * false_env, jobject obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetObjectClass(env, obj);
  else
    return (int32_t) (*env)->GetObjectClass(env, obj);
}

int32_t NaClSysJNI_Call_IsInstanceOf(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_IsInstanceOf(env, obj, clazz);
  else
    return (int32_t) (*env)->IsInstanceOf(env, obj, clazz);
}

int32_t NaClSysJNI_Call_GetMethodID(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, const char *usr_name, const char *usr_sig)
{
  jmethodID mid;
  JNIEnv * env = natp->real_env;

  //convert the name and signature to sys addresses
  char * sys_name = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_name);
  char * sys_sig = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_sig);

  if (bda_init_finish == 1)
    mid = bda_c2j_proxy_GetMethodID(env, clazz, sys_name, sys_sig);
  else
    mid = (*env)->GetMethodID(env, clazz, sys_name, sys_sig);

  //create a methodID node which contains metadata about a methodID 
  robusta_createMethodIDNode(natp->method_id_list, env, clazz, mid, sys_sig);

  return (int32_t) mid;
}

// 31
int32_t NaClSysJNI_Call_CallObjectMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;
  jobject result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  // mes310
  // sometimes a methodID is got from one thread and used in another,
  // in which case Robusta fails to find the method in its list. For
  // now this happens only to String.getBytes() in libjava.so, which
  // has one hidden parameter. We make a hack here...
  if(hidden_param_count == -1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallVoidMethod:] FATAL - hidden_param_count == -1");
    // include one more in case of need
    hidden_param_count = 2;
  }

  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallObjectMethod(env, obj, methodID);
  else
    result = (*env)->CallObjectMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));
  
  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallObjectMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallObjectMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallObjectMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallObjectMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallObjectMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallObjectMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallBooleanMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jboolean result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;		(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));
  
  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallBooleanMethod(env, obj, methodID);
  else
    result = (*env)->CallBooleanMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallBooleanMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallBooleanMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallBooleanMethodV(env, obj, methodID, sys_args);
}

// 36
int32_t NaClSysJNI_Call_CallBooleanMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallBooleanMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallBooleanMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallByteMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jbyte result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallByteMethod(env, obj, methodID);
  else
    result = (*env)->CallByteMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallByteMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallByteMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallByteMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallByteMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallByteMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallByteMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallCharMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jchar result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallCharMethod(env, obj, methodID);
  else
    result = (*env)->CallCharMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

// 41
int32_t NaClSysJNI_Call_CallCharMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallCharMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallCharMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallCharMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallCharMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallCharMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallShortMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jshort result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallShortMethod(env, obj, methodID);
  else
    result = (*env)->CallShortMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallShortMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallShortMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallShortMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallShortMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallShortMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallShortMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallIntMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr, result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallVoidMethod:] FATAL - hidden_param_count == -1");
  }

  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallIntMethod(env, obj, methodID);
  else
    result = (*env)->CallIntMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallIntMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallIntMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallIntMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallIntMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallIntMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallIntMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallLongMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jlong result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallLongMethod(env, obj, methodID);
  else
    result = (*env)->CallLongMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallLongMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallLongMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallLongMethodV(env, obj, methodID, sys_args);
}

// 51
int32_t NaClSysJNI_Call_CallLongMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallLongMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallLongMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallFloatMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jfloat result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallFloatMethod(env, obj, methodID);
  else
    result = (*env)->CallFloatMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallFloatMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallFloatMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallFloatMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallFloatMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallFloatMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallFloatMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallDoubleMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jdouble result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallDoubleMethod(env, obj, methodID);
  else
    result = (*env)->CallDoubleMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

// 56
int32_t NaClSysJNI_Call_CallDoubleMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallDoubleMethodV(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallDoubleMethodV(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallDoubleMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallDoubleMethodA(env, obj, methodID, sys_args);
  else
    return (int32_t) (*env)->CallDoubleMethodA(env, obj, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallVoidMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);
  
  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallVoidMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj; 	(x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    bda_c2j_proxy_CallVoidMethod(env, obj, methodID);
  else
    (*env)->CallVoidMethod(env, obj, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return 0;
}

int32_t NaClSysJNI_Call_CallVoidMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    bda_c2j_proxy_CallVoidMethodV(env, obj, methodID, sys_args);
  else
    (*env)->CallVoidMethodV(env, obj, methodID, sys_args);

  return 0;
}

int32_t NaClSysJNI_Call_CallVoidMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jmethodID methodID, jvalue * usr_args)
{
  JNIEnv * env = natp->real_env;

  // usr_args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) usr_args);

  if (bda_init_finish == 1)
    bda_c2j_proxy_CallVoidMethodA(env, obj, methodID, sys_args);
  else
    (*env)->CallVoidMethodA(env, obj, methodID, sys_args);

  return 0;
}

// 61
int32_t NaClSysJNI_Call_CallNonvirtualObjectMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jobject result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualObjectMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualObjectMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualObjectMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualObjectMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualObjectMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualObjectMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualObjectMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualObjectMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualBooleanMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jboolean result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualBooleanMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualBooleanMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualBooleanMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualBooleanMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualBooleanMethodV(env, obj, clazz, methodID, sys_args);
}

// 66
int32_t NaClSysJNI_Call_CallNonvirtualBooleanMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualBooleanMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualBooleanMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualByteMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jbyte result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)  
    result = bda_c2j_proxy_CallNonvirtualByteMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualByteMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualByteMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualByteMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualByteMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualByteMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualByteMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualByteMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualCharMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jchar result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualCharMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualCharMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

// 71
int32_t NaClSysJNI_Call_CallNonvirtualCharMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualCharMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualCharMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualCharMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualCharMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualCharMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualShortMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jshort result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj; 	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualShortMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualShortMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualShortMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualShortMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualShortMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualShortMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualShortMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualShortMethodA(env, obj, clazz, methodID, sys_args);
}

// 76
int32_t NaClSysJNI_Call_CallNonvirtualIntMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jint result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualIntMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualIntMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualIntMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualIntMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualIntMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualIntMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualIntMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualIntMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualLongMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jlong result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj; 	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualLongMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualLongMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualLongMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualLongMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualLongMethodV(env, obj, clazz, methodID, sys_args);
}

// 81
int32_t NaClSysJNI_Call_CallNonvirtualLongMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualLongMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualLongMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualFloatMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jfloat result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj; 	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualFloatMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualFloatMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallNonvirtualFloatMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualFloatMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualFloatMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualFloatMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualFloatMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualFloatMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualDoubleMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jdouble result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallNonvirtualDoubleMethod(env, obj, clazz, methodID);
  else
    result = (*env)->CallNonvirtualDoubleMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

// 86
int32_t NaClSysJNI_Call_CallNonvirtualDoubleMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualDoubleMethodV(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualDoubleMethodV(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualDoubleMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallNonvirtualDoubleMethodA(env, obj, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallNonvirtualDoubleMethodA(env, obj, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallNonvirtualVoidMethod(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here we have one more argument, so the stack is different
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jobject obj;	(x_esp)
    jclass clazz;       (x_esp+4)
    jmethodID methodID;	(x_esp+8)
    <optional param 1>	(x_esp+C)
    <optional param 2>	(x_esp+10)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  //mes310: one more argument, so offset should be 11+4=15???
  src_addr = ((int *) natp->x_esp)+15;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    bda_c2j_proxy_CallNonvirtualVoidMethod(env, obj, clazz, methodID);
  else
    (*env)->CallNonvirtualVoidMethod(env, obj, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return 0;
}

int32_t NaClSysJNI_Call_CallNonvirtualVoidMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    bda_c2j_proxy_CallNonvirtualVoidMethodV(env, obj, clazz, methodID, sys_args);
  else
    (*env)->CallNonvirtualVoidMethodV(env, obj, clazz, methodID, sys_args);

  return 0;
}

int32_t NaClSysJNI_Call_CallNonvirtualVoidMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  // args is usr addr, so first convert it to sys addr
  jvalue * sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    bda_c2j_proxy_CallNonvirtualVoidMethodA(env, obj, clazz, methodID, sys_args);
  else
    (*env)->CallNonvirtualVoidMethodA(env, obj, clazz, methodID, sys_args);

  return 0;
}

// 91
int32_t NaClSysJNI_Call_GetFieldID(struct NaClAppThread *natp, JNIEnv * false_env, jclass clazz, const char *usr_name, const char *usr_sig)
{
  JNIEnv * env = natp->real_env;

  //convert the name and signature to sys addresses
  char * sys_name = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_name);
  char * sys_sig = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_sig);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetFieldID(env, clazz, sys_name, sys_sig);
  else
    return (int32_t) (*env)->GetFieldID(env, clazz, sys_name, sys_sig);
}

int32_t NaClSysJNI_Call_GetObjectField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetObjectField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetObjectField(env, obj, fieldID);
}

int32_t NaClSysJNI_Call_GetBooleanField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetBooleanField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetBooleanField(env, obj, fieldID);
}

int32_t NaClSysJNI_Call_GetByteField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetByteField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetByteField(env, obj, fieldID);
}

int32_t NaClSysJNI_Call_GetCharField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetCharField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetCharField(env, obj, fieldID);
}

// 96
int32_t NaClSysJNI_Call_GetShortField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetShortField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetShortField(env, obj, fieldID);
}

int32_t NaClSysJNI_Call_GetIntField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetIntField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetIntField(env, obj, fieldID);
}

int32_t NaClSysJNI_Call_GetLongField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  long ret;

  if (bda_init_finish == 1)
    ret = bda_c2j_proxy_GetLongField(env, obj, fieldID);
  else
    ret = (*env)->GetLongField(env, obj, fieldID);

  return (int32_t) ret;
}

int32_t NaClSysJNI_Call_GetFloatField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetFloatField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetFloatField(env, obj, fieldID);
}

int32_t NaClSysJNI_Call_GetDoubleField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetDoubleField(env, obj, fieldID);
  else
    return (int32_t) (*env)->GetDoubleField(env, obj, fieldID);
}

// 101
int32_t NaClSysJNI_Call_SetObjectField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jobject value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetObjectField(env, obj, fieldID, value);
  else
    (*env)->SetObjectField(env, obj, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetBooleanField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jboolean value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetBooleanField(env,obj,fieldID,value);
  else
    (*env)->SetBooleanField(env, obj, fieldID, value);
  
  return 0;
}

int32_t NaClSysJNI_Call_SetByteField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jbyte value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetByteField(env, obj, fieldID, value);
  else
    (*env)->SetByteField(env, obj, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetCharField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jchar value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetCharField(env, obj, fieldID, value);
  else
    (*env)->SetCharField(env, obj, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetShortField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jshort value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetShortField(env, obj, fieldID, value);
  else
    (*env)->SetShortField(env, obj, fieldID, value);
  
  return 0;
}

// 106
int32_t NaClSysJNI_Call_SetIntField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jint value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetIntField(env,obj,fieldID,value);
  else
    (*env)->SetIntField(env, obj, fieldID, value);
  
  return 0;
}

int32_t NaClSysJNI_Call_SetLongField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jlong value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetLongField(env, obj, fieldID, value);
  else
    (*env)->SetLongField(env, obj, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetFloatField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jfloat value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetFloatField(env, obj, fieldID, value);
  else
    (*env)->SetFloatField(env, obj, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetDoubleField(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj, jfieldID fieldID, jdouble value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetDoubleField(env, obj, fieldID, value);
  else
    (*env)->SetDoubleField(env, obj, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_GetStaticMethodID(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, const char *usr_name, const char *usr_sig)
{
  jmethodID mid;
  JNIEnv * env = natp->real_env;

  //convert the name and signature to sys addresses
  char * sys_name = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_name);
  char * sys_sig = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_sig);

  if (bda_init_finish == 1)
    mid = bda_c2j_proxy_GetStaticMethodID(env, clazz, sys_name, sys_sig);
  else
    mid = (*env)->GetStaticMethodID(env, clazz, sys_name, sys_sig);
  
  //create a methodID node which contains metadata about a methodID
  // mes310 temporarily commented out...
  if (mid != NULL)
    robusta_createMethodIDNode(natp->method_id_list, env, clazz, mid, sys_sig);
  
  return (int32_t) mid;
}

// 111
int32_t NaClSysJNI_Call_CallStaticObjectMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jobject result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticObjectMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticObjectMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticObjectMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  jobject result;
  
  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticObjectMethodV(env, clazz, methodID, sys_args);
  else
    result = (*env)->CallStaticObjectMethodV(env, clazz, methodID, sys_args);

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticObjectMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticObjectMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticObjectMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticBooleanMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jboolean result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  // mes310 currently just make the number fixed... very dirty hack..
  //int hidden_param_count = 5;

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticBooleanMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticBooleanMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticBooleanMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticBooleanMethodV(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticBooleanMethodV(env, clazz, methodID, sys_args);
}

// 116
int32_t NaClSysJNI_Call_CallStaticBooleanMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticBooleanMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticBooleanMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticByteMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jbyte result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticByteMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticByteMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticByteMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticByteMethodV(env, clazz, methodID, sys_args);
  else
    return (*env)->CallStaticByteMethodV(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticByteMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticByteMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticByteMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticCharMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jchar result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticCharMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticCharMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

// 121
int32_t NaClSysJNI_Call_CallStaticCharMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticCharMethodV(env, clazz, methodID, sys_args);
  else
    return (*env)->CallStaticCharMethodV(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticCharMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticCharMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticCharMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticShortMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jshort result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticShortMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticShortMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticShortMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticShortMethodV(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticShortMethodV(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticShortMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticShortMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticShortMethodA(env, clazz, methodID, sys_args);
}

// 126
int32_t NaClSysJNI_Call_CallStaticIntMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jint result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticIntMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticIntMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticIntMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticIntMethodV(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticIntMethodV(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticIntMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticIntMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticIntMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticLongMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jlong result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
      NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticLongMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticLongMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticLongMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticLongMethodV(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticLongMethodV(env, clazz, methodID, sys_args);
}

// 131
int32_t NaClSysJNI_Call_CallStaticLongMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticLongMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticLongMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticFloatMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jfloat result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticFloatMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticFloatMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

int32_t NaClSysJNI_Call_CallStaticFloatMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticFloatMethodV(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticFloatMethodV(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticFloatMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticFloatMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticFloatMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticDoubleMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  jdouble result;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if(hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    result = bda_c2j_proxy_CallStaticDoubleMethod(env, clazz, methodID);
  else
    result = (*env)->CallStaticDoubleMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return (int32_t) result;
}

// 136
int32_t NaClSysJNI_Call_CallStaticDoubleMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticDoubleMethodV(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticDoubleMethodV(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticDoubleMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_CallStaticDoubleMethodA(env, clazz, methodID, sys_args);
  else
    return (int32_t) (*env)->CallStaticDoubleMethodA(env, clazz, methodID, sys_args);
}

int32_t NaClSysJNI_Call_CallStaticVoidMethod(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID)
{
  int * old_esp, * new_esp, * src_addr, * dst_addr;

  JNIEnv * env = natp->real_env;

  //push appropriate number of words onto the stack
  int hidden_param_count = robusta_findParameterCount(natp->method_id_list, methodID);

  if (hidden_param_count==-1) {
    NaClLog(0, "[nacl_syscall_impl.c: CallBooleanMethod:] FATAL - hidden_param_count == -1");
  }
  //int num_bytes = param_count*4;
	
  //the contents of the sandbox stack is the following
  //mes310
  //Here the number of args is the same with the original implementation
  /*TOS: (natp->x_esp)
    JNIEnv *false_env; 	(x_esp-4)
    jclass clazz;       (x_esp)
    jmethodID methodID;	(x_esp+4)
    <optional param 1>	(x_esp+8)
    <optional param 2>	(x_esp+C)
    ....
    <optional param n>
    }*/

  //***probably compiler/platform sensitive***
  //Not sure why the parameters we want are at the particular offset of 11
  src_addr = ((int *) natp->x_esp)+11;

  //adjust the stack pointer so we can sneak in some parameters
  asm volatile("movl %%esp, %0" :"=r" (old_esp));
  new_esp = old_esp - hidden_param_count;
  asm volatile("movl %0, %%esp"::"r" (new_esp));

  //inline memcpy
  dst_addr = new_esp + 3; //adjust for the parameters the compiler expects
  asm volatile("cld ; rep ; movsb":  : "c" ((unsigned int) hidden_param_count*4), "S" (src_addr), "D" (dst_addr));

  if (bda_init_finish == 1)
    bda_c2j_proxy_CallStaticVoidMethod(env, clazz, methodID);
  else
    (*env)->CallStaticVoidMethod(env, clazz, methodID);

  asm volatile("movl %0, %%esp"::"r" (old_esp));

  return 0;
}

int32_t NaClSysJNI_Call_CallStaticVoidMethodV(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, va_list args)
{
  JNIEnv * env = natp->real_env;

  // args is a usr addr, so first convert it to a sys addr
  va_list sys_args = (va_list) NaClUserToSys(natp->nap, (uintptr_t) args);
  
  if (bda_init_finish == 1)
    bda_c2j_proxy_CallStaticVoidMethodV(env, clazz, methodID, sys_args);
  else
    (*env)->CallStaticVoidMethodV(env, clazz, methodID, sys_args);

  return 0;
}

int32_t NaClSysJNI_Call_CallStaticVoidMethodA(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jmethodID methodID, jvalue *args)
{
  JNIEnv * env = natp->real_env;

  //args is a usr addr, so first convert it to sys addr
  jvalue *sys_args = (jvalue *) NaClUserToSys(natp->nap, (uintptr_t) args);

  if (bda_init_finish == 1)
    bda_c2j_proxy_CallStaticVoidMethodA(env, clazz, methodID, sys_args);
  else
    (*env)->CallStaticVoidMethodA(env, clazz, methodID, sys_args);

  return 0;
}

// 141
int32_t NaClSysJNI_Call_GetStaticFieldID(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, const char *usr_name, const char *usr_sig)
{
  jfieldID fid;
  JNIEnv * env = natp->real_env;

  //convert the name and signature buffer to a system addr
  char * sys_name = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_name);
  char * sys_sig = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_sig);

  if (bda_init_finish == 1)
    fid = bda_c2j_proxy_GetStaticFieldID(env, clazz, sys_name, sys_sig);
  else
    fid = (*env)->GetStaticFieldID(env, clazz, sys_name, sys_sig);
  
  robusta_createFieldIDNode(natp->field_id_list, env, clazz, fid, sys_sig);

  return (int32_t) fid;
}

int32_t NaClSysJNI_Call_GetStaticObjectField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticObjectField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticObjectField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticBooleanField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticBooleanField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticBooleanField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticByteField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticByteField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticByteField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticCharField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticCharField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticCharField(env, clazz, fieldID);
}

// 146
int32_t NaClSysJNI_Call_GetStaticShortField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticShortField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticShortField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticIntField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticIntField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticIntField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticLongField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticLongField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticLongField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticFloatField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticFloatField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticFloatField(env, clazz, fieldID);
}

int32_t NaClSysJNI_Call_GetStaticDoubleField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStaticDoubleField(env, clazz, fieldID);
  else
    return (int32_t) (*env)->GetStaticDoubleField(env, clazz, fieldID);
}

// 151
int32_t NaClSysJNI_Call_SetStaticObjectField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jobject value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticObjectField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticObjectField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticBooleanField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jboolean value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticBooleanField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticBooleanField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticByteField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jbyte value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticByteField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticByteField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticCharField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jchar value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticCharField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticCharField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticShortField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jshort value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticShortField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticShortField(env, clazz, fieldID, value);

  return 0;
}

// 156
int32_t NaClSysJNI_Call_SetStaticIntField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jint value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticIntField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticIntField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticLongField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jlong value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticLongField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticLongField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticFloatField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jfloat value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticFloatField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticFloatField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_SetStaticDoubleField(struct NaClAppThread *natp, JNIEnv *false_env, jclass clazz, jfieldID fieldID, jdouble value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetStaticDoubleField(env, clazz, fieldID, value);
  else
    (*env)->SetStaticDoubleField(env, clazz, fieldID, value);

  return 0;
}

int32_t NaClSysJNI_Call_NewString(struct NaClAppThread *natp, JNIEnv *false_env, const jchar *uchars, jsize len)
{
  JNIEnv * env = natp->real_env;

  //uchars is a usr addr, so first convert it to a sys addr
  jchar *sys_uchars = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) uchars);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewString(env, sys_uchars, len);
  else
    return (int32_t) (*env)->NewString(env, sys_uchars, len);
}

// 161
int32_t NaClSysJNI_Call_GetStringLength(struct NaClAppThread *natp, JNIEnv *false_env, jstring string)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStringLength(env, string);
  else
    return (int32_t) (*env)->GetStringLength(env, string);
}

int32_t NaClSysJNI_Call_GetStringChars(struct NaClAppThread *natp, JNIEnv *false_env, jstring string, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStringChars(env, string, sys_isCopy);
  else
    return (int32_t) (*env)->GetStringChars(env, string, sys_isCopy);
}

int32_t NaClSysJNI_Call_ReleaseStringChars(struct NaClAppThread *natp, JNIEnv *false_env, jstring string, const jchar *chars)
{
  JNIEnv * env = natp->real_env;

  //chars is a usr addr, so first convert it to a sys addr
  jchar * sys_chars = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) chars);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseStringChars(env, string, sys_chars);
  else
    (*env)->ReleaseStringChars(env, string, sys_chars);

  return 0;
}

int32_t NaClSysJNI_Call_NewStringUTF(struct NaClAppThread *natp, JNIEnv * false_env, const char * usr_buf)
{
  JNIEnv * env = natp->real_env;

  //bytes is a usr addr, so first convert it to a sys addr
  char * sys_buf = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  int r;
  if (bda_init_finish == 1)
    r = (int32_t) bda_c2j_proxy_NewStringUTF(env, sys_buf);
  else
    r = (int32_t) (*env)->NewStringUTF(env, sys_buf);
  return r;
}

int32_t NaClSysJNI_Call_GetStringUTFLength(struct NaClAppThread *natp, JNIEnv * false_env, jstring string)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return bda_c2j_proxy_GetStringUTFLength(env, string);
  else
    return (*env)->GetStringUTFLength(env, string);
}

// 166
int32_t NaClSysJNI_Call_GetStringUTFChars(struct NaClAppThread *natp, JNIEnv * false_env, jstring string, jboolean *isCopy)
{
  // This syscall is currently unused...
  return 0;
}

int32_t NaClSysJNI_Call_ReleaseStringUTFChars(struct NaClAppThread *natp, JNIEnv * false_env, jstring string, const char * utf)
{
  JNIEnv * env = natp->real_env;

  //utf is a usr addr, so first convert it to a sys addr
  char * sys_utf = (char *) NaClUserToSys(natp->nap, (uintptr_t) utf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseStringUTFChars(env, string, sys_utf);
  else
    (*env)->ReleaseStringUTFChars(env, string, sys_utf);

  return 0;
}

int32_t NaClSysJNI_Call_GetArrayLength(struct NaClAppThread *natp, JNIEnv *false_env, jarray array)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return bda_c2j_proxy_GetArrayLength(env,array);
  else
    return (*env)->GetArrayLength(env, array);
}

int32_t NaClSysJNI_Call_NewObjectArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length, jclass elementClass, jobject initialElement)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewObjectArray(env, length, elementClass, initialElement);
  else
    return (int32_t) (*env)->NewObjectArray(env, length, elementClass, initialElement);
}

int32_t NaClSysJNI_Call_GetObjectArrayElement(struct NaClAppThread *natp, JNIEnv *false_env, jobjectArray array, jsize index)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetObjectArrayElement(env, array, index);
  else
    return (int32_t) (*env)->GetObjectArrayElement(env, array, index);
}

// 171
int32_t NaClSysJNI_Call_SetObjectArrayElement(struct NaClAppThread *natp, JNIEnv *false_env, jobjectArray array, jsize index, jobject value)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetObjectArrayElement(env, array, index, value);
  else
    (*env)->SetObjectArrayElement(env, array, index, value);

  return 0;
}

int32_t NaClSysJNI_Call_NewBooleanArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewBooleanArray(env, length);
  else
    return (int32_t) (*env)->NewBooleanArray(env, length);
}

int32_t NaClSysJNI_Call_NewByteArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewByteArray(env, length);
  else
    return (int32_t) (*env)->NewByteArray(env, length);
}

int32_t NaClSysJNI_Call_NewCharArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewCharArray(env, length);
  else
    return (int32_t) (*env)->NewCharArray(env, length);
}

int32_t NaClSysJNI_Call_NewShortArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewShortArray(env, length);
  else
    return (int32_t) (*env)->NewShortArray(env, length);
}

// 176
int32_t NaClSysJNI_Call_NewIntArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewIntArray(env, length);
  else
    return (int32_t) (*env)->NewIntArray(env, length);
}

int32_t NaClSysJNI_Call_NewLongArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewLongArray(env, length);
  else
    return (int32_t) (*env)->NewLongArray(env, length);
}

int32_t NaClSysJNI_Call_NewFloatArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewFloatArray(env, length);
  else
    return (int32_t) (*env)->NewFloatArray(env, length);
}

int32_t NaClSysJNI_Call_NewDoubleArray(struct NaClAppThread *natp, JNIEnv *false_env, jsize length)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewDoubleArray(env, length);
  else
    return (int32_t) (*env)->NewDoubleArray(env, length);
}

// 181
int32_t NaClSysJNI_Call_GetBooleanArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jbooleanArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetBooleanArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetBooleanArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_GetByteArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jbyteArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetByteArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetByteArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_GetCharArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jcharArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetCharArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetCharArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_GetShortArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jshortArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetShortArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetShortArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_GetIntArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jintArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetIntArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetIntArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_GetLongArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jlongArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetLongArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetLongArrayElements(env, array, sys_isCopy);
}

// 186
int32_t NaClSysJNI_Call_GetFloatArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jfloatArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetFloatArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetFloatArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_GetDoubleArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jdoubleArray array, jboolean *isCopy)
{
  JNIEnv * env = natp->real_env;

  //isCopy is a usr addr, so first convert it to a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetDoubleArrayElements(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetDoubleArrayElements(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_ReleaseBooleanArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jbooleanArray array, jboolean *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jboolean * sys_elems = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseBooleanArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseBooleanArrayElements(env, array, sys_elems, mode);

  return 0;
}

int32_t NaClSysJNI_Call_ReleaseByteArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jbyteArray array, jbyte *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jbyte * sys_elems = (jbyte *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseByteArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseByteArrayElements(env, array, sys_elems, mode);

  return 0;
}

int32_t NaClSysJNI_Call_ReleaseCharArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jcharArray array, jchar *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jchar * sys_elems = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseCharArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseCharArrayElements(env, array, sys_elems, mode);

  return 0;
}

// 191
int32_t NaClSysJNI_Call_ReleaseShortArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jshortArray array, jshort *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jshort * sys_elems = (jshort *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseShortArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseShortArrayElements(env, array, sys_elems, mode);

  return 0;
}

int32_t NaClSysJNI_Call_ReleaseIntArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jintArray array, jint *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jint * sys_elems = (jint *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseIntArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseIntArrayElements(env, array, sys_elems, mode);

  return 0;
}

int32_t NaClSysJNI_Call_ReleaseLongArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jlongArray array, jlong *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jlong * sys_elems = (jlong *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseLongArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseLongArrayElements(env, array, sys_elems, mode);

  return 0;
}

int32_t NaClSysJNI_Call_ReleaseFloatArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jfloatArray array, jfloat *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jfloat * sys_elems = (jfloat *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseFloatArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseFloatArrayElements(env, array, sys_elems, mode);

  return 0;
}

int32_t NaClSysJNI_Call_ReleaseDoubleArrayElements(struct NaClAppThread *natp, JNIEnv *false_env, jdoubleArray array, jdouble *elems, jint mode)
{
  JNIEnv * env = natp->real_env;

  //elems is a usr addr, so first convert it to a sys addr
  jdouble * sys_elems = (jdouble *) NaClUserToSys(natp->nap, (uintptr_t) elems);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseDoubleArrayElements(env, array, sys_elems, mode);
  else
    (*env)->ReleaseDoubleArrayElements(env, array, sys_elems, mode);

  return 0;
}

// 196
int32_t NaClSysJNI_Call_GetBooleanArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jbooleanArray array, jsize start, jsize len, jboolean *buf)
{
  JNIEnv * env = natp->real_env;

  //buf is a usr addr, so first convert it to a sys addr
  jboolean * sys_buf = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetBooleanArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetBooleanArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetByteArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jbyteArray array, jsize start, jsize len, jbyte *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jbyte * sys_buf = (jbyte *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetByteArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetByteArrayRegion(env, array, start, len, sys_buf);
    
  return 0;
}

int32_t NaClSysJNI_Call_GetCharArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jcharArray array, jsize start, jsize len, jchar *buf)
{
  JNIEnv * env = natp->real_env;

  //buf is a usr addr, so first convert it to a sys addr
  jchar * sys_buf = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetCharArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetCharArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetShortArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jshortArray array, jsize start, jsize len, jshort *buf)
{
  JNIEnv * env = natp->real_env;

  //buf is a usr addr, so first convert it to a sys addr
  jshort * sys_buf = (jshort *) NaClUserToSys(natp->nap, (uintptr_t) buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetShortArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetShortArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetIntArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jintArray array, jsize start, jsize len, jint *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jint * sys_buf = (jint *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetIntArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetIntArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

// 201
int32_t NaClSysJNI_Call_GetLongArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jlongArray array, jsize start, jsize len, jlong *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jlong * sys_buf = (jlong *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetLongArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetLongArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetFloatArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jfloatArray array, jsize start, jsize len, jfloat *buf)
{
  JNIEnv * env = natp->real_env;

  //buf is a usr addr, so first convert it to a sys addr
  jfloat * sys_buf = (jfloat *) NaClUserToSys(natp->nap, (uintptr_t) buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetFloatArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetFloatArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetDoubleArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jdoubleArray array, jsize start, jsize len, jdouble *buf)
{
  JNIEnv * env = natp->real_env;

  //buf is a usr addr, so first convert it to a sys addr
  jdouble * sys_buf = (jdouble *) NaClUserToSys(natp->nap, (uintptr_t) buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_GetDoubleArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->GetDoubleArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_SetBooleanArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jbooleanArray array, jsize start, jsize len, jboolean *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jboolean * sys_buf = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetBooleanArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetBooleanArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_SetByteArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jbyteArray array, jsize start, jsize len, jbyte *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jbyte * sys_buf = (jbyte *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetByteArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetByteArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

// 206
int32_t NaClSysJNI_Call_SetCharArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jcharArray array, jsize start, jsize len, jchar *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jchar * sys_buf = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetCharArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetCharArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_SetShortArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jshortArray array, jsize start, jsize len, jshort *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jshort * sys_buf = (jshort *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetShortArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetShortArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_SetIntArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jintArray array, jsize start, jsize len, jint *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jint * sys_buf = (jint *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetIntArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetIntArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_SetLongArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jlongArray array, jsize start, jsize len, jlong *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jlong * sys_buf = (jlong *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetLongArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetLongArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_SetFloatArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jfloatArray array, jsize start, jsize len, jfloat *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jfloat * sys_buf = (jfloat *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetFloatArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetFloatArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

// 211
int32_t NaClSysJNI_Call_SetDoubleArrayRegion(struct NaClAppThread *natp, JNIEnv *false_env, jdoubleArray array, jsize start, jsize len, jdouble *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  jdouble * sys_buf = (jdouble *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  if (bda_init_finish == 1)
    bda_c2j_proxy_SetDoubleArrayRegion(env, array, start, len, sys_buf);
  else
    (*env)->SetDoubleArrayRegion(env, array, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_RegisterNatives(struct NaClAppThread * natp, JNIEnv * false_env, jclass clazz, JNINativeMethod * methods, jint nMethods)
{
  int i;
  JNIEnv * env = natp->real_env;

  // we have to reconstruct the methods list since it's actually a
  // 2-dimensional array

  JNINativeMethod * nacl_methods = (JNINativeMethod *) malloc(nMethods * sizeof(JNINativeMethod));
  assert(nacl_methods != NULL);

  //convert a sandbox addr to a system addr
  JNINativeMethod * sys_methods = (JNINativeMethod *) NaClUserToSys(natp->nap, (uintptr_t) methods);
  
  for (i = 0; i < nMethods; i++) {
    // convert name
    nacl_methods[i].name = (char *) NaClUserToSys(natp->nap, (uintptr_t) sys_methods[i].name);
    // convert signature
    nacl_methods[i].signature = (char *) NaClUserToSys(natp->nap, (uintptr_t) sys_methods[i].signature);
    // convert addr
    nacl_methods[i].fnPtr = (void *) NaClUserToSys(natp->nap, (uintptr_t) sys_methods[i].fnPtr);
  }

  printf("[nacl: RegisterNatives] nMethods: %d, addr of the strings:\n", nMethods);

  for (i = 0; i < nMethods; i++) {
    printf("%s\t%s\t%p\n", nacl_methods[i].name, nacl_methods[i].signature, nacl_methods[i].fnPtr);
  }
  fflush(stdout);

  int ret;
  
  if (bda_init_finish == 1)
    ret = (int32_t) bda_c2j_proxy_RegisterNatives(env, clazz, nacl_methods, nMethods);
  else
    ret = (int32_t) (*env)->RegisterNatives(env, clazz, nacl_methods, nMethods);

  return ret;
}

int32_t NaClSysJNI_Call_UnregisterNatives(struct NaClAppThread * natp, JNIEnv * false_env, jclass clazz)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_UnregisterNatives(env, clazz);
  else
    return (int32_t) (*env)->UnregisterNatives(env, clazz);
}

int32_t NaClSysJNI_Call_MonitorEnter(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return bda_c2j_proxy_MonitorEnter(env, obj);
  else
    return (*env)->MonitorEnter(env, obj);
}

int32_t NaClSysJNI_Call_MonitorExit(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return bda_c2j_proxy_MonitorExit(env, obj);
  else
    return (*env)->MonitorExit(env, obj);
}

// 216
int32_t NaClSysJNI_Call_GetJavaVM(struct NaClAppThread *natp, JNIEnv *false_env, JavaVM ** vm)
{
  JNIEnv * env = natp->real_env;

  //convert a sandbox addr to a system addr
  JavaVM ** sys_vm = (JavaVM **) NaClUserToSys(natp->nap, (uintptr_t) vm);

  if (bda_init_finish == 1)
    return bda_c2j_proxy_GetJavaVM(env, sys_vm);
  else
    return (*env)->GetJavaVM(env, sys_vm);
}

int32_t NaClSysJNI_Call_GetStringRegion(struct NaClAppThread *natp, JNIEnv *false_env, jstring str, jsize start, jsize len, jchar *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert the user address into a system address
  jchar * sys_buf = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  //do the copy
  if (bda_init_finish == 1)
    bda_c2j_proxy_GetStringRegion(env, str, start, len, sys_buf);
  else
    (*env)->GetStringRegion(env, str, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetStringUTFRegion(struct NaClAppThread *natp, JNIEnv *false_env, jstring str, jsize start, jsize len, char *usr_buf)
{
  JNIEnv * env = natp->real_env;

  //convert the user address into a system address
  char * sys_buf = (char *) NaClUserToSys(natp->nap, (uintptr_t) usr_buf);

  //do the copy
  if (bda_init_finish == 1)
    bda_c2j_proxy_GetStringUTFRegion(env, str, start, len, sys_buf);
  else
    (*env)->GetStringUTFRegion(env, str, start, len, sys_buf);

  return 0;
}

int32_t NaClSysJNI_Call_GetPrimitiveArrayCritical(struct NaClAppThread * natp, JNIEnv * false_env, jarray array, jboolean * isCopy)
{
  JNIEnv * env = natp->real_env;

  //convert the user addr into a sys addr
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetPrimitiveArrayCritical(env, array, sys_isCopy);
  else
    return (int32_t) (*env)->GetPrimitiveArrayCritical(env, array, sys_isCopy);
}

int32_t NaClSysJNI_Call_ReleasePrimitiveArrayCritical(struct NaClAppThread *natp, JNIEnv *false_env, jarray array, void * carray, jint mode)
{
  JNIEnv * env = natp->real_env;

  //convert the user address into a system address
  void * sys_carray = (void *) NaClUserToSys(natp->nap, (uintptr_t) carray);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleasePrimitiveArrayCritical(env, array, sys_carray, mode);
  else
    (*env)->ReleasePrimitiveArrayCritical(env, array, sys_carray, mode);

  return 0;
}

// 221
int32_t NaClSysJNI_Call_GetStringCritical(struct NaClAppThread *natp, JNIEnv *false_env, jstring string, jboolean * isCopy)
{
  JNIEnv * env = natp->real_env;

  //convert the user address into a system address
  jboolean * sys_isCopy = (jboolean *) NaClUserToSys(natp->nap, (uintptr_t) isCopy);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetStringCritical(env, string, sys_isCopy);
  else
    return (int32_t) (*env)->GetStringCritical(env, string, sys_isCopy);
}

int32_t NaClSysJNI_Call_ReleaseStringCritical(struct NaClAppThread *natp, JNIEnv *false_env, jstring string, jchar * carray)
{
  JNIEnv * env = natp->real_env;

  //convert the user address into a system address
  jchar * sys_carray = (jchar *) NaClUserToSys(natp->nap, (uintptr_t) carray);

  if (bda_init_finish == 1)
    bda_c2j_proxy_ReleaseStringCritical(env, string, sys_carray);
  else
    (*env)->ReleaseStringCritical(env, string, sys_carray);

  return 0;
}

int32_t NaClSysJNI_Call_NewWeakGlobalRef(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewWeakGlobalRef(env, obj);
  else
    return (int32_t) (*env)->NewWeakGlobalRef(env, obj);
}

int32_t NaClSysJNI_Call_DeleteWeakGlobalRef(struct NaClAppThread *natp, JNIEnv *false_env, jweak wref)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    bda_c2j_proxy_DeleteWeakGlobalRef(env, wref);
  else
    (*env)->DeleteWeakGlobalRef(env, wref);

  return 0;
}

int32_t NaClSysJNI_Call_ExceptionCheck(struct NaClAppThread *natp, JNIEnv *false_env)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_ExceptionCheck(env);
  else
    return (int32_t) (*env)->ExceptionCheck(env);
}

// 226
int32_t NaClSysJNI_Call_NewDirectByteBuffer(struct NaClAppThread *natp, JNIEnv *false_env, void * address, jlong capacity)
{
  JNIEnv * env = natp->real_env;

  //convert the user address into a system address
  void * sys_address = (void *) NaClUserToSys(natp->nap, (uintptr_t) address);

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_NewDirectByteBuffer(env, sys_address, capacity);
  else
    return (int32_t) (*env)->NewDirectByteBuffer(env, sys_address, capacity);
}

int32_t NaClSysJNI_Call_GetDirectBufferAddress(struct NaClAppThread *natp, JNIEnv *false_env, jobject buf)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetDirectBufferAddress(env, buf);
  else
    return (int32_t) (*env)->GetDirectBufferAddress(env, buf);
}

int32_t NaClSysJNI_Call_GetDirectBufferCapacity(struct NaClAppThread *natp, JNIEnv *false_env, jobject buf)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetDirectBufferCapacity(env, buf);
  else
    return (int32_t) (*env)->GetDirectBufferCapacity(env, buf);
}

int32_t NaClSysJNI_Call_GetObjectRefType(struct NaClAppThread *natp, JNIEnv *false_env, jobject obj)
{
  JNIEnv * env = natp->real_env;

  if (bda_init_finish == 1)
    return (int32_t) bda_c2j_proxy_GetObjectRefType(env, obj);
  else
    return (int32_t) (*env)->GetObjectRefType(env, obj);
}


//-------------------------------------------------------------------------------------------------

int32_t NaClSysMmap(struct NaClAppThread  *natp,
                    void                  *start,
                    size_t                length,
                    int                   prot,
                    int                   flags,
                    int                   d,
                    nacl_abi_off_t        offset) {
  // gtan: a temporary hack for getting out of the sandbox
  if (((int)start == 0xdeadbeef) && (length == 0xcafe)) {
    return (int32_t) natp->false_env;
  } else
    return NaClCommonSysMmap(natp, start, length, prot, flags, d, offset);
}

int32_t NaClSysMunmap(struct NaClAppThread  *natp,
                      void                  *start,
                      size_t                length) {
  int32_t   retval = -NACL_ABI_EINVAL;
  uintptr_t sysaddr;
  int       holding_app_lock = 0;
  size_t    alloc_rounded_length;

  NaClLog(3, "NaClSysMunmap(0x%08"PRIxPTR", 0x%08"PRIxPTR", 0x%"PRIxS")\n",
          (uintptr_t) natp, (uintptr_t) start, length);

  NaClSysCommonThreadSyscallEnter(natp);

  if (!NaClIsAllocPageMultiple((uintptr_t) start)) {
    NaClLog(4, "start addr not allocation multiple\n");
    retval = -NACL_ABI_EINVAL;
    goto cleanup;
  }
  if (0 == length) {
    /*
     * linux mmap of zero length yields a failure, but osx does not, leading
     * to a NaClVmmapUpdate of zero pages, which should not occur.
     */
    retval = -NACL_ABI_EINVAL;
    goto cleanup;
  }
  alloc_rounded_length = NaClRoundAllocPage(length);
  if (alloc_rounded_length != length) {
    length = alloc_rounded_length;
    NaClLog(LOG_WARNING,
            "munmap: rounded length to 0x%"PRIxS"\n", length);
  }
  sysaddr = NaClUserToSysAddrRange(natp->nap, (uintptr_t) start, length);
  if (kNaClBadAddress == sysaddr) {
    NaClLog(4, "region not user addresses\n");
    retval = -NACL_ABI_EFAULT;
    goto cleanup;
  }

  NaClXMutexLock(&natp->nap->mu);
  holding_app_lock = 1;
  /*
   * NB: windows (or generic) version would use Munmap virtual
   * function from the backing NaClDesc object obtained by iterating
   * through the address map for the region, and those Munmap virtual
   * functions may return -NACL_ABI_E_MOVE_ADDRESS_SPACE.
   *
   * We should hold the application lock while doing this iteration
   * and unmapping, so that the address space is consistent for other
   * threads.
   */

  /*
   * User should be unable to unmap any executable pages.  We check here.
   */
  if (NaClSysCommonAddrRangeContainsExecutablePages_mu(natp->nap,
                                                       (uintptr_t) start,
                                                       length)) {
    NaClLog(2, "NaClSysMunmap: region contains executable pages\n");
    retval = -NACL_ABI_EINVAL;
    goto cleanup;
  }

  /*
   * Overwrite current mapping with inaccessible, anonymous
   * zero-filled pages, which should be copy-on-write and thus
   * relatively cheap.  Do not open up an address space hole.
   */
  NaClLog(3,
          ("NaClSysMunmap: mmap(0x%08"PRIxPTR", 0x%"PRIxS","
           " 0x%x, 0x%x, -1, 0)\n"),
          sysaddr, length, PROT_NONE,
          MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED);
  if (MAP_FAILED == mmap((void *) sysaddr,
                         length,
                         PROT_NONE,
                         MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
                         -1,
                         (off_t) 0)) {
    NaClLog(3, "mmap to put in anonymous memory failed, errno = %d\n", errno);
    retval = -NaClXlateErrno(errno);
    goto cleanup;
  }
  NaClVmmapUpdate(&natp->nap->mem_map,
                  (NaClSysToUser(natp->nap, (uintptr_t) sysaddr)
                   >> NACL_PAGESHIFT),
                  length >> NACL_PAGESHIFT,
                  0,  /* prot */
                  (struct NaClMemObj *) NULL,
                  1);  /* Delete mapping */
  retval = 0;
cleanup:
  if (holding_app_lock) {
    NaClXMutexUnlock(&natp->nap->mu);
  }
  NaClSysCommonThreadSyscallLeave(natp);
  return retval;
}

int32_t NaClSysExit(struct NaClAppThread *natp,
                    int                  status) {
  return NaClCommonSysExit(natp, status);
}

int32_t NaClSysGetpid(struct NaClAppThread *natp) {
  int32_t pid;

  NaClSysCommonThreadSyscallEnter(natp);

  pid = getpid();  /* TODO(bsy): obfuscate? */
  NaClLog(4, "NaClSysGetpid: returning %d\n", pid);

  NaClSysCommonThreadSyscallLeave(natp);
  return pid;
}

int32_t NaClSysThread_Exit(struct NaClAppThread  *natp,
                           int32_t               *stack_flag) {
  return NaClCommonSysThreadExit(natp, stack_flag);
}

int32_t NaClSysGetTimeOfDay(struct NaClAppThread      *natp,
                            struct nacl_abi_timeval   *tv,
                            struct nacl_abi_timezone  *tz) {
  uintptr_t               sysaddr;
  int                     retval;
  struct nacl_abi_timeval now;

  NaClSysCommonThreadSyscallEnter(natp);

  sysaddr = NaClUserToSysAddrRange(natp->nap, (uintptr_t) tv, sizeof tv);

  /*
   * tz is not supported in linux, nor is it supported by glibc, since
   * tzset(3) and the zoneinfo file should be used instead.
   *
   * TODO(bsy) Do we make the zoneinfo directory available to
   * applications?
   */

  if (kNaClBadAddress == sysaddr) {
    retval = -NACL_ABI_EFAULT;
    goto cleanup;
  }

  retval = NaClGetTimeOfDay(&now);
  if (0 == retval) {
    /*
     * To make it harder to distinguish Linux platforms from Windows,
     * coarsen the time to the same level we get on Windows -
     * milliseconds, unless in "debug" mode.
     */
    if (!NaClHighResolutionTimerEnabled()) {
      now.nacl_abi_tv_usec = (now.nacl_abi_tv_usec / 1000) * 1000;
    }
    *(struct nacl_abi_timeval *) sysaddr = now;
  }
cleanup:
  NaClSysCommonThreadSyscallLeave(natp);
  return retval;
}

/*
 * TODO(bsy): REMOVE THIS AND PROVIDE GETRUSAGE.  This is normally
 * not a syscall; instead, it is a library routine on top of
 * getrusage, with appropriate clock tick translation.
 */
int32_t NaClSysClock(struct NaClAppThread *natp) {
  int32_t retval;

  NaClSysCommonThreadSyscallEnter(natp);
  retval = clock();
  NaClSysCommonThreadSyscallLeave(natp);
  return retval;
}

#if defined(HAVE_SDL)

int32_t NaClSysMultimedia_Init(struct NaClAppThread *natp,
                               int                  subsys) {
  /* tail call, should compile to a jump */
  return NaClCommonSysMultimedia_Init(natp, subsys);
}

int32_t NaClSysMultimedia_Shutdown(struct NaClAppThread *natp) {
  return NaClCommonSysMultimedia_Shutdown(natp);
}

int32_t NaClSysVideo_Init(struct NaClAppThread *natp,
                          int                  width,
                          int                  height) {
  /* tail call, should compile to a jump */
  return NaClCommonSysVideo_Init(natp, width, height);
}

int32_t NaClSysVideo_Shutdown(struct NaClAppThread *natp) {
  return NaClCommonSysVideo_Shutdown(natp);
}

int32_t NaClSysVideo_Update(struct NaClAppThread *natp,
                            unsigned char        *data) {
  /* tail call, should compile to a jump */
  return NaClCommonSysVideo_Update(natp, data);
}

int32_t NaClSysVideo_Poll_Event(struct NaClAppThread *natp,
                                union NaClMultimediaEvent *event) {
  return NaClCommonSysVideo_Poll_Event(natp, event);
}

int32_t NaClSysAudio_Init(struct NaClAppThread *natp,
                          enum NaClAudioFormat format,
                          int                  desired_samples,
                          int                  *obtained_samples) {
  return NaClCommonSysAudio_Init(natp, format,
                                 desired_samples, obtained_samples);
}


int32_t NaClSysAudio_Stream(struct NaClAppThread *natp,
                            const void           *data,
                            size_t               *size) {
  return NaClSliceSysAudio_Stream(natp, data, size);
}


int32_t NaClSysAudio_Shutdown(struct NaClAppThread *natp) {
  return NaClCommonSysAudio_Shutdown(natp);
}


#endif /* HAVE_SDL */

int32_t NaClSysSrpc_Get_Fd(struct NaClAppThread *natp) {
  extern int NaClSrpcFileDescriptor;

  return NaClSrpcFileDescriptor;
}

int32_t NaClSysImc_MakeBoundSock(struct NaClAppThread *natp,
                                 int                  *sap) {
  return NaClCommonSysImc_MakeBoundSock(natp, sap);
}

int32_t NaClSysImc_Accept(struct NaClAppThread  *natp,
                          int                   d) {
  return NaClCommonSysImc_Accept(natp, d);
}

int32_t NaClSysImc_Connect(struct NaClAppThread *natp,
                           int                  d) {
  return NaClCommonSysImc_Connect(natp, d);
}

int32_t NaClSysImc_Sendmsg(struct NaClAppThread *natp,
                       int                  d,
                       struct NaClImcMsgHdr *nimhp,
                       int                  flags) {
  return NaClCommonSysImc_Sendmsg(natp, d, nimhp, flags);
}

int32_t NaClSysImc_Recvmsg(struct NaClAppThread *natp,
                           int                  d,
                           struct NaClImcMsgHdr *nimhp,
                           int                  flags) {
  return NaClCommonSysImc_Recvmsg(natp, d, nimhp, flags);
}

int32_t NaClSysImc_Mem_Obj_Create(struct NaClAppThread  *natp,
                                  size_t                size) {
  return NaClCommonSysImc_Mem_Obj_Create(natp, size);
}

int32_t NaClSysTls_Init(struct NaClAppThread  *natp,
                        void                  *tdb,
                        size_t                size) {
  return NaClCommonSysTls_Init(natp, tdb, size);
}

int32_t NaClSysThread_Create(struct NaClAppThread *natp,
                             void                 *eip,
                             void                 *esp,
                             void                 *tdb,
                             size_t               tdb_size) {
  return NaClCommonSysThread_Create(natp, eip, esp, tdb, tdb_size);
}

/* mutex */

int32_t NaClSysMutex_Create(struct NaClAppThread *natp) {
  return NaClCommonSysMutex_Create(natp);
}

int32_t NaClSysMutex_Lock(struct NaClAppThread *natp,
                          int32_t              mutex_handle) {
  return NaClCommonSysMutex_Lock(natp, mutex_handle);
}

int32_t NaClSysMutex_Unlock(struct NaClAppThread *natp,
                            int32_t              mutex_handle) {
  return NaClCommonSysMutex_Unlock(natp, mutex_handle);
}

int32_t NaClSysMutex_Trylock(struct NaClAppThread *natp,
                             int32_t              mutex_handle) {
  return NaClCommonSysMutex_Trylock(natp, mutex_handle);
}


/* condition variable */

int32_t NaClSysCond_Create(struct NaClAppThread *natp) {
  return NaClCommonSysCond_Create(natp);
}

int32_t NaClSysCond_Wait(struct NaClAppThread *natp,
                         int32_t              cond_handle,
                         int32_t              mutex_handle) {
  return NaClCommonSysCond_Wait(natp, cond_handle, mutex_handle);
}

int32_t NaClSysCond_Signal(struct NaClAppThread *natp,
                           int32_t              cond_handle) {
  return NaClCommonSysCond_Signal(natp, cond_handle);
}

int32_t NaClSysCond_Broadcast(struct NaClAppThread *natp,
                              int32_t              cond_handle) {
  return NaClCommonSysCond_Broadcast(natp, cond_handle);
}

int32_t NaClSysCond_Timed_Wait_Abs(struct NaClAppThread     *natp,
                                   int32_t                  cond_handle,
                                   int32_t                  mutex_handle,
                                   struct nacl_abi_timespec *ts) {
  return NaClCommonSysCond_Timed_Wait_Abs(natp,
                                          cond_handle,
                                          mutex_handle,
                                          ts);
}

int32_t NaClSysImc_SocketPair(struct NaClAppThread  *natp,
                              int32_t               *d_out) {
  return NaClCommonSysImc_SocketPair(natp, d_out);
}
int32_t NaClSysSem_Create(struct NaClAppThread *natp,
                          int32_t              init_value) {
  return NaClCommonSysSem_Create(natp, init_value);
}

int32_t NaClSysSem_Wait(struct NaClAppThread *natp,
                        int32_t              sem_handle) {
  return NaClCommonSysSem_Wait(natp, sem_handle);
}

int32_t NaClSysSem_Post(struct NaClAppThread *natp,
                        int32_t              sem_handle) {
  return NaClCommonSysSem_Post(natp, sem_handle);
}

int32_t NaClSysSem_Get_Value(struct NaClAppThread *natp,
                             int32_t              sem_handle) {
  return NaClCommonSysSem_Get_Value(natp, sem_handle);
}

int32_t NaClSysSched_Yield(struct NaClAppThread *natp) {
  sched_yield();
  return 0;
}

int32_t NaClSysSysconf(struct NaClAppThread *natp,
                       int32_t name,
                       int32_t *result) {
  int32_t         retval = -NACL_ABI_EINVAL;
  static int32_t  number_of_workers = -1;
  uintptr_t       sysaddr;

  NaClSysCommonThreadSyscallEnter(natp);

  sysaddr = NaClUserToSysAddrRange(natp->nap,
                                   (uintptr_t) result,
                                   sizeof(*result));
  if (kNaClBadAddress == sysaddr) {
    retval = -NACL_ABI_EINVAL;
    goto cleanup;
  }

  switch (name) {
#ifdef _SC_NPROCESSORS_ONLN
    case NACL_ABI__SC_NPROCESSORS_ONLN: {
      if (-1 == number_of_workers) {
        number_of_workers = sysconf(_SC_NPROCESSORS_ONLN);
      }
      if (-1 == number_of_workers) {
        /* failed to get the number of processors */
        retval = -NACL_ABI_EINVAL;
        goto cleanup;
      }
      *(int32_t*)sysaddr = number_of_workers;
      break;
    }
#endif
    default:
      retval = -NACL_ABI_EINVAL;
      goto cleanup;
  }
  retval = 0;
cleanup:
  NaClSysCommonThreadSyscallLeave(natp);
  return retval;
}
