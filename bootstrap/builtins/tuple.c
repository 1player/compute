#include "object.h"

Object *tuple_new(Object *left, Object *right) {
  Tuple *t = (Tuple *)vtable_allocate(tuple_vt);
  t->left = left;
  t->right = right;
  return (Object *)t;
}
