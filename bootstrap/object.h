#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include "lang.h"
#include "lib.h"

#if defined __x86_64__
#define WORDSIZE 8
#endif

typedef intptr_t NativeInteger;
typedef intptr_t WORD;
typedef uintptr_t UWORD;

#if WORDSIZE == 8
typedef int32_t HALFWORD;
#endif

#define TO_NATIVE(n) ((object *)((((intptr_t)(n)) << 1) | 1))
#define FROM_NATIVE(n) (((intptr_t)(n)) >> 1)
#define IS_NATIVE(n) (((intptr_t)(n)) & 1)

typedef struct trait trait;

typedef struct object {
  trait *_trait[0];
  WORD _data[];
} object;

#define TRAIT(o)      (((object *)(o))->_trait[-1])

typedef struct trait {
  trait *parent;
  UWORD n_slots;
  UWORD data_size;
  object **selectors;
  object **slots;
} trait;

enum slot_type {
  UNSET_SLOT = 0,
  METHOD_SLOT,
  DATA_SLOT,
};

typedef struct slot_definition {
  enum slot_type type;
  char *selector;
  void *value;
} slot_definition;

typedef struct {
  expr_t *body;
  object *scope;
  array_t *arg_names; // An array of Symbols
} interpreted_closure_t;

typedef struct Symbol {
  size_t handle;
} Symbol;

typedef struct __lookup __lookup;

#define HANDLER(name, ...)                                              \
  object *name(void *closure_data, void *self, object *receiver, ##__VA_ARGS__)

__lookup *trait_lookup(trait *self, object *name);
trait *trait_derive(trait *parent, size_t trait_size, slot_definition *defs);

object *object_new(trait *_trait);

char *inspect(object *o);
object *intern(char *string);
object *send_args(object *receiver, object *selector, int n_args, object **args);
object *send_(object *receiver, object *selector, int n_args, ...);

object *eval(expr_t *expr, object *scope);
object *eval_interpreted_closure(interpreted_closure_t *cdata, void *tdata, object *r, ...);

object *closure_new(void *entrypoint, void *data);
object *closure_new_interpreted(array_t *arg_names, expr_t *body, object *scope);

#define VA_NARGS(...) ((int)(sizeof((object *[]){ __VA_ARGS__ })/sizeof(object *)))
#define send(RCV, SEL, ...) send_((RCV), (SEL), VA_NARGS(__VA_ARGS__), ##__VA_ARGS__)

object *root_scope_bootstrap();

extern trait *Object_trait;

#endif
