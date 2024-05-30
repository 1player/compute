#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"

trait *String_trait;

static String *string_new_with_size(size_t len) {
  String *str = (String *)object_new(String_trait);
  str->len = len;
  str->buf = calloc(1, len + 1);
  return str;
}

object *string_new(char *buf) {
  size_t len = strlen(buf);
  String *str = string_new_with_size(len);
  memmove(str->buf, buf, len);
  str->buf[len] = 0;

  return (object *)str;
}

static object *string_equals(String *self, String *other) {
  if (strequals(self->buf, self->len, other->buf, other->len)) {
    return singleton_true;
  }

  return singleton_false;
}

object *string_concat(String *self, String *other) {
  String *str = string_new_with_size(self->len + other->len);

  memmove(str->buf, self->buf, self->len);
  memmove(&str->buf[self->len], other->buf, other->len);

  return (object *)str;
}

object *string_inspect(String *self) {
  char *buf = malloc(self->len + 2 + 1);
  buf[0] = '"';
  memcpy(&buf[1], self->buf, self->len);
  buf[self->len + 1] = '"';
  buf[self->len + 2] = 0;

  return string_new(buf);
}

object *string_println(String *self) {
  fwrite(self->buf, self->len, 1, stdout);
  fputc('\n', stdout);
  return NULL;
}

static slot_definition String_slots[] = {
  { .type = METHOD_SLOT, .selector = "==",      .value = string_equals },
  { .type = METHOD_SLOT, .selector = "concat",  .value = string_concat },
  { .type = METHOD_SLOT, .selector = "println", .value = string_println },
  { .type = METHOD_SLOT, .selector = "inspect", .value = string_inspect },
  { 0 },
};

void string_bootstrap(object *scope) {
  String_trait = trait_derive(Object_trait, sizeof(String), String_slots);

  scope_set(scope, intern("String"), (object *)String_trait);
}
