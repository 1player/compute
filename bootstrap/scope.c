#include <stdlib.h>
#include <string.h>

#include "builtins.h"

typedef struct Scope {
  VTable *_vtable;
  array_t entries; // Array of tuples (String, Object)
} Scope;

static VTable *scope_vt;

Scope *scope_new() {
  Scope *scope = (Scope *)vtable_allocate(scope_vt);
  array_init(&scope->entries);
  return scope;
}

Scope *scope_add(Scope *self, String *name, Object *obj) {
   // Check if an entry with this name already exists
  for (int i = 0; i < self->entries.size; i++) {
    Tuple *t = self->entries.elements[i];
    if (string_equals(name, (String *)t->left)) {
      t->right = obj;
      return self;
    }
  }

  Object *t = tuple_new((Object *)name, (Object *)obj);
  array_append(&self->entries, t);
  return self;
}

Object *scope_lookup(Scope *self, char *name, bool *found) {
  size_t name_len = strlen(name);

  for (int i = 0; i < self->entries.size; i++) {
    Tuple *t = self->entries.elements[i];
    String *entry_name = (String *)t->left;
    if (entry_name->len == name_len && strncmp(entry_name->buf, name, name_len) == 0) {
      *found = true;
      return t->right;
    }
  }

  *found = false;
  return NULL;
}

static method_descriptor_t Scope_methods[] = {
  { NULL },
};

VTable *scope_bootstrap() {
  scope_vt = vtable_delegated(vtable_vt, sizeof(Scope));
  vtable_add_method_descriptors(scope_vt, Scope_methods);

  return scope_vt;
}
