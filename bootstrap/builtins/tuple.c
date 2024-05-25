#include "builtins.h"

static VTable *tuple_vt;

Object *tuple_new(Object *left, Object *right) {
  Tuple *t = (Tuple *)vtable_allocate(tuple_vt);
  t->left = left;
  t->right = right;
  return (Object *)t;
}

static method_descriptor_t Tuple_methods[] = {
  { NULL },
};

VTable *tuple_bootstrap() {
  tuple_vt = vtable_delegated(vtable_vt, sizeof(Tuple));
  vtable_add_method_descriptors(tuple_vt, Tuple_methods);

  return tuple_vt;
}
