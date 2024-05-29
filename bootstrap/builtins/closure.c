#include <stdlib.h>
#include <stdio.h>

#include "builtins.h"
#include "lang.h"

typedef struct Closure {
  object _o;
  expr_t *body;
  object *scope;
  array_t *arg_names; // An array of Symbols
} Closure;

object *the_Closure;

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
  Closure *self = (Closure *)object_derive(the_Closure, sizeof(Closure));
  self->arg_names = arg_names;
  self->body = body;
  self->scope = scope;

  object_set_method((object *)self, intern("call"), arg_names->size, closure_call);

  return (object *)self;
}

void closure_bootstrap(object *scope) {
  the_Closure = object_derive(the_Object, sizeof(Closure));
  object_set_method(the_Closure, intern("inspect"), 0, closure_inspect);

  object_set_variable(scope, intern("Closure"), the_Closure);
}
