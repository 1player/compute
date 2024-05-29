#ifndef BUILTINS_H
#define BUILTINS_H

#include "lib.h"
#include "object.h"

typedef struct String {
  object _o;
  size_t len; // Doesn't include the '\0' at the end
  char buf[]; // NULL-terminated
} String;

void string_bootstrap(object *scope);
void native_integer_bootstrap(object *scope);
void boolean_bootstrap(object *scope);
void runtime_bootstrap(object *scope);
void closure_bootstrap(object *scope);

object *string_new(char *buf);
object *native_integer_new(intptr_t number);
object *closure_new(array_t *arg_names, expr_t *body, object *scope);

extern object *the_NativeInteger;
extern object *singleton_true;
extern object *singleton_false;

#endif
