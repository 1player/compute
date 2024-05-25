#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"

typedef struct String {
  VTable *_vtable;
  size_t len;
  char *buf;
} String;

VTable *string_bootstrap();
Object *string_new(char *buf);
bool string_equals(String *self, String *other);

//

typedef struct Scope Scope;

VTable *scope_bootstrap();
Scope *scope_new();
Scope *scope_add(Scope *self, String *name, Object *obj);
Object *scope_lookup(Scope *self, char *name, bool *found);

//

typedef struct Tuple {
  VTable *_vtable;
  Object *left;
  Object *right;
} Tuple;

VTable *tuple_bootstrap();
Object *tuple_new(Object *left, Object *right);

//

Object *native_integer_new(intptr_t number);
VTable *native_integer_bootstrap();

#endif
