// A hash-table optimized for storing null-terminated strings as keys

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"

static inline size_t next_power_of_2(size_t n) {
  return 1 << (32 - __builtin_clz (n - 1));
}

static inline char *string_pool_at(string_pool_t *sp, size_t index) {
  return &sp->buf[index];
}

static size_t string_pool_add(string_pool_t *sp, const char *str) {
  size_t str_len = strlen(str);
  size_t new_size = sp->size + str_len + 1;

  if (new_size > sp->cap) {
    size_t new_cap = next_power_of_2(new_size);
    sp->buf = realloc(sp->buf, new_cap);
    memset(&sp->buf[sp->cap], 0, new_cap - sp->cap);
    sp->cap = new_cap;
  }

  strncpy(&sp->buf[sp->size], str, str_len);
  size_t old_size = sp->size;
  sp->size = new_size;

  return old_size;
}

static void string_pool_init(string_pool_t *sp) {
  sp->buf = NULL;
  sp->size = 0;
  sp->cap = 0;
}

// FNV-1A
static size_t hash(const char* s)
{
    size_t hash = 2166136261;
    for (const char* ptr = s; *ptr != '\0'; ptr++)
    {
        hash ^= *ptr;
        hash *= 16777619;
    }
    return hash;
}

string_table_t *string_table_new() {
  size_t initial_size = 32;

  string_table_t *t = malloc(sizeof(string_table_t));
  string_pool_init(&t->pool);

  t->pointers = array_new_with_capacity(initial_size);
  array_fill(t->pointers, (void *)-1);

  t->values = array_new_with_capacity(initial_size);

  t->count = 0;

  return t;
}

static void string_table_expand(string_table_t *t) {
  size_t size = t->pointers->size;
  size_t new_size = size * 2;

  // Allocate new arrays for pointers and values
  array_t *new_pointers_array = array_new_with_capacity(new_size);
  array_fill(new_pointers_array, (void *)-1);
  array_t *new_values_array = array_new_with_capacity(new_size);

  ssize_t *pointers = (ssize_t *)t->pointers->elements;
  ssize_t *new_pointers = (ssize_t *)new_pointers_array->elements;

  void **values = (void *)t->values->elements;
  void **new_values = new_values_array->elements;

  // Rehash table
  for (size_t i = 0; i < size; i++) {
    if (pointers[i] == -1) {
      continue;
    }

    char *key = string_pool_at(&t->pool, pointers[i]);
    size_t index = hash(key) % new_size;
    while (new_pointers[index] != -1) {
      index = (index + 1) % new_size;
    }
    new_pointers[index] = pointers[i];
    new_values[index] = values[i];
  }

  array_free(t->pointers);
  array_free(t->values);

  t->pointers = new_pointers_array;
  t->values = new_values_array;
}

bool string_table_lookup(string_table_t *t, const char *key, location_t *loc) {
  ssize_t *pointers = (ssize_t *)t->pointers->elements;
  size_t size = t->pointers->size;
  location_t index = hash(key) % size;

  // Open addressing, linear probing
  while (pointers[index] != -1) {
    if (strcmp(key, string_pool_at(&t->pool, pointers[index])) == 0) {
      *loc = index;
      return true;
    }
    index = (index + 1) % size;
  }

  *loc = index;
  return false;
}

void *string_table_get(string_table_t *t, location_t loc) {
  void **values = t->values->elements;
  return values[loc];
}

char *string_table_get_key(string_table_t *t, size_t handle) {
  return string_pool_at(&t->pool, handle);
}

size_t string_table_set(string_table_t *t, location_t loc, const char *key, void *value) {
  ssize_t *pointers = (ssize_t *)t->pointers->elements;
  void **values = t->values->elements;

  size_t handle = string_pool_add(&t->pool, key);
  pointers[loc] = handle;
  values[loc] = value;

  t->count++;

  // Adjust at 66% occupation
  size_t size = t->pointers->size;
  if (t->count > (size * 2) / 3) {
    string_table_expand(t);
  }

  return handle;
}
