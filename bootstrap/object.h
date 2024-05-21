#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

typedef struct Meta Meta;
typedef struct Object Object;

typedef intptr_t NativeInteger;

#define WORD_SIZE (sizeof(NativeInteger))

#define TO_NATIVE(n) (((n) << 1) | 1)
#define FROM_NATIVE(n) ((n) >> 1)

typedef struct {
  Meta *_meta;
  void *key;
  void *method;
  void *data;
} MetaEntry;

typedef struct Meta {
  Meta *_meta;

  // Method closures
  NativeInteger n_entries;
  MetaEntry *entries[];
} Meta;

#define OBJECT_SIZE(o) (sizeof(o) - WORD_SIZE)

typedef struct {
  Meta *_meta;
  NativeInteger length;
  char data[];
} String;

typedef struct Object {
  Meta *_meta;
  char data[];
} Object;

String *string_new(int length);
String *string_new_from_char(char *s);
String *string_concat(String *self, String *other);
void string_println(String *self);
void object_bootstrap();

#endif
