#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "common.h"

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
