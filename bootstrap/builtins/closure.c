#include <stdlib.h>
#include <stdio.h>

#include "builtins.h"
#include "lang.h"

typedef struct Closure {
  expr_t *body;
  object *scope;
  array_t *arg_names; // An array of Symbols
} Closure;

trait *Closure_trait;

object *closure_inspect(Closure *self) {
  char *buf;
  asprintf(&buf, "<Closure with %d arguments>", self->arg_names->size);
  object *s = string_new(buf);
  free(buf);

  return s;
}

object *closure_call(Closure *self, ...) {
  va_list args;
  va_start(args, self);

  object *ret = eval_closure_call(self->body, self->scope, self->arg_names, args);

  va_end(args);
  return ret;
}

object *closure_new(array_t *arg_names, expr_t *body, object *scope) {
  Closure *self = (Closure *)object_new(Closure_trait);
  self->arg_names = arg_names;
  self->body = body;
  self->scope = scope;

  return (object *)self;
}

static slot_definition Closure_slots[] = {
  { .type = METHOD_SLOT, .selector = "call", .value = closure_call },
  { .type = METHOD_SLOT, .selector = "inspect", .value = closure_inspect },
  { 0 },
};


void closure_bootstrap(object *scope) {
  (void)scope;

  Closure_trait = trait_derive(Object_trait, sizeof(Closure), Closure_slots);
}
