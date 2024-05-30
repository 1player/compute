#include <stdio.h>
#include "object.h"
#include "builtins.h"

trait *NativeInteger_trait;

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

static slot_definition NativeInteger_slots[] = {
  { .type = METHOD_SLOT, .selector = "+", .value = native_integer_plus },
  { .type = METHOD_SLOT, .selector = "-", .value = native_integer_minus },
  { .type = METHOD_SLOT, .selector = "*", .value = native_integer_multiply },
  { .type = METHOD_SLOT, .selector = "/", .value = native_integer_divide },
  { .type = METHOD_SLOT, .selector = "%", .value = native_integer_mod },
  { .type = METHOD_SLOT, .selector = "inspect", .value = native_integer_inspect },
  { 0 },
};


void native_integer_bootstrap(object *scope) {
  (void)scope;

  NativeInteger_trait = trait_derive(Object_trait, 0, NativeInteger_slots);
}
