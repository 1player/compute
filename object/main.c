#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

Meta *meta_meta = NULL;

// Create a new object with this metatable
Object *meta_alloc(Meta *_meta, int object_size) {
  // Allocate one word for the meta pointer, plus the actual object size
  Object *o = calloc(1, WORD_SIZE + (object_size * WORD_SIZE));
  o->_meta = meta_meta;
  return o;
}

// Create a new metatable
Meta *meta_new(int n_entries, MetaEntry *entries[]) {
  Meta *m = (Meta *)meta_alloc(meta_meta, OBJECT_SIZE(Meta) + (n_entries * sizeof(MetaEntry)));
  m->n_entries = TO_NATIVE(n_entries);

  for (int i = 0; i < n_entries; i++) {
    m->entries[i] = entries[i];
  }

  return m;
}

////

Meta *string_meta;


String *string_new(int length) {
  String *string = (String *)meta_alloc(string_meta, OBJECT_SIZE(String) + length);
  string->length = TO_NATIVE(length);
  memset(string->data, 0, length);

  return string;
}

String *string_new_from_char(char *s) {
  int length = strlen(s);

  String *string = string_new(length);
  memcpy(string->data, s, length);

  return string;
}

String *string_concat(String *self, String *other) {
  int our_length = FROM_NATIVE(self->length);
  int other_length = FROM_NATIVE(other->length);

  String *new_string = string_new(our_length + other_length);

  memcpy(&new_string->data, self->data, our_length);
  memcpy(&new_string->data[our_length], other->data, other_length);

  return new_string;
}

void string_println(String *self) {
  int length = FROM_NATIVE(self->length);
  char s[length + 1];

  memcpy(s, self->data, length);
  s[length] = 0;
  printf("%s\n", s);
}

MetaEntry string_methods[] = {
  { .key = "concat", .method = string_concat },
  { .key = "println", .method = string_println },
};

void bootstrap() {
  meta_meta = meta_new(0, NULL);
  meta_meta->_meta = meta_meta;

  string_meta = meta_new(2, (MetaEntry **)&string_methods);
}


int main(int argc, char *argv[]) {
  bootstrap();

  String *hello = string_new_from_char("hello, ");
  String *world = string_new_from_char("world");

  String *hello_world = string_concat(hello, world);
  string_println(hello_world);

  return 0;
}
