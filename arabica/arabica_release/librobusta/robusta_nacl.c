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

#include "librobusta.h"
#include "robusta_common.h"
#include "thirdparty/linkedlist/list.h"
#include "JavaTypeParser.h"
#include "robusta_jni_checks.h"
#include "native_client/src/trusted/platform/nacl_log.h"

robusta_jmethodID * methodID_node_Ctor(JNIEnv * env, jclass clazz, jmethodID mid, const char * sys_sig)
{
	int count=0;
	JavaType * tmp;

	//create the struct to insert into the list
	robusta_jmethodID * new_node = (robusta_jmethodID *) malloc( 1 * sizeof(robusta_jmethodID));
	new_node->__mid = mid;
	//NaClLog(0, "[robusta_nacl.c: methodID_node_Ctor:] The signature is: %s\n", sys_sig);
	new_node->__tysig = parsecompletemethodtype(sys_sig);

	jobject methodobj = (*env)->ToReflectedMethod(env, clazz, mid, JNI_FALSE);
    	ASSERT_BUG(methodobj, "Can't convert to a reflected method.");

    	new_node->__pubmember = access_control(env, methodobj);
	//mes310 here we eliminate the security checks for unknown bugs
 	//new_node

    	(*env)->DeleteLocalRef(env, methodobj);

	//ignore the return typew
	//NaClLog(0, "The first node is: %i\n", (new_node->__tysig)->tk);

	tmp = (new_node->__tysig)->next;

	while(tmp != NULL)
	{
		count++;
		//NaClLog(0, "The next node is: %i\n", tmp->tk);
		tmp = tmp->next;
	}

	new_node->__num_sig_words=count;
	//NaClLog(0, "[robusta_nacl.c: methodID_node_Ctor:] Storing a parameter count of: %i\n", count);
	//printf("[robusta_nacl.c: methodID_node_Ctor:] mid: %p, Storing a parameter count of: %i\n", mid, count);
	//fflush(stdout);

	return new_node;
}

//methodID_list is created in natp during
void robusta_createMethodIDNode(LIST * methodID_list, JNIEnv * env, jclass clazz, jmethodID mid, const char * sys_sig)
{
	robusta_jmethodID * new_node = methodID_node_Ctor(env, clazz, mid, sys_sig);

	ListAppend(methodID_list, new_node);
}

robusta_jfieldID * fieldID_node_Ctor(JNIEnv * env, jclass clazz, jfieldID fid, const char * sys_sig)
{
	robusta_jfieldID * new_node = (robusta_jfieldID *) malloc( 1 * sizeof(robusta_jfieldID));

	new_node->__fid=fid;	

	new_node->__clazz=clazz;	

	new_node->__tysig = parsecompletesimpletype(sys_sig);

	jobject fieldobj = (*env)->ToReflectedField(env, clazz, fid, JNI_TRUE);
	ASSERT_BUG(fieldobj, "Can't convert to a reflected field.");
	new_node->__pubmember = access_control(env, fieldobj);
	(*env)->DeleteLocalRef(env, fieldobj);

	return new_node;
}

void robusta_createFieldIDNode(LIST * field_id_list, JNIEnv * env, jclass clazz, jfieldID fid, const char * sys_sig)
{
	NaClLog(1, "[robusta_nacl.c: createFieldIDNode:] field_id_list: %p, env: %p, clazz: %p, fid: %p, sig: %s\n", field_id_list, env, clazz, fid, sys_sig);
	robusta_jfieldID * new_node = fieldID_node_Ctor(env, clazz, fid, sys_sig);
	
	ListAppend(field_id_list, new_node);
}

int mid_comparator(void *item1, void *item2)
{
	//item1 is a robusta_jmethodID node, item2 is an ordinary jmethodID
	robusta_jmethodID * node = (robusta_jmethodID *) item1;
	jmethodID mid = *((jmethodID *)item2);

	//printf("[robusta_nacl.c: comparator:] node->__mid: %i, mid: %i\n", (int) node->__mid, (int) mid);

	if(node->__mid == mid)
	{
		return 1;
	}
	return 0;
}

//***returns -1 if the methodID was not found***
int robusta_findParameterCount(LIST * method_id_list, jmethodID mid)
{
	/*printf("[robusta_nacl.c: robusta_recordSignature:] mid: %i\n", (int) mid);
	fflush(stdout);*/

	robusta_jmethodID * node = ListSearch(method_id_list, &mid_comparator, &mid);

	if(node != NULL)
	{
		return node->__num_sig_words;
	}

	return -1;
}

int fid_comparator(void *item1, void *item2)
{
	//item1 is a robusta_jmethodID node, item2 is an ordinary jmethodID
	robusta_jfieldID * node = (robusta_jfieldID *) item1;
	jfieldID fid = *((jfieldID *)item2);

	//printf("[robusta_nacl.c: comparator:] node->__mid: %i, mid: %i\n", (int) node->__mid, (int) mid);

	if(node->__fid == fid)
	{
		return 1;
	}
	return 0;
}

void robusta_verifySetFieldOp(LIST * field_id_list, JNIEnv * env, jclass clazz, jfieldID fid)
{
	NaClLog(0,"[robusta_nacl.c: verifySetFieldOp:] field_id_list: %p, env: %p, clazz: %p, fid: %p\n", field_id_list, env, clazz, fid);
	
	robusta_jfieldID * node = ListSearch(field_id_list, &fid_comparator, &fid);

	if(node != NULL)
	{
	        #ifdef ACCESS_CONTROL
		CHECK_ACCESS_FIELD(env, node->__pubmember, node->__clazz, clazz);
		#endif

		NaClLog(0, "[robusta_nacl.c: verifySetFieldOp:] fid: %p is a legal field id.\n", fid);
		return;
	}

	//TODO: failure, abort somehow
	NaClLog(LOG_FATAL,"[robusta_nacl.c: verifySetFieldOp:] *FATAL* Could not find fieldID node.\n");
}

int robusta_verifyOperation(JNIEnv * env, char * path, char * optype)
{
	//NaClLog(0, "[robusta_nacl.c: verifyOperation:] env: %p, path: %s, optype: %s\n", env, path, optype);
	// mes310
	// if the JNI vars are not initialized yet, can't do check... this is for libjava.so
	if(env == NULL || jvm_JNIvars_valid == 0)
	{
		//environment is not yet available, can't do check...
		return 1;
	}

	//NOTE: JNI vars have to be initialized at this point***

	//do the check
	jstring j_path = (*env)->NewStringUTF(env, path);
	jstring j_optype = (*env)->NewStringUTF(env, optype);

	//NaClLog(0, "j_path: %p\n", j_path);
	//NaClLog(0, "j_optype: %p\n", j_optype);

	//mes310
	file_permission = (*env)->FindClass(env, "java/io/FilePermission");

	file_permission_ctor = (*env)->GetMethodID(env, file_permission, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");


	//NaClLog(0, "file_permission: %p, file_permission_ctor: %p, j_path: %p, j_optype: %p\n", file_permission, file_permission_ctor, j_path, j_optype);
	jobject file_permission_inst = (*env)->NewObject(env, file_permission, file_permission_ctor, j_path, j_optype);


	(*env)->CallStaticVoidMethod(env, access_controller, check_permission, file_permission_inst);

	jthrowable exc = (*env)->ExceptionOccurred(env);

	if(exc)
	{
		//TODO: what happens when we get a FilePermissions exception?
		(*env)->ExceptionDescribe(env);
		(*env)->ExceptionClear(env);
		return 0;
	}

	NaClLog(0, "[robusta_nacl.c: verifyOperation:] %s permission granted for: %s\n", optype, path);

	return 1;
}

int desc_comparator(void *item1, void *item2)
{
	//item1 is a file_desc_node, item2 is an ordinary int
	file_desc_node * node = (file_desc_node *) item1;
	int file_desc = *((int*)item2);

	//printf("[robusta_nacl.c: comparator:] node->__mid: %i, mid: %i\n", (int) node->__mid, (int) mid);

	if(node->file_desc == file_desc)
	{
		return 1;
	}
	return 0;
}

file_desc_node * file_desc_node_Ctor(char * path, int fd)
{
	int path_len = strlen(path);

	file_desc_node * new_node = (file_desc_node *) malloc( 1 * sizeof(file_desc_node *));
	char * path_cpy = malloc(path_len);

	strcpy(path_cpy, path);

	new_node->file_desc=fd;

	new_node->path=path_cpy;

	return new_node;
}

void file_desc_node_Dtor(file_desc_node * fdn)
{
	//free the string
	free(fdn->path);

	//free the object
	free(fdn);
}


void robusta_addPath(char * path, int fd)
{
	file_desc_node * node = ListSearch(file_desc_list, &fid_comparator, &fd);

	if(node == NULL)
	{
		//this fd isn't already in the list
		file_desc_node * new_node = file_desc_node_Ctor(path, fd);

		ListAppend(file_desc_list, new_node);
	}
	else
	{
		//error condition?
	}
}

void robusta_removePath(int fd)
{
	NaClLog(0, "[robusta_nacl.c: removePath:] removing fd: %i\n", fd);
	file_desc_node * tmp = ListFirst(file_desc_list);

	while(tmp != NULL && tmp->file_desc != fd)
	{
		tmp = ListNext(file_desc_list);
	}

	if(tmp != NULL)
	{
		//remove it
		tmp = ListRemove(file_desc_list);
		//file_desc_node_Dtor(tmp);
	}
}

char * robusta_findPath(int fd)
{
	file_desc_node * node = ListSearch(file_desc_list, &fid_comparator, &fd);

	if(node == NULL)
	{
		return NULL;
	}

	return node->path;
}
