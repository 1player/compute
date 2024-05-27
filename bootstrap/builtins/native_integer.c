#include <stdio.h>
#include "object.h"
#include "builtins.h"

object *the_NativeInteger;

object *native_integer_new(intptr_t number) {
  return (object *)TO_NATIVE(number);
}

object *native_integer_plus(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) + FROM_NATIVE(other));
}

object *native_integer_minus(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) - FROM_NATIVE(other));
}

object *native_integer_multiply(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) * FROM_NATIVE(other));
}

object *native_integer_divide(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) / FROM_NATIVE(other));
}

object *native_integer_mod(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) % FROM_NATIVE(other));
}

object *native_integer_inspect(NativeInteger self) {
  char *buf;
  asprintf(&buf, "%ld", FROM_NATIVE(self));
  return string_new(buf);
}

void native_integer_bootstrap(object *scope) {
  (void)scope;

  the_NativeInteger = object_derive(the_Object, sizeof(object));

  object_set_method(the_NativeInteger, intern("+"), 1, native_integer_plus);
  object_set_method(the_NativeInteger, intern("-"), 1, native_integer_minus);
  object_set_method(the_NativeInteger, intern("*"), 1, native_integer_multiply);
  object_set_method(the_NativeInteger, intern("/"), 1, native_integer_divide);
  object_set_method(the_NativeInteger, intern("%"), 1, native_integer_mod);
  object_set_method(the_NativeInteger, intern("inspect"), 0, native_integer_inspect);
}
