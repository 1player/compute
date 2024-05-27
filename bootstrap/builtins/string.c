#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "builtins.h"

typedef struct String {
  object _o;
  size_t len; // Doesn't include the '\0' at the end
  char buf[];
} String;

object *the_String;

static String *string_new_with_size(size_t len) {
  String *str = (String *)object_derive(the_String, sizeof(String) + len + 1);
  str->len = len;
  return str;
}

object *string_new(char *buf) {
  size_t len = strlen(buf);
  String *str = string_new_with_size(len);
  memmove(&str->buf, buf, len);
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

void string_bootstrap(object *scope) {
  the_String = object_derive(the_Object, sizeof(String));

  object_set_method(the_String, intern("=="), 1, string_equals);
  object_set_method(the_String, intern("concat"), 1, string_concat);
  object_set_method(the_String, intern("println"), 0, string_println);
  object_set_method(the_String, intern("inspect"), 0, string_inspect);

  object_set_variable(scope, intern("String"), the_String);
}
