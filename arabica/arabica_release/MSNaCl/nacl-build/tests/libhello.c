
#include <stdio.h>


void foo(void)
{
  printf("Hello world, in libhello\n");
}

void hello(void)
{
  /* Calls via PLT */
  foo();
}
