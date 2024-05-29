#include "builtins.h"

object *runtime_assert(object *self, object *left, object *right) {
  (void)self;

  object *cmp = send(left, intern("=="), right);
  if (cmp != singleton_true) {
    panic("Assertion failed: %s == %s", inspect(left), inspect(right));
  }
  return NULL;
}

void runtime_bootstrap(object *scope) {
  object *the_Runtime = object_derive(the_Object, sizeof(object));
  object_set_method(the_Runtime, intern("assert"), 2, runtime_assert);

  object_set_variable(scope, intern("Runtime"), the_Runtime);
}
