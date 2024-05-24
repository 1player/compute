#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include "lang.h"

typedef struct VTable VTable;
typedef struct Object Object;

typedef intptr_t NativeInteger;

#define TO_NATIVE(n) ((Object *)((((intptr_t)(n)) << 1) | 1))
#define FROM_NATIVE(n) (((intptr_t)(n)) >> 1)
#define IS_NATIVE(n) (((intptr_t)(n)) & 1)

#define WORD_SIZE (sizeof(NativeInteger))

typedef struct String {
  VTable *_vtable;
  size_t len;
  char *buf;
} String;

typedef struct VTable {
  VTable *_vtable;
  VTable *parent;
  size_t object_size;

  size_t len;
  size_t cap;
  String **names;
  void **ptrs;
} VTable;

typedef struct Object {
  VTable *_vtable;
} Object;

typedef struct Tuple {
  VTable *_vtable;
  Object *left;
  Object *right;
} Tuple;

typedef struct {
  char *name;
  void *fn;
} method_descriptor_t;

extern VTable *vtable_vt;
extern VTable *object_vt;
extern VTable *string_vt;
extern VTable *native_integer_vt;
extern VTable *tuple_vt;

VTable *vtable_delegated(VTable *self, size_t object_size);
Object *vtable_allocate(VTable *self);
void vtable_add_method(VTable *self, String *name, void *ptr);
void vtable_add_method_descriptors(VTable *self, method_descriptor_t *desc);
void *vtable_lookup(VTable *self, char *selector);

Object *bootstrap();

Object *_send(Object *receiver, char *selector, int n_args, ...);
Object *send_args(Object *receiver, char *selector, array_t *args);

Object *eval(expr_t *expr);

#define VA_NARGS(...) ((int)(sizeof((Object *[]){ __VA_ARGS__ })/sizeof(Object *)))
#define send(RCV, SEL, ...) _send((Object *)(RCV), (SEL), VA_NARGS(__VA_ARGS__), ##__VA_ARGS__)


#endif
