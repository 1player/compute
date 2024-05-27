#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "builtins.h"
#include "lib.h"

object *the_Object;

//

void *bind(object *receiver, object *name) {
  Slot *slot = (Slot *)object_lookup(receiver, name);
  if (!slot) {
    panic("object %p does not respond to name %p", receiver, name);
  }

  return slot;
}

#define PREPARE_SEND(n_args) \
  Slot *slot = bind(receiver, name); \
  if (slot_is_variable(slot)) { \
    return slot->data; \
  } \
  if (slot->arguments != 0) { \
    panic("expected %d arguments, got %d", 0, slot->arguments); \
  } \
  void *(*fn)() = slot->data

object *send0(object *receiver, object *name) {
  PREPARE_SEND(0);
  return fn(receiver);
}

object *send1(object *receiver, object *name, object *arg1) {
  PREPARE_SEND(1);
  return fn(receiver, arg1);
}

object *send2(object *receiver, object *name, object *arg1, object *arg2) {
  PREPARE_SEND(1);
  return fn(receiver, arg1, arg2);
}

object *send(object *receiver, object *selector, int n_args, object **args) {
  panic("Unimplemented");
  return NULL;
}

//

object *object_derive(object *self, size_t data_size) {
  assert(data_size >= sizeof(object));

  object *child = calloc(1, data_size);
  child->parent = self;
  return child;
}

object *object_derive_ni(object *self, object *data_size_ni) {
  return object_derive(self, FROM_NATIVE(data_size_ni));
}

static void object_expand_slots(object *self) {
  HALFWORD cap = self->capacity ? self->capacity * 2 : 4;
  self->selectors = realloc(self->selectors, sizeof(WORD) * cap);
  self->slots = realloc(self->slots, sizeof(WORD) * cap);
  self->capacity = cap;
}

object *object_set(object *self, object *name, object *slot) {
  // Find the first available position
  int pos;
  for (pos = 0; pos < self->capacity; pos++) {
    if (!self->selectors[pos]) break;
  }

  if (pos == self->capacity) {
    object_expand_slots(self);
  }

  self->selectors[pos] = name;
  self->slots[pos] = slot;

  return self;
}

object *object_lookup(object *self, object *name) {
  for (HALFWORD pos = 0; pos < self->capacity; pos++) {
    if (self->selectors[pos] == name) {
      return self->slots[pos];
    }
  }

  if (self->parent) {
    return object_lookup(self->parent, name);
  }

  return NULL;
}

object *object_set_variable(object *self, object *name, object *value) {
  return object_set(self, name, slot_for_variable(value));
}

object *object_set_method(object *self, object *name, unsigned int arguments, void *fn) {
  return object_set(self, name, slot_for_method(arguments, fn));
}


//

typedef struct Symbol {
  object _o;
  char *string;
} Symbol;

typedef struct SymbolTable {
  object _o;
  string_table_t *table;
} SymbolTable;

SymbolTable *global_symbol_table;

object *intern(char *string) {
  location_t loc;
  Symbol *sym;

  if (string_table_lookup(global_symbol_table->table, string, &loc)) {
    sym = (Symbol *)string_table_get(global_symbol_table->table, loc);
  } else {
    sym = (Symbol *)object_derive(the_Object, sizeof(Symbol));
    sym->string = string_table_set(global_symbol_table->table, loc, string, sym);
  }

  return (object *)sym;
}

object *intern_function(char *name, int args) {
  if (args < 0) { panic("args < 0"); }
  char *buf;

  asprintf(&buf, "%s.%d", name, args);
  object *sym = intern(buf);
  free(buf);

  return sym;
}

//

object *root_scope_bootstrap() {
  the_Object = object_derive(NULL, sizeof(object));

  global_symbol_table = (SymbolTable *)object_derive(the_Object, sizeof(SymbolTable));
  global_symbol_table->table = string_table_new();

  // Set up root scope
  object *root_scope = object_derive(the_Object, sizeof(object));
  object_set_variable(root_scope, intern("scope"), root_scope);

  // Set up builtins
  native_integer_bootstrap(root_scope);
  string_bootstrap(root_scope);
  boolean_bootstrap(root_scope);

  return root_scope;
}
