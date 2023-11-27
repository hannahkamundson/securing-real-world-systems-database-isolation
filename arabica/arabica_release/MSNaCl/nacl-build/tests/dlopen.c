
#include <stdlib.h>
#include <assert.h>
#include <dlfcn.h>

int main()
{
  void *handle = dlopen("tests/libhello.so", RTLD_LAZY);
  assert(handle != NULL);
  void (*func)(void) = dlsym(handle, "hello");
  func();
  dlclose(handle);
  return 0;
}
