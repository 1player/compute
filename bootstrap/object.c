#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "builtins.h"
#include "lib.h"

object *the_nil;
object *the_Object;

//

void *bind(object *receiver, object *name) {
  Slot *slot = (Slot *)object_lookup(receiver, name);
  if (!slot) {
    panic("object %p does not respond to name %p", receiver, name);
  }

  return slot;
}

#define PREPARE_SEND(receiver, selector, n_args) ({                     \
      Slot *slot = bind((receiver), (selector));                        \
      if (slot_is_variable(slot)) {                                     \
        return slot->data;                                              \
      }                                                                 \
      if (slot->arguments != (n_args)) {                                \
        panic("expected %d arguments, got %d", (n_args), slot->arguments); \
      }                                                                 \
      slot->data;                                                       \
    })

#define DISPATCH(fn, n_args, next_arg) ({                               \
      object *ret;                                                      \
      switch ((n_args)) {                                               \
      case 0:                                                           \
        ret = fn(receiver);                                             \
        break;                                                          \
      case 1:                                                           \
        ret = fn(receiver, (next_arg));                                 \
        break;                                                          \
      default:                                                          \
        panic("Sending messages with %d arguments not implemented\n", n_args); \
      }                                                                 \
      ret;                                                              \
    })

object *send_(object *receiver, object *selector, int n_args, ...) {
  object *ret = NULL;
  va_list args;
  va_start(args, n_args);

  void *(*fn)() = PREPARE_SEND(receiver, selector, n_args);
  ret = DISPATCH(fn, n_args, va_arg(args, object *));

  va_end(args);
  return ret;
}

object *send_args(object *receiver, object *selector, int n_args, object **args) {
  void *(*fn)() = PREPARE_SEND(receiver, selector, n_args);
  return DISPATCH(fn, n_args, *args++);
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
  if (self == NULL) {
    self = the_nil;
  } else if (IS_NATIVE(self)) {
    self = the_NativeInteger;
  }

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

object *object_inspect(object *self) {
  if (self == NULL) {
    return string_new("nil");
  }

  char *buf;
  asprintf(&buf, "<Object %p>", self);
  object *s = string_new(buf);
  free(buf);
  return s;
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

//

object *root_scope_bootstrap() {
  // Root objects: Object, nil, the symbol table
  the_Object = object_derive(NULL, sizeof(object));
  the_nil = object_derive(NULL, sizeof(object));

  global_symbol_table = (SymbolTable *)object_derive(the_Object, sizeof(SymbolTable));
  global_symbol_table->table = string_table_new();

  object *inspect_s = intern("inspect");
  object_set_method(the_Object, inspect_s, 0, object_inspect);
  object_set_method(the_nil, inspect_s, 0, object_inspect);

  // Set up root scope
  object *root_scope = object_derive(the_Object, sizeof(object));
  object_set_variable(root_scope, intern("scope"), root_scope);
  object_set_variable(root_scope, intern("nil"), NULL);

  // Set up builtins
  native_integer_bootstrap(root_scope);
  string_bootstrap(root_scope);
  boolean_bootstrap(root_scope);

  return root_scope;
}
