#ifndef UNIVERSE_H
#define UNIVERSE_H

#include <stdint.h>
#include "lang.h"

#if defined __x86_64__
#define WORDSIZE 8
#endif

typedef intptr_t NativeInteger;
typedef void * WORD;

#if WORDSIZE == 8
typedef int32_t HALFWORD;
#endif

#define TO_NATIVE(n) ((object *)((((intptr_t)(n)) << 1) | 1))
#define FROM_NATIVE(n) (((intptr_t)(n)) >> 1)
#define IS_NATIVE(n) (((intptr_t)(n)) & 1)

typedef struct object {
  struct object *parent;
  HALFWORD flags;
  HALFWORD capacity;
  struct object **selectors; // NULL-padded to capacity
  struct object **slots;     // NULL-padded to capacity
  char data[];
} object;


typedef struct Slot {
  object _o;
  int arguments;
  void *data;
} Slot;

extern object *the_Object;

object *root_scope_bootstrap();

object *object_derive(object *self, size_t data_size);
object *object_set(object *self, object *name, object *slot);
object *object_lookup(object *self, object *name);
object *object_set_variable(object *self, object *name, object *value);
object *object_set_method(object *self, object *name, unsigned int arguments, void *fn);

bool slot_is_variable(Slot *slot);
object *slot_for_variable(object *value);
object *slot_for_method(int arguments, void *fn);

object *intern(char *string);
object *send(object *receiver, object *selector, int n_args, object **args);
object *send0(object *receiver, object *selector);
object *send1(object *receiver, object *selector, object *arg1);
object *send2(object *receiver, object *selector, object *arg1, object *arg2);

object *eval(expr_t *expr, object *scope);

#endif
