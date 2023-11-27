#include "thirdparty/linkedlist/list.h"
#include "sandbox_policy.h"
#include <string.h>

Sandbox_Policy * Sandbox_Policy_Node_Ctor(char * class_name, int class_name_len)
{
	Sandbox_Policy * new_node = malloc( 1 * sizeof(Sandbox_Policy));

	//malloc space for the copy of the class name
	new_node->class_name = (char *) malloc(class_name_len * sizeof(char)); 

	//copy the class name into the struct, including the null termination
	strncpy(new_node->class_name, class_name, class_name_len);
	
	return new_node;
}

void readSandboxPolicies()
{
	Sandbox_Policy * new_node;	

	FILE * in;	

	char class_name[MAX_CLASSNAME_LEN+1];
	int len;

	//initialize the list
	sandbox_policies = ListCreate();

	if( (in = fopen("/etc/java/sandbox.policy", "r")) == NULL)
	{
		// error-handling code 
		printf("Problem opening sandbox policy file\n");
  	}
	else
	{
		//read the policy from the file
		while(fgets(class_name, MAX_CLASSNAME_LEN +1 ,in ) != NULL)
		{
			len = strlen(class_name);
			//eliminate the newline
			class_name[len -1]='\0';		

			//create a new sandbox policy node
			new_node = Sandbox_Policy_Node_Ctor(class_name, len);

			//printf("[sandbox_policy.c: readSandboxPolicies:] Adding a new policy for %s\n", new_node->class_name);
	
			//store the node in the sandbox policy list
			ListAppend(sandbox_policies, new_node);
		}
	}
}
