#include <assert.h>
#include "object.h"


static Slot *slot_new() {
  return (Slot *)object_derive(the_Object, sizeof(Slot));
}

inline bool slot_is_variable(Slot *slot) {
  return slot->arguments < 0;
}

object *slot_for_variable(object *value) {
  Slot *slot = slot_new();
  slot->arguments = -1;
  slot->data = value;

  return (object *)slot;
}

object *slot_for_method(int arguments, void *fn) {
  assert(arguments >= 0);

  Slot *slot = slot_new();
  slot->arguments = arguments;
  slot->data = fn;

  return (object *)slot;
}

