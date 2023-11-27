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

#include "thirdparty/linkedlist/list.h"
#include "stub_policy.h"
#include <string.h>
#include <pwd.h>
#include <assert.h>

Stub_Policy * Stub_Policy_Node_Ctor(char * method_name, int method_name_len)
{
	Stub_Policy * new_node = malloc( 1 * sizeof(Stub_Policy));

	//malloc space for the copy of the method name
	new_node->method_name = (char *) malloc(method_name_len * sizeof(char)); 

	//copy the class name into the struct, including the null termination
	strncpy(new_node->method_name, method_name, method_name_len);
	
	return new_node;
}

void readStubPolicies()
{
	Stub_Policy * new_node;	

	FILE * in;	

	char method_name[MAX_METHOD_NAME_LEN+1];
	char policy_file_path[50];
	int len;

	//initialize the list
	stub_policies = ListCreate();

	//Our policy file is stored under current user's home directory.
	//So we need to get the directory first
	uid_t uid;
	struct passwd * pw;
	uid = getuid();
	pw = getpwuid(uid);
	strncpy(policy_file_path, pw->pw_dir, 36);
	strncat(policy_file_path, "/.stub.policy", 13);

	if( (in = fopen(policy_file_path, "r")) == NULL)
	{
		// error-handling code 
		printf("Problem opening stub policy file\n");
		fflush(stdout);
		assert(0);
  	}
	else
	{
	  //read the policy from the file
	  while(fgets(method_name, MAX_METHOD_NAME_LEN + 1 ,in ) != NULL)
		{
			len = strlen(method_name);
			//eliminate the newline
			method_name[len -1]='\0';		

			//create a new sandbox policy node
			new_node = Stub_Policy_Node_Ctor(method_name, len);

			//printf("[sandbox_policy.c: readSandboxPolicies:] Adding a new policy for %s\n", new_node->class_name);
	
			//store the node in the sandbox policy list
			ListAppend(stub_policies, new_node);
		}
	}
}
