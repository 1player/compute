#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"

static VTable *string_vt;

Object *string_new(char *buf) {
  String *str = (String *)vtable_allocate(string_vt);
  str->buf = buf;
  str->len = strlen(buf);
  return (Object *)str;
}

bool string_equals(String *self, String *other) {
  return strequals(self->buf, self->len, other->buf, other->len);
}

static Object *string_equals_(String *self, String *other) {
  if (string_equals(self, other)) {
    return singleton_true;
  }

  return singleton_false;
}

Object *string_concat(String *self, String *other) {
  String *str = (String *)vtable_allocate(string_vt);
  str->len = self->len + other->len;
  str->buf = malloc(str->len);

  memmove(str->buf, self->buf, self->len);
  memmove(&str->buf[self->len], other->buf, other->len);

  return (Object *)str;
}

Object *string_inspect(String *self) {
  char *buf = malloc(self->len + 2 + 1);
  buf[0] = '"';
  memcpy(&buf[1], self->buf, self->len);
  buf[self->len + 1] = '"';
  buf[self->len + 2] = 0;

  return string_new(buf);
}

Object *string_println(String *self) {
  fwrite(self->buf, self->len, 1, stdout);
  fputc('\n', stdout);
  return NULL;
}

method_descriptor_t String_methods[] = {
  { .name = "==",      .fn = string_equals_ },
  { .name = "concat",  .fn = string_concat },
  { .name = "println", .fn = string_println },
  { .name = "inspect", .fn = string_inspect },
  { NULL },
};

VTable *string_bootstrap() {
  string_vt = vtable_delegated(vtable_vt, sizeof(String));
  vtable_add_method_descriptors(string_vt, String_methods);

  return string_vt;
}
