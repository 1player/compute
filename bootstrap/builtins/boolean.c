#include "builtins.h"
#include "lib.h"

object *singleton_true;
object *singleton_false;

object *boolean_inspect(object *self) {
  if (self == singleton_true) {
    return string_new("true");
  } else if (self == singleton_false) {
    return string_new("false");
  }

  panic("Called boolean_inspect on something that ain't a Boolean");
  return NULL;
}

static slot_definition Boolean_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = boolean_inspect },
  { 0 },
};


void boolean_bootstrap(object *scope) {
  trait *Boolean_trait = trait_derive(Object_trait, 0, Boolean_slots);

  singleton_true = object_new(Boolean_trait);
  singleton_false = object_new(Boolean_trait);

  scope_set(scope, intern("true"), singleton_true);
  scope_set(scope, intern("false"), singleton_false);
}
