#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lang.h"

void array_init(array_t *ary) {
  ary->elements = NULL;
  ary->size = 0;
  ary->capacity = 0;
}

void array_append(array_t *ary, void *el) {
  if (ary->size == ary->capacity) {
    array_resize(ary, ary->size == 0 ? 2 : (ary->size * 2));
  }

  int i = ary->size;
  ary->elements[i] = el;
  ary->size++;
}

void array_resize(array_t *ary, int new_capacity) {
  assert(new_capacity >= ary->capacity);

  ary->elements = realloc(ary->elements, sizeof(void *) * new_capacity);
  ary->capacity = new_capacity;
}

void *array_pop_start(array_t *ary) {
  if (ary->size < 1) {
    panic("array_pop_start called with fewer than one element");
  }

  void *popped = ary->elements[0];
  ary->size--;
  memmove(&ary->elements[0], &ary->elements[1], sizeof(void *) * ary->size);

  return popped;
}
