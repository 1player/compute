#include "builtins.h"

HANDLER(Runtime__assert, object *left, object *right) {
  (void)self;

  object *cmp = send(left, intern("=="), right);
  if (cmp != singleton_true) {
    panic("Assertion failed: %s == %s", inspect(left), inspect(right));
  }
  return NULL;
}

static slot_definition Runtime_slots[] = {
  { .type = METHOD_SLOT, .selector = "assert", .value = Runtime__assert },
  { 0 },
};

void runtime_bootstrap(object *scope) {
  trait *Runtime_trait = trait_derive(Object_trait, 0, Runtime_slots);
  object *the_Runtime = object_new(Runtime_trait);

  scope_assign(scope, intern("Runtime"), the_Runtime, true);
}
