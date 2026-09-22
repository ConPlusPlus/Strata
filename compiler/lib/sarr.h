/* Copyright © 2026 Connor Rutberg */
/* sarr.h - Strata's dynamic array runtime (`T[dynamic]`).
 *
 * A type-erased growable array: elements are stored inline by value. The compiler tracks
 * the element type, so indexing/push are type-checked in Strata; the runtime only needs
 * the element size. Grows geometrically via realloc.
 *
 * v1 note: this uses malloc/realloc and is not freed individually (arena philosophy: don't
 * free piecemeal; the OS reclaims at exit). An allocator-aware version comes later.
 *
 * Part of the Strata RUNTIME (compiler/lib): GPL-3.0 WITH the runtime linking exception
 * (see ../../LICENSE-RUNTIME.md) - programs built with Strata are not covered by the GPL.
 */
#ifndef STRATA_SARR_H
#define STRATA_SARR_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    void*     data;
    long long len;
    long long cap;
    long long elem;   /* element size in bytes */
} Array;

static inline Array arr_make(long long elem) {
    Array a; a.data = 0; a.len = 0; a.cap = 0; a.elem = elem; return a;
}

static inline void arr_push(Array* a, const void* v) {
    if (a->len == a->cap) {
        long long nc = a->cap ? a->cap * 2 : 8;
        a->data = realloc(a->data, (size_t)(nc * a->elem));
        a->cap = nc;
    }
    memcpy((char*)a->data + a->len * a->elem, v, (size_t)a->elem);
    a->len += 1;
}

#endif /* STRATA_SARR_H */
