#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lang.h"

array_t *array_new() {
  array_t *ary = malloc(sizeof(array_t));
  ary->elements = NULL;
  ary->size = 0;
  ary->capacity = 0;

  return ary;
}

array_t *array_new_with_capacity(int cap) {
  array_t *ary = array_new();
  array_resize(ary, cap);

  return ary;
}

void array_free(array_t *ary) {
  free(ary->elements);
  free(ary);
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

void array_fill(array_t *ary, void *el) {
  for (int i = 0; i < ary->capacity; i++) {
    ary->elements[i] = el;
  }
  ary->size = ary->capacity;
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
