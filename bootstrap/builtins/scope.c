#include <assert.h>
#include <stdlib.h>

#include "builtins.h"

typedef struct Scope {
  Scope *parent;
  size_t len;
  size_t cap;
  object **names;
  object **values;
} Scope;

trait *Scope_trait;

// Traverse the list of scopes to find the slot that contains a particular name
// Returns whether the name was found
static bool lookup_name(Scope *scope, object *name, size_t *pos, Scope **owner) {
  Scope *s = scope;

  while (s) {
    for (size_t i = 0; i < s->len; i++) {
      if (s->names[i] != name) {
        continue;
      }

      *pos = i;
      *owner = s;
      return true;
    }
    s = s->parent;
  }

  return false;
}

static void maybe_resize(Scope *self) {
  if (self->len == self->cap) {
    size_t new_cap = max(self->cap * 2, 2u);
    self->names = realloc(self->names, new_cap * sizeof(*self->names));
    self->values = realloc(self->values, new_cap * sizeof(*self->values));
    self->cap = new_cap;
  }
}

// Assigns a value to a variable name.
bool scope_assign(object *self_, object *name_s, object *value, bool is_definition) {
  Scope *self = (Scope *)self_;
  Scope *owner;
  size_t pos;

  if (lookup_name(self, name_s, &pos, &owner)) {
    if (is_definition && owner != self) {
      // We are redefining a variable contained in an outer scope.
      // Redefine it in this scope.
      owner = self;
      pos = self->len;
    } else {
      owner->values[pos] = value;
      return true;
    }
  } else if (!is_definition) {
    // This name does not exist and we're not defining it. Fail.
    return false;
  } else {
    // This name does not exist, add it to ourselves
    owner = self;
    pos = self->len;
  }

  maybe_resize(owner);

  owner->names[owner->len] = name_s;
  owner->values[owner->len] = value;
  owner->len++;

  return true;
}

object *scope_lookup(object *self, object *name_s, bool *found) {
  Scope *owner;
  size_t pos;

  if (!lookup_name((Scope *)self, name_s, &pos, &owner)) {
    *found = false;
    return NULL;
  }

  *found = true;
  return owner->values[pos];
}

object *scope_derive(object *parent_) {
  Scope *parent = (Scope *)parent_;

  Scope *child = (Scope *)object_new(Scope_trait);
  child->parent = parent;

  return (object *)child;
}

HANDLER(Scope__dispatch, object *selector, NativeInteger n_args, object **args) {
  bool found;
  object *o = scope_lookup(self, selector, &found);
  if (!found) {
    panic("%s not found in current scope", inspect(selector));
  }

  if (is_closure(o)) {
    return closure_call((Closure *)o, NULL, NULL, n_args, args);
  }

  return o;
}

slot_definition Scope_slots[] = {
  { .type = METHOD_SLOT, .selector = "__dispatch__", .value = Scope__dispatch, },
  { 0 },
};

object *scope_bootstrap() {
  Scope_trait = trait_derive(Object_trait, sizeof(Scope), Scope_slots);

  object *the_RootScope = scope_derive(NULL);
  return the_RootScope;
}
