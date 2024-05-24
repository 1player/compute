#ifndef BUILTINS_H
#define BUILTINS_H

#include "object.h"

typedef struct String String;

Object *string_new(char *buf);
Object *native_integer_new(intptr_t number);
Object *tuple_new(Object *left, Object *right);

bool string_equals(String *self, String *other);

typedef struct Scope Scope;
VTable *scope_bootstrap();
Scope *scope_new();
Scope *scope_add(Scope *self, String *name, Object *obj);
Object *scope_lookup(Scope *self, char *name);

extern method_descriptor_t String_methods[];
extern method_descriptor_t NativeInteger_methods[];

#endif
