#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "builtins.h"
#include "lib.h"

typedef struct Symbol {
  size_t handle;
} Symbol;

trait *Trait_trait;
trait *Object_trait;
trait *Symbol_trait;
trait *nil_trait;

object *inspect_s;

string_table_t *global_symbol_table;


char *inspect(object *o) {
  String *s = (String *)send(o, inspect_s);
  return s->buf;
}


object *intern(char *string) {
  assert (Symbol_trait != NULL);

  location_t loc;
  Symbol *sym;

  if (string_table_lookup(global_symbol_table, string, &loc)) {
    sym = (Symbol *)string_table_get(global_symbol_table, loc);
  } else {
    sym = (Symbol *)object_new(Symbol_trait);
    sym->handle = string_table_set(global_symbol_table, loc, string, sym);
  }

  return (object *)sym;
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

object *object_is(object *self, object *other) {
  if (self == other) {
    return singleton_true;
  }
  return singleton_false;
}


object *symbol_inspect(Symbol *self) {
  char *buf;
  asprintf(&buf, "#%s", string_table_get_key(global_symbol_table, self->handle));
  object *s = string_new(buf);
  free(buf);
  return s;
}

static object *object_alloc(trait *_trait, size_t data_size) {
  void **m = calloc(1, sizeof(trait *) + data_size);
  object *o = (object *)(m + 1);

  TRAIT(o) = _trait;
  return o;
}

// Allocate a new Trait, but without initialising any of the selectors or slots
static trait *trait_alloc(trait *parent, size_t data_size, size_t n_slots) {
  trait *child = (trait *)object_alloc(parent ? TRAIT(parent) : NULL, sizeof(trait));
  child->parent = parent;
  child->n_slots = n_slots;
  child->data_size = data_size;

  if (n_slots > 0) {
    child->selectors = calloc(n_slots, sizeof(object *));
    child->slots = calloc(n_slots, sizeof(object *));
  }

  return child;
}

// Set the slots of a Trait from slot definitions.
// WARNING: Traits are supposed to be immutable. Do not use this function to
// replace the slots of an existing MetaObject.
static void trait_set_slots(trait *self, size_t n_slots, slot_definition defs[]) {
  assert(self->n_slots == n_slots);

  for (size_t i = 0; i < n_slots; i++) {
    self->selectors[i] = intern(defs[i].selector);

    if (defs[i].type == METHOD_SLOT) {
      self->slots[i] = defs[i].value;
    } else {
      panic("Unimplemented setting DATA_SLOTS");
    }
  }
}

static size_t count_slots(slot_definition defs[]) {
  if (!defs) {
    return 0;
  }

  size_t n_slots = 0;
  slot_definition *ptr = defs;
  while (ptr->type) {
    n_slots++;
    ptr++;
  }

  return n_slots;
}

// Create a new Trait
trait *trait_derive(trait *parent, size_t data_size, slot_definition defs[]) {
  if (parent && parent->data_size > 0 && data_size > 0) {
    panic("Deriving from stateful traits is unimplemented.");
  }

  size_t n_slots = count_slots(defs);

  trait *child = trait_alloc(parent, data_size, n_slots);
  trait_set_slots(child, n_slots, defs);
  return child;
}

object *object_new(trait *_trait) {
  // TODO: make sure trait is complete
  // TODO: calculate the allocation size based on the sum of our traits' sizes

  object *o = object_alloc(_trait, _trait->data_size);

  // TODO: initialize traits

  return o;
}

object *trait_lookup(trait *self, object *name) {
  trait *t = self;

  while (t) {
    for (size_t pos = 0; pos < t->n_slots; pos++) {
      if (t->selectors[pos] == name) {
        return t->slots[pos];
      }
    }

    t = self->parent;
  }


  return NULL;
}


void *bind(object *receiver, object *name) {
  trait *_trait;

  if (receiver == NULL) {
    _trait = nil_trait;
  } else if (IS_NATIVE(receiver)) {
    _trait = NativeInteger_trait;
  } else {
    _trait = TRAIT(receiver);
  }

  // TODO: we should delegate this operation by sending a `lookup` message
  // as suggested by Piumarta
  object *value = trait_lookup(_trait, name);
  if (!value) {
    panic("%s does not respond to %s", inspect(receiver), inspect(name));
  }

  return value;
}

#define PREPARE_SEND(receiver, selector, n_args) ({ \
      void *value = bind((receiver), (selector));   \
      if (IS_NATIVE(value)) {                       \
        panic("Unimplemented fetching data slot."); \
      }                                             \
      value;                                        \
    })

#define DISPATCH(fn, n_args, next_arg) ({                               \
      object *ret;                                                      \
      object *arg1, *arg2, *arg3;                                       \
      switch ((n_args)) {                                               \
      case 0:                                                           \
        ret = fn(receiver);                                             \
        break;                                                          \
      case 1:                                                           \
        ret = fn(receiver, (next_arg));                                 \
        break;                                                          \
      case 2:                                                           \
        arg1 = (next_arg);                                              \
        arg2 = (next_arg);                                              \
        ret = fn(receiver, arg1, arg2);                                 \
        break;                                                          \
      case 3:                                                           \
        arg1 = (next_arg);                                              \
        arg2 = (next_arg);                                              \
        arg3 = (next_arg);                                              \
        ret = fn(receiver, arg1, arg2, arg3);                           \
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


static slot_definition Object_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = object_inspect },
  { .type = METHOD_SLOT, .selector = "==",      .value = object_is },
  { .type = METHOD_SLOT, .selector = "===",     .value = object_is },
  { 0 },
};

static slot_definition Symbol_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = symbol_inspect },
  { 0 },
};

static slot_definition nil_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = object_inspect },
  { 0 },
};

object *root_scope_bootstrap() {
  // The Piumarta loop
  Trait_trait = trait_alloc(NULL, sizeof(trait), 0);
  TRAIT(Trait_trait) = Trait_trait;

  Object_trait = trait_alloc(NULL, sizeof(object), count_slots(Object_slots));
  TRAIT(Object_trait) = Trait_trait;
  Trait_trait->parent = Object_trait;

  nil_trait = trait_alloc(NULL, 0, count_slots(nil_slots));
  TRAIT(nil_trait) = Trait_trait;

  // Set up the Symbol trait and symbol table so we can start interning selectors
  Symbol_trait = trait_alloc(Object_trait, sizeof(Symbol), count_slots(Symbol_slots));
  global_symbol_table = string_table_new();

  // Now intern should work, so fill in the slots of Trait, Object, and Symbol
  inspect_s = intern("inspect");
  trait_set_slots(Object_trait, count_slots(Object_slots), Object_slots);
  trait_set_slots(Symbol_trait, count_slots(Symbol_slots), Symbol_slots);
  trait_set_slots(nil_trait, count_slots(nil_slots), nil_slots);

  // Create the root scope
  object *root_scope = scope_bootstrap();
  scope_set(root_scope, intern("nil"), (object *)NULL);

  // Now bootstrap all the builtin objects
  native_integer_bootstrap(root_scope);
  closure_bootstrap(root_scope);
  string_bootstrap(root_scope);
  boolean_bootstrap(root_scope);
  runtime_bootstrap(root_scope);

  return root_scope;
}
