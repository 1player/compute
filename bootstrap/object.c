#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "lib.h"
#include "object.h"
#include "builtins.h"

VTable *vtable_vt;
VTable *object_vt;
VTable *string_vt;
VTable *native_integer_vt;
VTable *tuple_vt;
VTable *world_vt;
World *world;

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

void world_init(World *world) {
  array_init(&world->entries);
}

Object *world_lookup(char *name) {
  size_t name_len = strlen(name);

  for (int i = 0; i < world->entries.size; i++) {
    Tuple *t = world->entries.elements[i];
    String *entry_name = (String *)t->left;
    if (entry_name->len == name_len && strncmp(entry_name->buf, name, name_len) == 0) {
      return t->right;
    }
  }

  return NULL;
}

void world_add(World *world, String *name, Object *obj) {
  // Check if an entry with this name already exists
  for (int i = 0; i < world->entries.size; i++) {
    Tuple *t = world->entries.elements[i];
    if (string_equals(name, (String *)t->left)) {
      t->right = obj;
      return;
    }
  }

  Object *t = tuple_new((Object *)name, (Object *)obj);
  array_append(&world->entries, t);
}

static void add_method_descriptors(VTable *vt, method_descriptor_t *desc) {
  while (desc && desc->name) {
    vtable_add_method(vt, (String *)string_new(desc->name), desc->fn);
    desc++;
  }
}

void world_bootstrap() {
  // VTable
  vtable_vt = vtable_delegated(NULL, sizeof(VTable));
  vtable_vt->_vtable = vtable_vt;

  // Object
  object_vt = vtable_delegated(NULL, sizeof(Object));
  object_vt->_vtable = vtable_vt;
  vtable_vt->parent = object_vt;

  // String
  string_vt = vtable_delegated(vtable_vt, sizeof(String));
  add_method_descriptors(string_vt, String_methods);

  // Additional methods on Object
  add_method_descriptors(object_vt, Object_methods);

  // NativeInteger
  native_integer_vt = vtable_delegated(vtable_vt, 0);
  add_method_descriptors(native_integer_vt, NativeInteger_methods);

  // Tuple
  tuple_vt = vtable_delegated(vtable_vt, sizeof(Tuple));

  /* // World
   * world_vt = vtable_delegated(vtable_vt, sizeof(World));
   * world = (World *)vtable_allocate(world_vt);
   *
   * world_init(world);
   * world_add(world, string_new("VTable"), (Object *)vtable_vt);
   * world_add(world, string_new("Object"), (Object *)object_vt);
   * world_add(world, string_new("String"), (Object *)string_vt); */
}
