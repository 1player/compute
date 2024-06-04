#include <stdio.h>
#include <stdlib.h>

#include "builtins.h"

// Trait

slot_definition Trait_slots[] = {
  { 0 },
};

// Object

HANDLER(Object__inspect) {
  if (self == NULL) {
    return string_new("nil");
  }

  char *buf;
  asprintf(&buf, "<Object %p>", self);
  object *s = string_new(buf);
  free(buf);
  return s;
}

HANDLER(Object__is, object *other) {
  if (self == other) {
    return singleton_true;
  }
  return singleton_false;
}

slot_definition Object_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = Object__inspect },
  { .type = METHOD_SLOT, .selector = "==",      .value = Object__is },
  { .type = METHOD_SLOT, .selector = "===",     .value = Object__is },
  { 0 },
};

// Closure

HANDLER(Closure__inspect) {
  char *buf;
  asprintf(&buf, "<Closure>");
  object *s = string_new(buf);
  free(buf);

  return s;
}

slot_definition Closure_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = Closure__inspect },
  { 0 },
};

// Symbol

extern string_table_t *global_symbol_table;

HANDLER(Symbol__inspect) {
  Symbol *self_ = (Symbol *)self;

  char *buf;
  asprintf(&buf, "#%s", string_table_get_key(global_symbol_table, self_->handle));
  object *s = string_new(buf);
  free(buf);
  return s;
}

slot_definition Symbol_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = Symbol__inspect },
  { 0 },
};

// nil

slot_definition nil_slots[] = {
  { .type = METHOD_SLOT, .selector = "inspect", .value = Object__inspect },
  { 0 },
};
