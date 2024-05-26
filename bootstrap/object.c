#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "lib.h"
#include "object.h"
#include "builtins.h"

VTable *vtable_vt;
VTable *object_vt;
VTable *nil_vt;

//

VTable *vtable_delegated(VTable *self, size_t object_size) {
  VTable *child = (VTable *)malloc(sizeof(VTable));
  child->_vtable = self ? self->_vtable : NULL;
  child->parent = self;
  child->object_size = object_size;
  child->len = 0;
  child->cap = 0;
  child->selectors = NULL;
  child->ptrs = NULL;

  return child;
}

Object *vtable_allocate(VTable *self) {
  Object *obj = calloc(1, self->object_size);
  obj->_vtable = self;
  return obj;
}

void vtable_add_method(VTable *self, Object *selector, void *ptr) {
  // Replace an existing entry, if any
  for (size_t i = 0; i < self->len; i++) {
    if (self->selectors[i] == selector) {
      self->ptrs[i] = ptr;
      return;
    }
  }

  // Resize arrays, if necessary
  if (self->len == self->cap) {
    size_t new_cap = max(self->cap * 2, 2u);
    self->selectors = realloc(self->selectors, new_cap * sizeof(*self->selectors));
    self->ptrs = realloc(self->ptrs, new_cap * sizeof(*self->ptrs));
    self->cap = new_cap;
  }

  self->selectors[self->len] = selector;
  self->ptrs[self->len] = ptr;
  self->len++;
}

void vtable_add_method_descriptors(VTable *self, method_descriptor_t *desc) {
  while (desc && desc->name) {
    vtable_add_method(self, (Object *)symbol_intern(desc->name), desc->fn);
    desc++;
  }
}

void *vtable_lookup(VTable *self, Object *selector) {
  for (size_t i = 0; i < self->len; i++) {
    if (self->selectors[i] == selector) {
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

Object *object_is(Object *self, Object *other) {
  if (self == other) {
    return singleton_true;
  }
  return singleton_false;
}

method_descriptor_t Object_methods[] = {
  { .name = "==",      .fn = object_is },
  { .name = "===",     .fn = object_is },
  { .name = "inspect", .fn = object_inspect },
  { NULL },
};

Object *nil_inspect(Object *self) {
  (void)self;
  return string_new("nil");
}

Object *nil_is(Object *self) {
  return self == NULL ? singleton_true : singleton_false;
}

method_descriptor_t Nil_methods[] = {
  { .name = "===",     .fn = nil_is },
  { .name = "inspect", .fn = nil_inspect },
  { NULL },
};


//

static void *bind(Object *receiver, Object *selector) {
  VTable *vtable;

  if (receiver == NULL) {
    vtable = nil_vt;
  } else if (IS_NATIVE(receiver)) {
    vtable = native_integer_vt;
  } else {
    vtable = receiver->_vtable;
  }

  void *ptr = vtable_lookup(vtable, selector);
  if (!ptr) {
    Symbol *selector_ = (Symbol *)selector;
    String *i = (String *)send(receiver, "inspect");
    fprintf(stderr, "bind: %*s doesn't know how to respond to '%s'\n",
            (int)i->len, i->buf, selector_->string);
  }

  return ptr;
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
  void *ptr = bind(receiver, symbol_intern(selector));
  if (!ptr) {
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
  void *ptr = bind(receiver, symbol_intern(selector));
  if (!ptr) {
    return NULL;
  }

  Object *ret = NULL;
  int i = 0;
  DISPATCH(ret, ptr, args->size, args->elements[i++]);

  return ret;
}

//

Object *bootstrap() {
  // The core objects: VTable, Object, nil and Symbol
  vtable_vt = vtable_delegated(NULL, sizeof(VTable));
  vtable_vt->_vtable = vtable_vt;

  object_vt = vtable_delegated(NULL, sizeof(Object));
  object_vt->_vtable = vtable_vt;
  vtable_vt->parent = object_vt;

  symbol_bootstrap();

  nil_vt = vtable_delegated(NULL, 0);

  vtable_add_method_descriptors(object_vt, Object_methods);
  vtable_add_method_descriptors(nil_vt, Nil_methods);

  // Global scope
  scope_bootstrap();

  Scope *global_scope = scope_new();
  scope_add(global_scope, symbol_intern("VTable"), (Object *)vtable_vt);
  scope_add(global_scope, symbol_intern("Object"), (Object *)object_vt);
  scope_add(global_scope, symbol_intern("scope"), (Object *)global_scope);
  scope_add(global_scope, symbol_intern("nil"), (Object *)NULL);

  // Now bootstrap the rest of the builtin objects
  string_bootstrap(global_scope);
  native_integer_bootstrap(global_scope);
  boolean_bootstrap(global_scope);

  return (Object *)global_scope;
}
