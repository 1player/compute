#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "builtins.h"
#include "lib.h"


typedef struct Closure {
  object *(*entrypoint)();
  void *data;
} Closure;

typedef struct __lookup {
  enum slot_type type;
  Closure *closure;
  size_t offset; // Offset of this trait's data in the object's body
} __lookup;

trait *Trait_trait;
trait *Object_trait;
trait *Symbol_trait;
trait *Closure_trait;
trait *nil_trait;

object *inspect_s; // the symbol "inspect"
object *lookup_s;  // the symbol "lookup"

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
      self->slots[i] = closure_new(defs[i].value, NULL);
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


object *closure_new(void *entrypoint, void *data) {
  Closure *self = (Closure *)object_new(Closure_trait);
  self->entrypoint = entrypoint;
  self->data = data;

  return (object *)self;
}

object *closure_new_interpreted(array_t *arg_names, expr_t *body, object *scope) {
  interpreted_closure_t *i = (interpreted_closure_t *)malloc(sizeof(interpreted_closure_t));
  i->arg_names = arg_names;
  i->body = body;
  i->scope = scope;

  return closure_new(eval_interpreted_closure, i);
}

__lookup *trait_lookup(trait *self, object *name) {
  trait *t = self;
  __lookup *result = malloc(sizeof(__lookup));

  while (t) {
    for (size_t pos = 0; pos < t->n_slots; pos++) {
      if (t->selectors[pos] != name) {
        continue;
      }

      if (IS_NATIVE(t->slots[pos])) {
        result->type = DATA_SLOT;
      } else {
        result->type = METHOD_SLOT;
        result->closure = (Closure *)t->slots[pos];
      }

      goto end;
    }

    result->offset += t->data_size;
    t = t->parent;
  }

  result->type = UNSET_SLOT;

 end:
  return result;
}

object *send_(object *receiver, object *selector, int n_args, ...);

__lookup *bind(object *receiver, object *name) {
  trait *_trait;

  if (receiver == NULL) {
    _trait = nil_trait;
  } else if (IS_NATIVE(receiver)) {
    _trait = NativeInteger_trait;
  } else {
    _trait = TRAIT(receiver);
  }

  if (_trait == Trait_trait) {
    return trait_lookup(_trait, name);
  }

  return (__lookup *)send_((object *)_trait, lookup_s, 1, name);
}

object *send_(object *receiver, object *selector, int n_args, ...) {
  va_list va;
  va_start(va, n_args);

  object **args = (object **)alloca(n_args * sizeof(object *));
  for (int i = 0; i < n_args; i++) {
    args[i] = va_arg(va, object *);
  }

  va_end(va);

  return send_args(receiver, selector, n_args, args);
}

object *send_args(object *rcv, object *sel, int n_args, object **args) {
  __lookup *l = bind(rcv, sel);
  if (l->type == UNSET_SLOT) {
    panic("%s does not respond to %s", inspect(rcv), inspect(sel));
  } else if (l->type == DATA_SLOT) {
    panic("Unimplemented fetching data slot.");
  }

  Closure *closure = l->closure;
  void *trait_data = (char *)&rcv[l->offset];

  free(l);

  // Can we make this better, please?
  object *arg1, *arg2, *arg3;
  switch ((n_args)) {
  case 0:
    return closure->entrypoint(closure->data, trait_data, rcv);
  case 1:
    return closure->entrypoint(closure->data, trait_data, rcv, *args++);
  case 2:
    arg1 = *args++;
    arg2 = *args++;
    return closure->entrypoint(closure->data, trait_data, rcv, arg1, arg2);
  case 3:
    arg1 = *args++;
    arg2 = *args++;
    arg3 = *args++;
    return closure->entrypoint(closure->data, trait_data, rcv, arg1, arg2, arg3);
  }

  panic("Sending messages with %d arguments not implemented\n", n_args);
  return NULL;
}

object *root_scope_bootstrap() {
  // The Piumarta loop
  Trait_trait = trait_alloc(NULL, sizeof(trait), count_slots(Trait_slots));
  TRAIT(Trait_trait) = Trait_trait;

  Object_trait = trait_alloc(NULL, sizeof(object), count_slots(Object_slots));
  TRAIT(Object_trait) = Trait_trait;
  Trait_trait->parent = Object_trait;

  nil_trait = trait_alloc(NULL, 0, count_slots(nil_slots));
  TRAIT(nil_trait) = Trait_trait;

  // Set up the Symbol trait and symbol table so we can start interning selectors
  Symbol_trait = trait_alloc(Object_trait, sizeof(Symbol), count_slots(Symbol_slots));
  global_symbol_table = string_table_new();

  // Set up closures
  Closure_trait = trait_alloc(Object_trait, sizeof(Closure), count_slots(Closure_slots));

  // Now intern and closures should work, so fill in the slots of Trait, Object, and Symbol
  inspect_s = intern("inspect");
  lookup_s = intern("lookup");

  trait_set_slots(Trait_trait, count_slots(Trait_slots), Trait_slots);
  trait_set_slots(Object_trait, count_slots(Object_slots), Object_slots);
  trait_set_slots(Closure_trait, count_slots(Closure_slots), Closure_slots);
  trait_set_slots(Symbol_trait, count_slots(Symbol_slots), Symbol_slots);
  trait_set_slots(nil_trait, count_slots(nil_slots), nil_slots);

  // Create the root scope
  object *root_scope = scope_bootstrap();
  scope_set(root_scope, intern("nil"), (object *)NULL);

  // Now bootstrap all the builtin objects
  native_integer_bootstrap(root_scope);
  string_bootstrap(root_scope);
  boolean_bootstrap(root_scope);
  runtime_bootstrap(root_scope);

  return root_scope;
}
