#include "builtins.h"
#include "lib.h"

Object *singleton_true;
Object *singleton_false;

static VTable *boolean_vt;

Object *boolean_inspect(Object *self) {
  if (self == singleton_true) {
    return string_new("true");
  } else if (self == singleton_false) {
    return string_new("false");
  }

  panic("Called boolean_inspect on something that ain't a Boolean");
  return NULL;
}

static method_descriptor_t Boolean_methods[] = {
  { .name = "inspect", .fn = boolean_inspect },
  { NULL },
};

VTable *boolean_bootstrap() {
  boolean_vt = vtable_delegated(vtable_vt, 0);
  vtable_add_method_descriptors(boolean_vt, Boolean_methods);

  singleton_true = vtable_allocate(boolean_vt);
  singleton_false = vtable_allocate(boolean_vt);

  return boolean_vt;
}