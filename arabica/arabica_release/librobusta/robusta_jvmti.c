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

/*
 * jvmti part of Robusta
 *
 * This file defines event callbacks for JVMTI agent.
 * librobusta.so is modified to be an agent library for the JVM.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <jvmti.h>
#include <pthread.h>

#include "librobusta.h"
#include "thirdparty/linkedlist/list.h"
#include "robusta_jvmti.h"

#include "jinn/state.h"
#include "jinn/options.h"
#include "jinn/common.h"
#include "jinn/jnicheck.h"

#include "native_client/src/trusted/platform/nacl_log.h"

#define MAX_POLICY_LEN 100

//mes310
//int counter = 0;

/* variable definitions */
JavaVM* bda_jvm = NULL;                        /* The Java virtual machine */ 
jvmtiEnv* bda_jvmti = NULL;                    /* The JVMTI handle from JVM */
jniNativeInterface* bda_orig_jni_funcs = NULL; /* JNI function table */
int bda_init_finish = 0;                       /* Mark for the
						  completion of init */

/* global natp list */
LIST * gl_natp_list;
pthread_mutex_t gl_natp_list_lock;

/* mark for libzip.so */
int mark = 0;

/* function declarations */
static void JNICALL
robusta_start_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env);

static void JNICALL
robusta_init_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread);

static void JNICALL
robusta_thread_start_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env,
			      jthread thread);

static void JNICALL
robusta_thread_end_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env,
			    jthread thread);

static void JNICALL
robusta_death_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env);

static void JNICALL
robusta_bind_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env,
		      jthread thread, jmethodID method, void * address,
		      void ** new_address_ptr);

// VM Start event callback function
static void JNICALL
robusta_start_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env)
{
  unsigned int nthread;
  nthread = GET_NATIVE_THREADID();
}

// VM initialization event callback function
// This callback is used for the hook robusta_initJNIvars()
static void JNICALL
robusta_init_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread)
{
  unsigned int nthread;
  jvmtiError err;

  nthread = GET_NATIVE_THREADID();

  //Robusta's init JNI vars
  robusta_initJNIvars(jni_env);

  //set c2j jni proxies
  err = (*jvmti_env)->GetJNIFunctionTable(jvmti_env, &bda_orig_jni_funcs);
  assert(err == JVMTI_ERROR_NONE);
  //if (!agent_options.nointerpose) {
    bda_c2j_proxy_install(jvmti_env);
    //}

  if (agent_options.jniassert) {
    bda_jnicheck_init(jni_env);
  }

  if (agent_options.bia) {
    bda_agent_init(jni_env);
  }

  /* enable deferred native proxies during pridomial phase. */
  if (!agent_options.nointerpose) {
    bda_j2c_proxy_deferred_methods_reregister(jvmti_env, jni_env);
  }

  // set the finish bit to 1, the c2j proxy can be used at this time
  bda_init_finish = 1;
}

// Native method bind callback
static void JNICALL robusta_bind_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env,
					  jthread thread, jmethodID method,
					  void * address,
					  void ** new_address_ptr)
{
  //mes310
  //counter++;

  jvmtiPhase phase;
  jvmtiError err;
  char *name, *signature, *generic;
  char *cls_sig, *cls_gen;
  jclass kls;

  err = (*jvmti_env)->GetMethodName(jvmti_env, method, &name,
				    &signature, &generic);
  assert(err == JVMTI_ERROR_NONE);

  err = (*jvmti_env)->GetMethodDeclaringClass(jvmti_env, method, &kls);
  assert(err == JVMTI_ERROR_NONE);

  err = (*jvmti_env)->GetClassSignature(jvmti_env, kls, &cls_sig, &cls_gen);
  assert(err == JVMTI_ERROR_NONE);

  /* Check the policy file */
  // Generate a policy item from class signature and method name

  char p[MAX_POLICY_LEN];
  int i;
  for (i = 0; i < MAX_POLICY_LEN; i++)
    p[i] = '\0';

  // Eliminate the beginning 'L'
  if (strlen(cls_sig) + 1 + strlen(name) < MAX_POLICY_LEN) {
    strncpy(p, &cls_sig[1], strlen(&cls_sig[1]));
    // Rewrite ';' as '/'
    p[strlen(&cls_sig[1]) - 1] = '/';
    strncat(p, name, strlen(name));
  }

  printf("%s\n", p);
  fflush(stdout);
  NaClLog(0, "%s\n", p);
  if (robusta_methodUsesStubLibraries(p)) {
    err = (*jvmti_env)->GetPhase(jvmti_env, &phase);
    assert(err == JVMTI_ERROR_NONE);
    if (!bda_is_agent_native_method(address)) {
      switch(phase) {
      case JVMTI_PHASE_ONLOAD:
      case JVMTI_PHASE_PRIMORDIAL:
	bda_j2c_proxy_add_deferred(method, address);
	break;
      case JVMTI_PHASE_START:
      case JVMTI_PHASE_LIVE: {
	bda_j2c_proxy_add(jvmti_env, jni_env, method, address, new_address_ptr);
	break;
      }
      default:
	assert(0); /* not reachable. */
	break;
      }
    }
  }
}

//Thread start event callback function
static void JNICALL
robusta_thread_start_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env,
			      jthread thread)
{
  /* Write down the thread info */
  jvmtiError err;
  jvmtiPhase phase;
  jvmtiThreadInfo tinfo;
  unsigned int nthread;
  bda_state_id bid;
  struct bda_state_info * s;
  int i;   // For goto usage

  nthread = GET_NATIVE_THREADID();

  err = (*jvmti_env)->GetPhase(jvmti_env, &phase);
  assert(err == JVMTI_ERROR_NONE);
  if (phase != JVMTI_PHASE_LIVE)
    goto natp;

  bid = bda_state_allocate(jni_env);
  s = bda_state_get(bid);
  s->nthreadid = nthread;
  err = (*jvmti_env)->SetThreadLocalStorage(jvmti_env, NULL, (void *) bid);
  assert(err == JVMTI_ERROR_NONE);

  err = (*jvmti_env)->GetThreadInfo(jvmti_env, NULL, &tinfo);
  assert(err == JVMTI_ERROR_NONE);
  assert(tinfo.name != NULL);
  strncpy(s->name, tinfo.name, sizeof(s->name));
  if (sizeof(s->name) > 0)
    s->name[sizeof(s->name) - 1] = '\0';
  err = (*jvmti_env)->Deallocate(jvmti_env, (unsigned char *) tinfo.name);

  /* Analyse and record the natp information */
  // Temporary a statement for goto
 natp: i = 0;
  npitem * n;

  // (mes310) acquire the lock
  pthread_mutex_lock (&gl_natp_list_lock);

  if (gl_natp_list == NULL) {
    gl_natp_list = ListCreate();
  }
  assert(gl_natp_list != NULL);

  n = (npitem *) ListFirst(gl_natp_list);
  if (n != NULL) {
      do {
	  if (n->env == jni_env) {
	    printf("[thread start] env: %p, Same thread, which should not be correct!\n", jni_env);
	    fflush(stdout);
	    // (mes310) release the lock
	    pthread_mutex_unlock (&gl_natp_list_lock);
	    assert(0);
	    return;
	  }
      } while ((n = (npitem *) ListNext(gl_natp_list)) != NULL);
  }

  // Here we cannot find existing record about this thread,
  // so add a new record in the list
  n = (npitem *) malloc(sizeof(npitem));
  assert (n != NULL);

  n->env = jni_env;
  assert(n->env != NULL);

  // for supporting multisandbox, we need to have one natp for each
  // sandbox in each Java thread
  for (i = 0; i < NUM_SANDBOX; i++) {
    NATP_PTR ntp = _Z17robusta_init_natpv();
    assert(ntp != NULL);
    n->natp[i] = ntp;
  } 

  // add the natp item into the list
  ListAppend(gl_natp_list, n);

  // (mes310) release the lock
  pthread_mutex_unlock (&gl_natp_list_lock);
}

static void JNICALL
robusta_thread_end_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env,
			    jthread thread)
{
  jvmtiError err;
  bda_state_id bid;

  struct bda_state_info * s = bda_state_find(jni_env);

  if (s != NULL) {
    err = (*jvmti_env)->GetThreadLocalStorage(jvmti_env, NULL, (void **) &bid);
    assert(err == JVMTI_ERROR_NONE);

    bda_state_free(bid);
    err = (*jvmti_env)->SetThreadLocalStorage(jvmti_env, NULL, NULL);
    assert(err == JVMTI_ERROR_NONE);
  }

  // (mes310) acquire the lock
  pthread_mutex_lock (&gl_natp_list_lock);

  /* original
  // delete the natp item from the list
  npitem * n = (npitem *) ListFirst(gl_natp_list);
  if (n != NULL) {
    do {
      if (n->env == jni_env) {
	//printf("Thread End!\n");
	printf("[thread end] removing env: %p, act env: %p, n: %p\n",
	       n->env, jni_env, n);
	fflush(stdout);
	
	// remove the current node
	ListRemove(gl_natp_list);

	//break;
      }
    } while ((n = (npitem *) ListNext(gl_natp_list)) != NULL);
  }
  // here we must have a valid n
  assert (n != NULL);

  printf("[thread end] removing env: %p, act env: %p, n: %p\n", n->env, jni_env, n);
  fflush(stdout);

  // free the memory space
  int i;
  for (i = 0; i < NUM_SANDBOX; i++) {
    free(n->natp[i]);
  }
  free(n);
  
  // remove the current node
  ListRemove(gl_natp_list);
  */

  // delete the natp item from the list
  npitem * n = (npitem *) ListFirst(gl_natp_list);
  if (n != NULL) {
    do {
      while (n->env == jni_env) {
	//printf("[thread end] removing env: %p, act env: %p, n: %p\n",
	//       n->env, jni_env, n);
	//fflush(stdout);

	// free the memory space
	int i;
	for (i = 0; i < NUM_SANDBOX; i++) {
	  free(n->natp[i]);
	}
	free(n);
	
	// remove the current node
	ListRemove(gl_natp_list);

	// here the current node has been moved to next by ListRemove
	n = (npitem *) ListCurr(gl_natp_list);
      }
    } while ((n = (npitem *) ListNext(gl_natp_list)) != NULL);
  }
  
  //mes310 temp
  /*
  n = (npitem *) ListFirst(gl_natp_list);
  if (n != NULL) {
    do {
      if (n->env == jni_env) {
	printf("[thread end] still exist after removal! n: %p, env: %p\n", n, n->env);
	fflush(stdout);
	// mes310 try to delete all duplicate entries here???
	ListRemove(gl_natp_list);
      }
    } while ((n = (npitem *) ListNext(gl_natp_list)) != NULL);
  }
  */
  // (mes310) release the lock
  pthread_mutex_unlock (&gl_natp_list_lock);
}

static void JNICALL
robusta_death_callback(jvmtiEnv *jvmti_env, JNIEnv *jni_env)
{
  if (agent_options.jniassert)
    bda_jnicheck_exit(jni_env);
  if (agent_options.mcount) {
    bda_j2c_proxy_dump_stat();
    bda_c2j_proxy_dump_stat();
  }
  //printf("number of bind: %d\n", counter);
  //fflush(stdout);
  bda_jvmti = NULL;
}

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *vm, char* options, void* reserved)
{
  // initialize Robusta
  initRobusta();

  // set the finish bit to 0
  bda_init_finish = 0;

  jvmtiError error;
  jvmtiEventCallbacks robustaCallbacks;
  jvmtiCapabilities cap;
  jvmtiEnv *env;
  unsigned int nthread;

  nthread = GET_NATIVE_THREADID();

  assert(bda_jvm == NULL && vm != NULL);
  bda_jvm = vm;

  // initialize the list
  gl_natp_list = ListCreate();

  // mes310 initialize the locks
  pthread_mutex_init(&gl_natp_list_lock, NULL);

  // Parse incoming options
  agent_parse_options(options);

  // Get environment
  (*vm)->GetEnv(vm, (void**)&env, JVMTI_VERSION_1_0);
  assert (env != NULL);

  // Ensure JVMTI agent capabilities. */
  error = (*env)->GetCapabilities(env, &cap);
  assert(error == JVMTI_ERROR_NONE);
  if (!agent_options.nointerpose) {

      if (!cap.can_generate_native_method_bind_events) {
          cap.can_generate_native_method_bind_events = 1;
          error = (*env)->AddCapabilities(env, &cap);
          assert(error == JVMTI_ERROR_NONE);
	  }
      if (!cap.can_get_bytecodes) {
          cap.can_get_bytecodes = 1;
          error = (*env)->AddCapabilities(env, &cap);
          assert(error == JVMTI_ERROR_NONE);
      }
      if (!cap.can_get_line_numbers) {
          cap.can_get_line_numbers = 1;
          error = (*env)->AddCapabilities(env, &cap);
          assert(error == JVMTI_ERROR_NONE);
      }

  }

  // Set capabilities
  memset(&cap, 0, sizeof(jvmtiCapabilities));
	
  // Set callback events
  memset(&robustaCallbacks, 0, sizeof(jvmtiEventCallbacks));
  robustaCallbacks.VMStart = &robusta_start_callback;
  robustaCallbacks.VMInit = &robusta_init_callback;
  robustaCallbacks.ThreadStart = &robusta_thread_start_callback;
  robustaCallbacks.ThreadEnd = &robusta_thread_end_callback;
  robustaCallbacks.VMDeath = &robusta_death_callback;

  if (!agent_options.nointerpose) {
    robustaCallbacks.NativeMethodBind = &robusta_bind_callback;
  }

  error = (*env)->SetEventCallbacks(env, &robustaCallbacks,
				    sizeof(robustaCallbacks));
  assert(error == JVMTI_ERROR_NONE);

  error = (*env)->SetEventNotificationMode(env, JVMTI_ENABLE,
					   JVMTI_EVENT_VM_START, NULL);
  assert(error == JVMTI_ERROR_NONE);

  error = (*env)->SetEventNotificationMode(env, JVMTI_ENABLE,
					   JVMTI_EVENT_VM_INIT, NULL);
  assert(error == JVMTI_ERROR_NONE);

  error = (*env)->SetEventNotificationMode(env, JVMTI_ENABLE,
					   JVMTI_EVENT_THREAD_START, NULL);
  assert(error == JVMTI_ERROR_NONE);

  error = (*env)->SetEventNotificationMode(env, JVMTI_ENABLE,
					   JVMTI_EVENT_THREAD_END, NULL);
  assert(error == JVMTI_ERROR_NONE);

  error = (*env)->SetEventNotificationMode(env, JVMTI_ENABLE,
					   JVMTI_EVENT_VM_DEATH, NULL);
  assert(error == JVMTI_ERROR_NONE);

  if (!agent_options.nointerpose) {
    error = (*env)->SetEventNotificationMode(env, JVMTI_ENABLE,
					     JVMTI_EVENT_NATIVE_METHOD_BIND,
					     NULL);
    assert(error == JVMTI_ERROR_NONE);
  }

  bda_jvmti = env;
  return JNI_OK;
}


JNIEXPORT void JNICALL
Agent_OnUnload(JavaVM* vm)
{
}

/**
 * End of JVMTI section...
 *
 */
