#include <stdlib.h>
#include "builtins.h"

typedef struct Scope {
  VTable *_vtable;
  Scope *parent;
  array_t *keys;
  array_t *values;
} Scope;

static VTable *scope_vt;

Scope *scope_new(Scope *parent) {
  Scope *scope = (Scope *)vtable_allocate(scope_vt);
  scope->parent = parent;
  scope->keys = array_new();
  scope->values = array_new();
  return scope;
}

Scope *scope_add(Scope *self, Object *name, Object *obj) {
   // Check if an entry with this name already exists
  for (int i = 0; i < self->keys->size; i++) {
    if (self->keys->elements[i] == name) {
      self->values->elements[i] = obj;
      return self;
    }
  }

  array_append(self->keys, name);
  array_append(self->values, obj);
  return self;
}

Object *scope_lookup(Scope *self, Object *name, bool *found) {
  for (int i = 0; i < self->keys->size; i++) {
    if (self->keys->elements[i] == name) {
      *found = true;
      return self->values->elements[i];
    }
  }

  if (self->parent) {
    return scope_lookup(self->parent, name, found);
  }

  *found = false;
  return NULL;
}

static method_descriptor_t Scope_methods[] = {
  { NULL },
};

void scope_bootstrap() {
  scope_vt = vtable_delegated(object_vt, sizeof(Scope));
  vtable_add_method_descriptors(scope_vt, Scope_methods);
}
