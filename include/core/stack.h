#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#ifndef VECTOR_DEFAULT_SIZE
#define VECTOR_DEFAULT_SIZE 10
#endif

typedef struct Vector {
  void **_arr;
  uint8_t ops;
  uint32_t offset;
  uint32_t size;
  uint32_t maxsize;
  void* _stackarr[VECTOR_DEFAULT_SIZE];
} Vector;


#define Vector_INIT(vec, len)                                           \
  ({memset(vec, 0, offsetof(Vector, _stackarr));                        \
    (vec)->size = VECTOR_DEFAULT_SIZE;                                  \
    (vec)->maxsize = UINT32_MAX;                                        \
                                                                        \
    if (len > (vec)->size) {                                            \
      (vec)->size = len;                                                \
      (vec)->_arr = malloc(sizeof(void*) * len);                        \
      assert((vec)->_arr);                                              \
    }})

#define Vector_FREE(vec)                                               \
  if ((vec)->_arr) { free((vec)->_arr); (vec)->_arr = NULL;}           \


#define Vector_PUSH(vec, item)                                         \
  ({void** _arr = Vector_PREPARE(vec, 1);                              \
    _arr[(vec)->offset++] = (void*)(item);                             \
    1;})

#define Vector_POP(vec, type)                                          \
  ((type*)((vec)->offset ? Vector_ARR(vec)[--(vec)->offset] : NULL))

#define Vector_LEN(vec) ((vec)->offset)

#define Vector_ARR(vec)                                                \
  ((vec)->size > VECTOR_DEFAULT_SIZE ? (vec)->_arr : (vec)->_stackarr)


/**
 * Change the allocation of the vector to the new size. The first time
 * Vector_RESIZE is called, we move over from _stackarr to the
 * allocated _arr, even if _newsize is smaller and _stackarr has room
 */

#define Vector_RESIZE(vec, _newsize)                                    \
  ({void** _oldarr = NULL;                                              \
    if (!(vec)->_arr) {                                                 \
      /* Moving from _stackarr to allocated _arr */                     \
      (vec)->_arr = malloc(sizeof(void*) * _newsize);                   \
      assert((vec)->_arr);                                              \
      memcpy((vec)->_arr, (vec)->_stackarr, sizeof(void*) * (vec)->offset); \
    } else {                                                            \
      /* Reallocating to new size (can be smaller) */                   \
      _oldarr = (vec)->_arr;                                            \
      (vec)->_arr = realloc((vec)->_arr, sizeof(void*) * _newsize);     \
      assert((vec)->_arr);                                              \
    }                                                                   \
    (vec)->size = _newsize;                                             \
    (vec)->_arr;})

/**
 * Check whether there is enough space in the vecfer. If not, calculate
 * a new size based on limits defined and call Vector_resize
 */
#define Vector_PREPARE(vec, len)                                        \
  ({uint32_t _requiredsize = (vec)->offset + len;                       \
    assert(_requiredsize <= (vec)->maxsize);                            \
                                                                        \
    if ((vec)->size < _requiredsize) {                                  \
      uint32_t _newsize = (vec)->size * 2;                              \
      if (_newsize < _requiredsize) { _newsize = _requiredsize; }       \
      else if (_newsize > (vec)->maxsize) { _newsize = (vec)->maxsize; } \
      Vector_RESIZE(vec, _newsize);                                     \
    }                                                                   \
    Vector_ARR(vec);})
