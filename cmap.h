#ifndef CMAP_H_
#define CMAP_H_

#include "cvector.h"

#define CMap(tag)      CMap_##tag##_t
#define CMapEntry(tag) CMapEntry_##tag##_t
#define CMapIter(tag)  CMapIter_##tag##_t

#define cmap_initializer  {cvector_initializer, 0}
#define cmap_size(cm)     ((cvector_size_t) (cm)._size)
#define cmap_capacity(cm) cvector_capacity((cm)._vec)


// CMapEntry:
#define declare_CMapEntry(tag, Key, Value, keyDestroy, valueDestroy) \
struct CMapEntry(tag) { Key key; Value value; short untouched, _used; }; \
 \
static inline void cmapentry_##tag##_destroy(struct CMapEntry(tag)* p) { \
    keyDestroy(&p->key); \
    valueDestroy(&p->value); \
    p->_used = 0; \
} \
typedef struct CMapEntry(tag) CMapEntry(tag)


// CMap:
#define declare_CMap(...)  cdef_MACRO_OVERLOAD(declare_CMap, __VA_ARGS__)

#define declare_CMap_3(tag, Key, Value) \
    declare_CMap_4(tag, Key, Value, cdef_destroy)

#define declare_CMap_4(tag, Key, Value, valueDestroy) \
    declare_CMap_10(tag, Key, Value, valueDestroy, Key, cdef_getRaw, memcmp, cdef_murmurHash, cdef_initRaw, cdef_destroy)


// CMap<CString, Value>:
#define declare_CMap_STR(...)  cdef_MACRO_OVERLOAD(declare_CMap_STR, __VA_ARGS__)

#define declare_CMap_STR_2(tag, Value) \
    declare_CMap_STR_3(tag, Value, cdef_destroy)

#define declare_CMap_STR_3(tag, Value, valueDestroy) \
    declare_CMap_10(tag, CString, Value, valueDestroy, const char*, cstring_getRaw, cstring_compare, cstring_hash, cstring_make, cstring_destroy)


// CMap full:
#define declare_CMap_10(tag, Key, Value, valueDestroy, KeyRaw, keyGetRaw, keyCompare, keyHasher, keyInit, keyDestroy) \
  declare_CMapEntry(tag, Key, Value, keyDestroy, valueDestroy); \
  declare_CVector_3(_map##tag, CMapEntry(tag), cmapentry_##tag##_destroy); \
 \
typedef struct CMap(tag) { \
    CVector(_map##tag) _vec; \
    cvector_size_t _size; \
} CMap(tag); \
 \
typedef struct CMapIter(tag) { \
    CMapEntry(tag) *item, *_end; \
} CMapIter(tag); \
 \
static inline CMap(tag) cmap_##tag##_init(void) { \
    CMap(tag) map = cmap_initializer; \
    return map; \
} \
 \
static inline void cmap_##tag##_destroy(CMap(tag)* self) { \
    if (self->_size) { \
        cvector_size_t cap = _cvector_capacity(self->_vec); \
        CMapEntry(tag)* p = self->_vec.data, *end = p + cap; \
        for (; p != end; ++p) if (p->_used) cmapentry_##tag##_destroy(p); \
    } \
    cvector__map##tag##_destroy(&self->_vec); \
} \
 \
static inline void cmap_##tag##_clear(CMap(tag)* self) { \
    CMap(tag) cm = cmap_initializer; \
    cmap_##tag##_destroy(self); \
    *self = cm; \
} \
 \
static inline void cmap_##tag##_swap(CMap(tag)* a, CMap(tag)* b) { \
    cvector__map##tag##_swap(&a->_vec, &b->_vec); \
    _cdef_swap(cvector_size_t, a->_size, b->_size); \
} \
 \
static inline cvector_size_t _cmap_##tag##_findIndex(CMap(tag) cm, KeyRaw rawKey) { \
    cvector_size_t cap = cvector_capacity(cm._vec); \
    cvector_size_t idx = keyHasher(&rawKey, sizeof(Key)) % cap, first = idx; \
    FIBONACCI_DECL; \
    while (cm._vec.data[idx]._used && keyCompare(&cm._vec.data[idx].key, &rawKey, sizeof(Key)) != 0) \
        idx = (first + FIBONACCI_NEXT) % cap; \
    return idx; \
} \
 \
static inline CMapEntry(tag)* cmap_##tag##_get(CMap(tag) cm, KeyRaw rawKey) { \
    if (cm._size == 0) return NULL; \
    cvector_size_t idx = _cmap_##tag##_findIndex(cm, rawKey); \
    return cm._vec.data[idx]._used ? &cm._vec.data[idx] : NULL; \
} \
 \
static inline cvector_size_t cmap_##tag##_rehash(CMap(tag)* self); /* predeclared */ \
 \
static inline CMapEntry(tag)* cmap_##tag##_put(CMap(tag)* self, KeyRaw rawKey, Value value) { \
    cvector_size_t cap = cvector_capacity(self->_vec); \
    if (self->_size >= cap * 8 / 10) \
        cap = cmap_##tag##_rehash(self); \
    cvector_size_t idx = _cmap_##tag##_findIndex(*self, rawKey); \
    CMapEntry(tag)* e = &self->_vec.data[idx]; \
    e->value = value; \
    e->untouched = !e->_used; \
    if (!e->_used) { \
        e->key = keyInit(rawKey); \
        e->_used = 1; \
        ++self->_size; \
    } \
    return e; \
} \
 \
static inline cvector_size_t cmap_##tag##_rehash(CMap(tag)* self) { \
    CVector(_map##tag) vec = cvector_initializer; \
    cvector_size_t newcap = 7 + cmap_capacity(*self) * 2; \
    cvector__map##tag##_swap(&self->_vec, &vec); \
    cvector__map##tag##_reserve(&self->_vec, newcap); \
    self->_size = 0; \
    memset(self->_vec.data, 0, sizeof(CMapEntry(tag)) * newcap); \
    CMapEntry(tag)* p = vec.data; \
    cvector_size_t i, oldcap = cvector_capacity(vec); \
    for (i = 0; i < oldcap; ++i, ++p) \
        if (p->_used) cmap_##tag##_put(self, keyGetRaw(p->key), p->value); \
    return newcap; \
} \
 \
static inline int cmap_##tag##_erase(CMap(tag)* self, KeyRaw rawKey) { \
    CMapEntry(tag)* entryPtr = cmap_##tag##_get(*self, rawKey); \
    if (entryPtr) { \
        cmapentry_##tag##_destroy(entryPtr); \
        --self->_size; \
        return 1; \
    } \
    return 0; \
} \
 \
static inline CMapIter(tag) cmap_##tag##_begin(CMap(tag) map) { \
    CMapIter(tag) null = {NULL, NULL}; \
    if (map._size == 0) return null; \
    CMapEntry(tag)* p = map._vec.data, *end = p + _cvector_capacity(map._vec); \
    while (p != end && !p->_used) ++p; \
    CMapIter(tag) it = {p, end}; return it; \
} \
 \
static inline CMapIter(tag) cmap_##tag##_next(CMapIter(tag) iter) { \
    ++iter.item; \
    while (iter.item != iter._end && !iter.item->_used) ++iter.item; \
    return iter; \
} \
 \
static inline CMapIter(tag) cmap_##tag##_end(CMap(tag) map) { \
    CMapEntry(tag)* end = (map._size == 0) ? NULL : map._vec.data + _cvector_capacity(map._vec); \
    CMapIter(tag) iter = {end, end}; \
    return iter; \
} \
typedef Key cmap_##tag##_key_t; \
typedef Value cmap_##tag##_value_t


#define FIBONACCI_DECL  cvector_size_t fib1 = 1, fib2 = 2, fibx
#define FIBONACCI_NEXT  (fibx = fib1 + fib2, fib1 = fib2, fib2 = fibx)

#endif
