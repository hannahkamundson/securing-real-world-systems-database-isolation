#define MAX_CLASSNAME_LEN 100

typedef struct Sandbox_Policy
{
	//the class name that this policy applies to
	char * class_name;

} Sandbox_Policy;

void readSandboxPolicies();

//the global sandbox policy list
LIST * sandbox_policies;
