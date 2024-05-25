#ifndef LIB_H
#define LIB_H

#include <stdarg.h>
#include <stdbool.h>

#define max(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

//

typedef struct {
  void **elements;
  int size;
  int capacity;
} array_t;

array_t *array_new();
array_t *array_new_with_capacity(int capacity);
void array_append(array_t *ary, void *el);
void array_resize(array_t *ary, int new_capacity);
void *array_pop_start(array_t *ary);
void array_fill(array_t *ary, void *el);
void array_free(array_t *ary);

//

typedef struct string_pool {
  char *buf;
  size_t size;
  size_t cap;
} string_pool_t;

typedef struct string_table {
  string_pool_t pool;
  array_t *pointers;
  array_t *values;
  size_t count;
} string_table_t;

typedef size_t location_t;

string_table_t *string_table_new();
bool string_table_lookup(string_table_t *t, const char *key, location_t *loc);
void *string_table_get(string_table_t *t, location_t loc);
void string_table_set(string_table_t *t, location_t loc, const char *str, void *value);

//

void panic(const char *msg, ...);
void info(const char *msg, ...);
void error(const char *file, const int line, const char *msg, va_list args);

bool strequals(char *a, size_t a_len, char *b, size_t b_len);


#endif
