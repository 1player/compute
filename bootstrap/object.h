#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

typedef struct VTable VTable;
typedef struct Object Object;

typedef intptr_t NativeInteger;

#define TO_NATIVE(n) (((n) << 1) | 1)
#define FROM_NATIVE(n) ((n) >> 1)

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

typedef struct Array {
  VTable *_vtable;
  size_t length;
  size_t cap;
  Object **items;
} Array;

typedef struct World {
  VTable *_vtable;
  Array *entries;
} World;

void world_bootstrap(World *world);

#endif
