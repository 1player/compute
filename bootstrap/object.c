#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "object.h"

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

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

void vtable_add_method(VTable *self, String *name, void *ptr) {
  // Replace an existing entry, if any
  for (size_t i = 0; i < self->len; i++) {
    if (strncmp(name->buf, self->names[i]->buf, name->len) == 0) {
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

  self->names[self->len] = name;
  self->ptrs[self->len] = ptr;
  self->len++;
}

void *vtable_lookup(VTable *self, char *selector) {
  for (size_t i = 0; i < self->len; i++) {
    if (strncmp(selector, self->names[i]->buf, self->names[i]->len) == 0) {
      return self->ptrs[i];
    }
  }

  return NULL;
}

String *string_concat(String *self, String *other) {
  String *str = (String *)vtable_allocate(self->_vtable);
  str->len = self->len + other->len;
  str->buf = malloc(str->len);

  memmove(str->buf, self->buf, self->len);
  memmove(&str->buf[self->len], other->buf, other->len);

  return str;
}

void string_println(String *self) {
  fwrite(self->buf, self->len, 1, stdout);
  fputc('\n', stdout);
}

//

#define VA_NARGS(...) ((int)(sizeof((Object *[]){ __VA_ARGS__ })/sizeof(Object *)))
#define send(RCV, SEL, ...) _send((Object *)(RCV), (SEL), VA_NARGS(__VA_ARGS__), ##__VA_ARGS__)

Object *_send(Object *receiver, char *selector, int n_args, ...) {
  void *ptr = vtable_lookup(receiver->_vtable, selector);
  if (!ptr) {
    fprintf(stderr, "Object %p doesn't know how to respond to '%s'\n", receiver, selector);
    return NULL;
  }

  Object *ret = NULL;
  va_list args;
  va_start(args, n_args);

#define ARG (va_arg(args, Object *))

  switch (n_args) {
  case 0:
    Object *(*fn0)(Object *) = ptr;
    ret = fn0(receiver);
    break;

  case 1:
    Object *(*fn1)(Object *, Object *) = ptr;
    ret = fn1(receiver, ARG);
    break;

  default:
    fprintf(stderr, "Sending messages with %d arguments not implemented\n", n_args);
  }

  va_end(args);

  return ret;
}

//

void world_bootstrap(World *world) {
  VTable *vtable_vt = vtable_delegated(NULL, sizeof(VTable));
  vtable_vt->_vtable = vtable_vt;

  VTable *object_vt = vtable_delegated(NULL, sizeof(Object));
  object_vt->_vtable = vtable_vt;
  vtable_vt->parent = object_vt;

  VTable *string_vt = vtable_delegated(vtable_vt, sizeof(String));

  String *_make_string(char *buf) {
    String *str = (String *)vtable_allocate(string_vt);
    str->buf = buf;
    str->len = strlen(buf);
    return str;
  }

  vtable_add_method(string_vt, _make_string("println"), string_println);
  vtable_add_method(string_vt, _make_string("concat"), string_concat);

  String *h = _make_string("Hello, ");
  String *w = _make_string("world!");
  String *hw = (String *)send(h, "concat", (Object *)w);
  send(hw, "println");
}
