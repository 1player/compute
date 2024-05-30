#include <stdlib.h>

#include "builtins.h"

typedef struct Scope {
  Scope *parent;
  size_t height; // 0 indicates the root scope
  size_t len;
  size_t cap;
  object **names;
  object **values;
} Scope;

trait *Scope_trait;

void scope_set(object *self_, object *name_s, object *value) {
  Scope *self = (Scope *)self_;

  // Replace an existing entry, if any
  for (size_t i = 0; i < self->len; i++) {
    if (self->names[i] == name_s) {
      self->values[i] = value;
      return;
    }
  }

  // Resize arrays, if necessary
  if (self->len == self->cap) {
    size_t new_cap = max(self->cap * 2, 2u);
    self->names = realloc(self->names, new_cap * sizeof(*self->names));
    self->values = realloc(self->values, new_cap * sizeof(*self->values));
    self->cap = new_cap;
  }

  self->names[self->len] = name_s;
  self->values[self->len] = value;
  self->len++;
}

object *scope_lookup(object *self_, object *name_s, bool *found) {
  Scope *self = (Scope *)self_;

  for (size_t i = 0; i < self->len; i++) {
    if (self->names[i] == name_s) {
      *found = true;
      return self->values[i];
    }
  }

  if (self->height > 0) {
    return scope_lookup((object *)self->parent, name_s, found);
  }

  *found = false;
  return NULL;
}


object *scope_derive(object *parent_) {
  Scope *parent = (Scope *)parent_;

  Scope *child = (Scope *)object_new(Scope_trait);
  child->parent = parent;
  child->height = parent ? parent->height + 1 : 0;

  return (object *)child;
}

object *scope_bootstrap() {
  Scope_trait = trait_derive(Object_trait, sizeof(Scope), NULL);
  object *the_RootScope = scope_derive(NULL);

  return the_RootScope;
}
