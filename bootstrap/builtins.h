#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"

void string_bootstrap(object *scope);
void native_integer_bootstrap(object *scope);
void boolean_bootstrap(object *scope);

object *string_new(char *buf);
object *native_integer_new(intptr_t number);

extern object *the_NativeInteger;
extern object *singleton_true;
extern object *singleton_false;

#endif
