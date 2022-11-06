#include <3ds.h>
#include "sys.h"
#include <stdio.h>

void __attribute__((noreturn)) fail(char *reason) {
  printf("\nFatal error: %s", reason);
  svcExitProcess();
}
