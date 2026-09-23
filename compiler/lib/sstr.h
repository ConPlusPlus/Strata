/* Copyright © 2026 Connor Rutberg */
/* sstr.h - Strata's string runtime.
 *
 * Strata's `string` lowers to C `const char*` (so it passes to C libraries with no
 * conversion). This header adds the operations the language exposes on strings:
 * concatenation, length, and equality. Concatenation allocates in a lazily-created
 * global string arena, so results are never freed individually (the arena discipline).
 *
 * Part of the Strata RUNTIME (compiler/lib): GPL-3.0 WITH the runtime linking exception
 * (see ../../LICENSE-RUNTIME.md) - programs built with Strata are not covered by the GPL.
 */
#ifndef STRATA_SSTR_H
#define STRATA_SSTR_H

#include <string.h>
#include <stdio.h>
#include <arena.h>

static Arena strata_str_arena_v;
static int   strata_str_arena_ready = 0;
static inline Arena* strata_str_arena(void) {
    if (!strata_str_arena_ready) { strata_str_arena_v = arena_make(); strata_str_arena_ready = 1; }
    return &strata_str_arena_v;
}

static inline const char* str_concat(const char* x, const char* y) {
    size_t lx = strlen(x), ly = strlen(y);
    char* r = (char*)arena_alloc(strata_str_arena(), lx + ly + 1);
    memcpy(r, x, lx); memcpy(r + lx, y, ly); r[lx + ly] = '\0';
    return r;
}
static inline int      str_eq(const char* x, const char* y) { return strcmp(x, y) == 0; }
static inline long long str_len(const char* x) { return (long long)strlen(x); }

/* substr(s, start, n): n bytes from `start`, clamped to the string's bounds. */
static inline const char* str_sub(const char* s, long long start, long long n) {
    long long len = (long long)strlen(s);
    if (start < 0) start = 0;
    if (start > len) start = len;
    if (n < 0) n = 0;
    if (start + n > len) n = len - start;
    char* r = (char*)arena_alloc(strata_str_arena(), (size_t)n + 1);
    memcpy(r, s + start, (size_t)n); r[n] = '\0';
    return r;
}

/* int_to_str(n): decimal text of an integer. */
static inline const char* str_from_int(long long v) {
    char tmp[32];
    int n = snprintf(tmp, sizeof tmp, "%lld", v);
    char* r = (char*)arena_alloc(strata_str_arena(), (size_t)n + 1);
    memcpy(r, tmp, (size_t)n + 1);
    return r;
}

#endif /* STRATA_SSTR_H */
