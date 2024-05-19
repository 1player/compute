#ifndef ARRAY_H
#define ARRAY_H

// Adapted from https://github.com/rxi/vec/blob/master/src/vec.h
//
// Copyright (c) 2014 rxi
// Licensed under the terms of the MIT license.

#include <stdlib.h>
#include <string.h>

#define array_unpack_(v)\
  (char**)&(v)->data, &(v)->length, &(v)->capacity, sizeof(*(v)->data)


#define array_t(T)\
  struct { T *data; int length, capacity; }


#define array_init(v)\
  memset((v), 0, sizeof(*(v)))


#define array_deinit(v)\
  ( free((v)->data),\
    array_init(v) )


#define array_push(v, val)\
  ( array_expand_(array_unpack_(v)) ? -1 :\
    ((v)->data[(v)->length++] = (val), 0) )


#define array_pop(v)\
  (v)->data[--(v)->length]


#define array_splice(v, start, count)\
  ( array_splice_(array_unpack_(v), start, count),\
    (v)->length -= (count) )


#define array_swapsplice(v, start, count)\
  ( array_swapsplice_(array_unpack_(v), start, count),\
    (v)->length -= (count) )


#define array_insert(v, idx, val)\
  ( array_insert_(array_unpack_(v), idx) ? -1 :\
    ((v)->data[idx] = (val)), (v)->length++, 0 )


#define array_sort(v, fn)\
  qsort((v)->data, (v)->length, sizeof(*(v)->data), fn)


#define array_swap(v, idx1, idx2)\
  array_swap_(array_unpack_(v), idx1, idx2)


#define array_truncate(v, len)\
  ((v)->length = (len) < (v)->length ? (len) : (v)->length)


#define array_clear(v)\
  ((v)->length = 0)


#define array_first(v)\
  (v)->data[0]


#define array_last(v)\
  (v)->data[(v)->length - 1]


#define array_reserve(v, n)\
  array_reserve_(array_unpack_(v), n)


#define array_compact(v)\
  array_compact_(array_unpack_(v))


#define array_pusharr(v, arr, count)\
  do {\
    int i__, n__ = (count);\
    if (array_reserve_po2_(array_unpack_(v), (v)->length + n__) != 0) break;\
    for (i__ = 0; i__ < n__; i__++) {\
      (v)->data[(v)->length++] = (arr)[i__];\
    }\
  } while (0)


#define array_extend(v, v2)\
  array_pusharr((v), (v2)->data, (v2)->length)


#define array_find(v, val, idx)\
  do {\
    for ((idx) = 0; (idx) < (v)->length; (idx)++) {\
      if ((v)->data[(idx)] == (val)) break;\
    }\
    if ((idx) == (v)->length) (idx) = -1;\
  } while (0)


#define array_remove(v, val)\
  do {\
    int idx__;\
    array_find(v, val, idx__);\
    if (idx__ != -1) array_splice(v, idx__, 1);\
  } while (0)


#define array_reverse(v)\
  do {\
    int i__ = (v)->length / 2;\
    while (i__--) {\
      array_swap((v), i__, (v)->length - (i__ + 1));\
    }\
  } while (0)


#define array_foreach(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = 0;\
        (iter) < (v)->length && (((var) = (v)->data[(iter)]), 1);\
        ++(iter))


#define array_foreach_rev(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = (v)->length - 1;\
        (iter) >= 0 && (((var) = (v)->data[(iter)]), 1);\
        --(iter))


#define array_foreach_ptr(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = 0;\
        (iter) < (v)->length && (((var) = &(v)->data[(iter)]), 1);\
        ++(iter))


#define array_foreach_ptr_rev(v, var, iter)\
  if  ( (v)->length > 0 )\
  for ( (iter) = (v)->length - 1;\
        (iter) >= 0 && (((var) = &(v)->data[(iter)]), 1);\
        --(iter))



int array_expand_(char **data, int *length, int *capacity, int memsz);
int array_reserve_(char **data, int *length, int *capacity, int memsz, int n);
int array_reserve_po2_(char **data, int *length, int *capacity, int memsz,
                     int n);
int array_compact_(char **data, int *length, int *capacity, int memsz);
int array_insert_(char **data, int *length, int *capacity, int memsz,
                int idx);
void array_splice_(char **data, int *length, int *capacity, int memsz,
                 int start, int count);
void array_swapsplice_(char **data, int *length, int *capacity, int memsz,
                     int start, int count);
void array_swap_(char **data, int *length, int *capacity, int memsz,
               int idx1, int idx2);

#endif
