#include <stdio.h>
#include "object.h"
#include "builtins.h"

trait *NativeInteger_trait;

object *native_integer_new(intptr_t number) {
  return (object *)TO_NATIVE(number);
}

HANDLER(NativeInteger__plus, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) + FROM_NATIVE(other));
}

HANDLER(NativeInteger__minus, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) - FROM_NATIVE(other));
}

HANDLER(NativeInteger__multiply, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) * FROM_NATIVE(other));
}

HANDLER(NativeInteger__divide, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) / FROM_NATIVE(other));
}

HANDLER(NativeInteger__mod, NativeInteger other) {
  return TO_NATIVE(FROM_NATIVE(self) % FROM_NATIVE(other));
}

HANDLER(NativeInteger__lt, NativeInteger other) {
  return FROM_NATIVE(self) < FROM_NATIVE(other) ? singleton_true : singleton_false;
}

HANDLER(NativeInteger__lte, NativeInteger other) {
  return FROM_NATIVE(self) <= FROM_NATIVE(other) ? singleton_true : singleton_false;
}

HANDLER(NativeInteger__gt, NativeInteger other) {
  return FROM_NATIVE(self) > FROM_NATIVE(other) ? singleton_true : singleton_false;
}

HANDLER(NativeInteger__gte, NativeInteger other) {
  return FROM_NATIVE(self) >= FROM_NATIVE(other) ? singleton_true : singleton_false;
}

HANDLER(NativeInteger__inspect) {
  char *buf;
  asprintf(&buf, "%ld", FROM_NATIVE(self));
  return string_new(buf);
}

static slot_definition NativeInteger_slots[] = {
  { .type = METHOD_SLOT, .selector = "+", .value = NativeInteger__plus },
  { .type = METHOD_SLOT, .selector = "-", .value = NativeInteger__minus },
  { .type = METHOD_SLOT, .selector = "*", .value = NativeInteger__multiply },
  { .type = METHOD_SLOT, .selector = "/", .value = NativeInteger__divide },
  { .type = METHOD_SLOT, .selector = "%", .value = NativeInteger__mod },
  { .type = METHOD_SLOT, .selector = "<", .value = NativeInteger__lt },
  { .type = METHOD_SLOT, .selector = "<=", .value = NativeInteger__lte },
  { .type = METHOD_SLOT, .selector = ">", .value = NativeInteger__gt },
  { .type = METHOD_SLOT, .selector = ">=", .value = NativeInteger__gte },
  { .type = METHOD_SLOT, .selector = "inspect", .value = NativeInteger__inspect },
  { 0 },
};


void native_integer_bootstrap(object *scope) {
  (void)scope;

  NativeInteger_trait = trait_derive(Object_trait, 0, NativeInteger_slots);
}
