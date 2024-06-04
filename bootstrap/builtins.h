#ifndef BUILTINS_H
#define BUILTINS_H

#include "lib.h"
#include "object.h"

typedef struct String {
  size_t len; // Doesn't include the '\0' at the end
  char *buf; // NULL-terminated
} String;

object *scope_bootstrap();
void string_bootstrap(object *scope);
void native_integer_bootstrap(object *scope);
void boolean_bootstrap(object *scope);
void runtime_bootstrap(object *scope);

object *string_new(char *buf);
object *native_integer_new(intptr_t number);

typedef struct Scope Scope;

object *scope_derive(object *self);
void scope_set(object *self, object *key, object *value);
object *scope_lookup(object *self, object *key, bool *found);

extern slot_definition Trait_slots[];
extern slot_definition Object_slots[];
extern slot_definition Closure_slots[];
extern slot_definition Symbol_slots[];
extern slot_definition nil_slots[];

extern trait *NativeInteger_trait;
extern object *singleton_true;
extern object *singleton_false;

#endif
