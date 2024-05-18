#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "common.h"

void panic(char *msg, ...) {
  va_list args;

  fputs("PANIC: ", stderr);

  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);

  fputc('\n', stderr);

  exit(1);
}

void error(char *file, int line, char *msg, va_list args) {
  fprintf(stderr, "%s:%d: ", file, line);
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
}