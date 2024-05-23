#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "lang.h"

void panic(const char *msg, ...) {
  va_list args;

  fputs("PANIC: ", stderr);

  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);

  fputc('\n', stderr);

  exit(1);
}

void info(const char *msg, ...) {
  va_list args;

  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);

  fputc('\n', stderr);
}


void error(const char *file, const int line, const char *msg, va_list args) {
  fprintf(stderr, "%s:%d: ", file, line);
  vfprintf(stderr, msg, args);
  fputc('\n', stderr);
}
