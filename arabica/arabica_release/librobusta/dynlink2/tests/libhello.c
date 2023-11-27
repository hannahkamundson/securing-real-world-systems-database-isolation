#include <stdio.h>

void foo(void)
{
  printf("Hello world, in libhello\n");
  fflush(stdout);
}

void hello(void)
{
  /* Calls via PLT */
  foo();
}
