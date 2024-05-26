#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"

typedef struct Symbol {
  VTable *_vtable;
  char *string;
} Symbol;

VTable *symbol_bootstrap();
Object *symbol_intern(char *string);

//

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
Scope *scope_add(Scope *self, Object *name, Object *obj);
Object *scope_lookup(Scope *self, Object *name, bool *found);

//

Object *native_integer_new(intptr_t number);
VTable *native_integer_bootstrap();

//

VTable *boolean_bootstrap();
extern Object *singleton_true;
extern Object *singleton_false;

#endif
