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

void boolean_bootstrap(object *scope) {
  object *the_Boolean = object_derive(the_Object, sizeof(object));
  singleton_true = object_derive(the_Boolean, sizeof(object));
  singleton_false = object_derive(the_Boolean, sizeof(object));

  object_set_method(the_Boolean, intern("inspect"), 0, boolean_inspect);

  object_set_variable(scope, intern("true"), singleton_true);
  object_set_variable(scope, intern("false"), singleton_false);
}
