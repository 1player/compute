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

typedef struct Scope Scope;

void scope_bootstrap();
Scope *scope_new(Scope *parent);
Scope *scope_add(Scope *self, Object *name, Object *obj);
Object *scope_lookup(Scope *self, Object *name, bool *found);

//

typedef struct String {
  VTable *_vtable;
  size_t len;
  char *buf;
} String;

void string_bootstrap(Scope *scope);
Object *string_new(char *buf);

//

void native_integer_bootstrap(Scope *scope);
Object *native_integer_new(intptr_t number);
extern VTable *native_integer_vt;

//

void boolean_bootstrap(Scope *scope);
extern Object *singleton_true;
extern Object *singleton_false;

#endif
