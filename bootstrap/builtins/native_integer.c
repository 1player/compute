#include <stdio.h>
#include "object.h"
#include "builtins.h"

VTable *native_integer_vt;

Object *native_integer_new(intptr_t number) {
  return (Object *)TO_NATIVE(number);
}

Object *native_integer_plus(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) + FROM_NATIVE(other));
}

Object *native_integer_minus(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) - FROM_NATIVE(other));
}

Object *native_integer_multiply(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) * FROM_NATIVE(other));
}

Object *native_integer_divide(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) / FROM_NATIVE(other));
}

Object *native_integer_mod(NativeInteger self, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) % FROM_NATIVE(other));
}

Object *native_integer_inspect(NativeInteger self) {
  char *buf;
  asprintf(&buf, "%ld", FROM_NATIVE(self));
  return string_new(buf);
}

static method_descriptor_t NativeInteger_methods[] = {
  { .name = "+",       .fn = native_integer_plus },
  { .name = "-",       .fn = native_integer_minus },
  { .name = "*",       .fn = native_integer_multiply },
  { .name = "/",       .fn = native_integer_divide },
  { .name = "%",       .fn = native_integer_mod },
  { .name = "inspect", .fn = native_integer_inspect },
  { NULL },
};

void native_integer_bootstrap(Scope *scope) {
  native_integer_vt = vtable_delegated(object_vt, 0);
  vtable_add_method_descriptors(native_integer_vt, NativeInteger_methods);

  scope_add(scope, symbol_intern("NativeInteger"), (Object *)native_integer_vt);
}
