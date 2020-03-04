#ifndef CVECTOR__H__
#define CVECTOR__H__

#include <malloc.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "cdef.h"

typedef uint32_t cvector_size_t;

#define _cvector_size(cv) ((cvector_size_t *)(cv).data)[-2]
#define _cvector_capacity(cv) ((cvector_size_t *)(cv).data)[-1]

static inline cvector_size_t* _cvector_alloced(void* data) {
    return data ? ((cvector_size_t *) data) - 2 : NULL;
}
static inline cvector_size_t _cvector_safe_size(const void* data) {
    return data ? ((const cvector_size_t *) data)[-2] : 0;
}
static inline cvector_size_t _cvector_safe_capacity(const void* data) {
    return data ? ((const cvector_size_t *) data)[-1] : 0;
}


#define CVector(tag)           CVector_##tag##_t
#define CVectorIter(tag)       CVectorIter_##tag##_t

#define cvector_initializer    {NULL}
#define cvector_size(cv)       _cvector_safe_size((cv).data)
#define cvector_capacity(cv)   _cvector_safe_capacity((cv).data)
#define cvector_empty(cv)      (_cvector_safe_size((cv).data) == 0)

#define declare_CVector(...)       cdef_MACRO_OVERLOAD(declare_CVector, __VA_ARGS__)
#define declare_CVector_2(tag, Value)  declare_CVector_3(tag, Value, cdef_destroy)
#define declare_CVector_STR(tag)   declare_CVector_3(tag, CString, cstring_destroy)


#define declare_CVector_3(tag, Value, valueDestroy) \
typedef struct CVector(tag) { \
    Value* data; \
} CVector(tag); \
typedef struct CVectorIter(tag) { \
    Value* item; \
} CVectorIter(tag); \
 \
static inline CVector(tag) cvector_##tag##_init(void) { \
    CVector(tag) cv = cvector_initializer; \
    return cv; \
} \
 \
static inline void cvector_##tag##_swap(CVector(tag)* a, CVector(tag)* b) { \
    Value* data = a->data; a->data = b->data; b->data = data; \
} \
 \
static inline void cvector_##tag##_destroy(CVector(tag)* self) { \
    Value* p = self->data; \
    cvector_size_t i = 0, n = cvector_size(*self); \
    for (; i < n; ++p, ++i) valueDestroy(p); \
    free(_cvector_alloced(self->data)); \
} \
 \
static inline void cvector_##tag##_reserve(CVector(tag)* self, cvector_size_t cap) { \
    if (cap > cvector_capacity(*self)) { \
        cvector_size_t len = cvector_size(*self); \
        cvector_size_t* rep = (cvector_size_t *) realloc(_cvector_alloced(self->data), 2 * sizeof(cvector_size_t) + cap * sizeof(Value)); \
        self->data = (Value *) (rep + 2); \
        rep[0] = len; \
        rep[1] = cap; \
    } \
} \
 \
 \
static inline CVector(tag) cvector_##tag##_make(const Value* data, cvector_size_t len) { \
    CVector(tag) cv = cvector_initializer; \
    if (len) { \
        cvector_##tag##_reserve(&cv, len); \
        memcpy(cv.data, data, len * sizeof(Value)); \
        _cvector_size(cv) = len; \
    } \
    return cv; \
} \
 \
static inline void cvector_##tag##_clear(CVector(tag)* self) { \
    CVector(tag) cv = cvector_initializer; \
    cvector_##tag##_destroy(self); \
    *self = cv; \
} \
 \
 \
static inline void cvector_##tag##_push(CVector(tag)* self, Value value) { \
    cvector_size_t newsize = cvector_size(*self) + 1; \
    if (newsize > cvector_capacity(*self)) \
        cvector_##tag##_reserve(self, 7 + newsize * 5 / 3); \
    self->data[cvector_size(*self)] = value; \
    _cvector_size(*self) = newsize; \
} \
 \
static inline void cvector_##tag##_insert(CVector(tag)* self, cvector_size_t pos, Value value) { \
    cvector_##tag##_push(self, value); \
    cvector_size_t len = cvector_size(*self); \
    memmove(&self->data[pos + 1], &self->data[pos], (len - pos - 1) * sizeof(Value)); \
    self->data[pos] = value; \
} \
 \
 \
static inline Value cvector_##tag##_back(CVector(tag) cv) { \
    return cv.data[_cvector_size(cv) - 1]; \
} \
 \
 \
static inline void cvector_##tag##_pop(CVector(tag)* self) { \
    valueDestroy(&self->data[_cvector_size(*self) - 1]); \
    --_cvector_size(*self); \
} \
 \
 \
static inline CVectorIter(tag) cvector_##tag##_begin(CVector(tag) vec) { \
    CVectorIter(tag) iter = {vec.data}; \
    return iter; \
} \
 \
static inline CVectorIter(tag) cvector_##tag##_next(CVectorIter(tag) iter) { \
    ++iter.item; \
    return iter; \
} \
 \
static inline CVectorIter(tag) cvector_##tag##_end(CVector(tag) vec) { \
    CVectorIter(tag) iter = {vec.data + cvector_size(vec)}; \
    return iter; \
} \
typedef Value cvector_##tag##_value_t

#endif
