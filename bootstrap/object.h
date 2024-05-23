#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include "lang.h"

typedef struct VTable VTable;
typedef struct Object Object;

typedef intptr_t NativeInteger;

#define TO_NATIVE(n) (((n) << 1) | 1)
#define FROM_NATIVE(n) ((n) >> 1)
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

typedef struct World {
  VTable *_vtable;
  array_t entries; // Array of tuples (String, Object)
} World;

void world_bootstrap();
Object *world_lookup(char *name);
Object *world_make_tuple(Object *left, Object *right);
Object *world_make_string(char *str);
Object *world_make_native_integer(intptr_t number);

Object *_send(Object *receiver, char *selector, int n_args, ...);

Object *eval(expr_t *expr);

#define VA_NARGS(...) ((int)(sizeof((Object *[]){ __VA_ARGS__ })/sizeof(Object *)))
#define send(RCV, SEL, ...) _send((Object *)(RCV), (SEL), VA_NARGS(__VA_ARGS__), ##__VA_ARGS__)

#endif
