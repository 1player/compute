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

HANDLER(String__equals, String *other) {
  String *self_ = (String *)self;

  if (strequals(self_->buf, self_->len, other->buf, other->len)) {
    return singleton_true;
  }

  return singleton_false;
}

HANDLER(String__concat, String *other) {
  String *self_ = (String *)self;
  String *str = string_new_with_size(self_->len + other->len);

  memmove(str->buf, self_->buf, self_->len);
  memmove(&str->buf[self_->len], other->buf, other->len);

  return (object *)str;
}

HANDLER(String__inspect) {
  String *self_ = (String *)self;

  char *buf = malloc(self_->len + 2 + 1);
  buf[0] = '"';
  memcpy(&buf[1], self_->buf, self_->len);
  buf[self_->len + 1] = '"';
  buf[self_->len + 2] = 0;

  return string_new(buf);
}

HANDLER(String__println) {
  String *self_ = (String *)self;

  fwrite(self_->buf, self_->len, 1, stdout);
  fputc('\n', stdout);
  return NULL;
}

static slot_definition String_slots[] = {
  { .type = METHOD_SLOT, .selector = "==",      .value = String__equals },
  { .type = METHOD_SLOT, .selector = "concat",  .value = String__concat },
  { .type = METHOD_SLOT, .selector = "println", .value = String__println },
  { .type = METHOD_SLOT, .selector = "inspect", .value = String__inspect },
  { 0 },
};

void string_bootstrap(object *scope) {
  String_trait = trait_derive(Object_trait, sizeof(String), String_slots);

  scope_set(scope, intern("String"), (object *)String_trait);
}
