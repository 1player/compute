#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "lib.h"
#include "object.h"
#include "builtins.h"

VTable *vtable_vt;
VTable *object_vt;
VTable *native_integer_vt;

//

VTable *vtable_delegated(VTable *self, size_t object_size) {
  VTable *child = (VTable *)malloc(sizeof(VTable));
  child->_vtable = self ? self->_vtable : NULL;
  child->parent = self;
  child->object_size = object_size;
  child->len = 0;
  child->cap = 0;
  child->names = NULL;
  child->ptrs = NULL;

  return child;
}

Object *vtable_allocate(VTable *self) {
  Object *obj = calloc(1, self->object_size);
  obj->_vtable = self;
  return obj;
}

void vtable_add_method(VTable *self, Object *name_, void *ptr) {
  String *name = (String *)name_;

  // Replace an existing entry, if any
  for (size_t i = 0; i < self->len; i++) {
    String *this_name = (String *)self->names[i];
    if (strncmp(name->buf, this_name->buf, name->len) == 0) {
      self->ptrs[i] = ptr;
      return;
    }
  }

  // Resize arrays, if necessary
  if (self->len == self->cap) {
    size_t new_cap = max(self->cap * 2, 2u);
    self->names = realloc(self->names, new_cap * sizeof(*self->names));
    self->ptrs = realloc(self->ptrs, new_cap * sizeof(*self->ptrs));
    self->cap = new_cap;
  }

  self->names[self->len] = name_;
  self->ptrs[self->len] = ptr;
  self->len++;
}

void vtable_add_method_descriptors(VTable *self, method_descriptor_t *desc) {
  while (desc && desc->name) {
    vtable_add_method(self, string_new(desc->name), desc->fn);
    desc++;
  }
}

void *vtable_lookup(VTable *self, char *selector) {
  for (size_t i = 0; i < self->len; i++) {
    String *this_name = (String *)self->names[i];
    if (strncmp(selector, this_name->buf, this_name->len) == 0) {
      return self->ptrs[i];
    }
  }

  // If nothing found, check the parent
  if (self->parent) {
    return vtable_lookup(self->parent, selector);
  }

  return NULL;
}

Object *object_inspect(Object *self) {
  char *buf;
  asprintf(&buf, "<Object %p>", self);
  return string_new(buf);
}

method_descriptor_t Object_methods[] = {
  { .name = "inspect", .fn = object_inspect },
  { NULL },
};


//

static void *bind(Object *receiver, char *selector) {
  VTable *vtable;

  if (IS_NATIVE(receiver)) {
    vtable = native_integer_vt;
  } else {
    vtable = receiver->_vtable;
  }

  return vtable_lookup(vtable, selector);
}

#define DISPATCH(ret, ptr, n_args, next_arg) do {    \
    switch ((n_args)) {                      \
    case 0:                                  \
      Object *(*fn0)(Object *) = (ptr);      \
      ret = fn0(receiver);                   \
      break;                                 \
    case 1:                                         \
      Object *(*fn1)(Object *, Object *) = (ptr);   \
      ret = fn1(receiver, (next_arg));              \
      break;                                        \
    default:                                                            \
      fprintf(stderr, "Sending messages with %d arguments not implemented\n", n_args); \
    }                                                                   \
  } while(0)

Object *_send(Object *receiver, char *selector, int n_args, ...) {
  void *ptr = bind(receiver, selector);
  if (!ptr) {
    fprintf(stderr, "Object %p doesn't know how to respond to '%s'\n", receiver, selector);
    return NULL;
  }

  va_list args;
  va_start(args, n_args);

  Object *ret = NULL;
  DISPATCH(ret, ptr, n_args, va_arg(args, Object *));

  va_end(args);
  return ret;
}

Object *send_args(Object *receiver, char *selector, array_t *args) {
  void *ptr = bind(receiver, selector);
  if (!ptr) {
    fprintf(stderr, "Object %p doesn't know how to respond to '%s'\n", receiver, selector);
    return NULL;
  }

  Object *ret = NULL;
  int i = 0;
  DISPATCH(ret, ptr, args->size, args->elements[i++]);

  return ret;
}

//

Object *bootstrap() {
  // Core objects: VTable, Object and String
  vtable_vt = vtable_delegated(NULL, sizeof(VTable));
  vtable_vt->_vtable = vtable_vt;

  object_vt = vtable_delegated(NULL, sizeof(Object));
  object_vt->_vtable = vtable_vt;
  vtable_vt->parent = object_vt;

  VTable *string_vt = string_bootstrap();

  vtable_add_method_descriptors(object_vt, Object_methods);

  // Builtins
  native_integer_vt = native_integer_bootstrap();
  VTable *tuple_vt = tuple_bootstrap();
  VTable *scope_vt = scope_bootstrap();

  // Global scope
  Scope *global_scope = scope_new();

#define GLOBAL_SCOPE(name, obj) scope_add(global_scope, (String *)string_new((name)), (Object *)(obj))

  // Classes
  GLOBAL_SCOPE("VTable", vtable_vt);
  GLOBAL_SCOPE("Object", object_vt);
  GLOBAL_SCOPE("String", string_vt);
  GLOBAL_SCOPE("NativeInteger", native_integer_vt);
  GLOBAL_SCOPE("Tuple", tuple_vt);
  GLOBAL_SCOPE("Scope", scope_vt);

  // Singletons
  GLOBAL_SCOPE("scope", global_scope);

  return (Object *)global_scope;
}
