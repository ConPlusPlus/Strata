/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright © 2026 Connor Rutberg */
/* src/strata_host.h - the compiler's link to whatever is hosting it.
 *
 * The compiler is written in Strata, but a few things it needs are plain process state,
 * which Strata (no global variables) keeps here in C. Compiler modules
 * `import "strata_host.h"`:
 *
 *   strata_report(msg)     every message for the user (errors, "built ...") goes through
 *                          this: printed by the CLI, captured into a buffer when a host
 *                          (an engine, via libstrata) asks for it
 *   strata_host_reset()    free everything the compiler allocated (strings, boxed values,
 *                          arrays), so an engine can compile over and over without growing
 *   strata_host_libdir()   the runtime lib/ folder: set by the host, or found next to
 *                          libstrata.dll
 *
 * It sits beside the compiler's sources (not in lib/) so the pinned bootstrap release can
 * still compile the compiler. */
#ifndef STRATA_HOST_H
#define STRATA_HOST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arena.h>
#include <sstr.h>
#include <sarr.h>

/* ---- messages ------------------------------------------------------------- */

static char*  strata_host_buf = NULL;
static size_t strata_host_len = 0;
static size_t strata_host_cap = 0;
static int    strata_host_capturing = 0;

static inline void strata_report(const char* msg) {
    if (!strata_host_capturing) { puts(msg); return; }
    size_t n = strlen(msg);
    if (strata_host_len + n + 2 > strata_host_cap) {
        size_t cap = strata_host_cap ? strata_host_cap : 256;
        while (strata_host_len + n + 2 > cap) cap *= 2;
        strata_host_buf = (char*)realloc(strata_host_buf, cap);
        strata_host_cap = cap;
    }
    memcpy(strata_host_buf + strata_host_len, msg, n);
    strata_host_len += n;
    strata_host_buf[strata_host_len++] = '\n';
    strata_host_buf[strata_host_len] = '\0';
}

/* Start capturing messages (clears the previous capture). */
static inline void strata_capture_begin(void) {
    strata_host_capturing = 1;
    strata_host_len = 0;
    if (strata_host_buf) strata_host_buf[0] = '\0';
}
static inline void strata_capture_end(void) { strata_host_capturing = 0; }
/* The captured messages, one per line ("" if none). Survives strata_host_reset(). */
static inline const char* strata_capture_text(void) { return strata_host_buf ? strata_host_buf : ""; }

/* ---- joining strings -------------------------------------------------------- */

/* Concatenate a string[dynamic] in one pass: measure every piece, allocate once, copy
 * once. (In Strata, joining n pieces with `+` copies the growing result each time.) */
static inline const char* strata_join(const Array* parts) {
    const char** p = (const char**)parts->data;
    size_t total = 0;
    for (long long i = 0; i < parts->len; i++) total += strlen(p[i]);
    char* r = (char*)arena_alloc(strata_str_arena(), total + 1);
    size_t at = 0;
    for (long long i = 0; i < parts->len; i++) {
        size_t n = strlen(p[i]);
        memcpy(r + at, p[i], n);
        at += n;
    }
    r[total] = '\0';
#ifdef STRATA_STR_LEN_CACHE
    strata_len_put(r, total);
#endif
    return r;
}

/* ---- memory --------------------------------------------------------------- */

/* Free one dynamic array's buffer now (e.g. a module's tokens once it's parsed) and leave
 * it empty. Only for arrays nothing else still points into. */
static inline void strata_array_free(Array* a) {
#ifdef STRATA_ARR_TRACKED
    if (a->data) {
        StrataArrLink* h = ((StrataArrLink*)a->data) - 1;
        h->prev->next = h->next;
        h->next->prev = h->prev;
        free(h);
    }
#else
    free(a->data);           /* older runtimes: a plain malloc'd buffer */
#endif
    a->data = 0; a->len = 0; a->cap = 0;
}

static inline void strata_host_reset(void) {
#ifdef STRATA_STR_LEN_CACHE   /* frees the strings and forgets their remembered lengths */
    strata_str_reset();
#else
    arena_free(strata_str_arena());
#endif
    arena_free(strata_heap());
#ifdef STRATA_ARR_TRACKED   /* older runtimes (e.g. the bootstrap release's) can't free arrays */
    strata_arr_free_all();
#endif
}

/* ---- the runtime lib/ folder ------------------------------------------------ */

static char strata_host_libdir_buf[1024];

static inline void strata_host_set_libdir(const char* dir) {
    snprintf(strata_host_libdir_buf, sizeof strata_host_libdir_buf, "%s", dir ? dir : "");
}

#ifdef _WIN32
/* Declared here rather than including <windows.h>, whose macros would collide with names
 * in the compiler's generated C. */
__declspec(dllimport) int __stdcall GetModuleHandleExA(unsigned long flags, const char* name, void** module);
__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void* module, char* path, unsigned long size);
#endif

static inline int strata_host_is_libdir(const char* dir) {
    char probe[1100];
    snprintf(probe, sizeof probe, "%s/arena.h", dir);
    FILE* f = fopen(probe, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

/* The host's choice; else lib/ next to the module containing this code (for libstrata.dll:
 * the install folder), or one level up (the repo layout: bin/../lib); else "lib". */
static inline const char* strata_host_libdir(void) {
    if (strata_host_libdir_buf[0]) return strata_host_libdir_buf;
#ifdef _WIN32
    void* mod = NULL;
    /* 0x4 = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 0x2 = ..._UNCHANGED_REFCOUNT */
    if (GetModuleHandleExA(0x4 | 0x2, (const char*)(void*)&strata_host_libdir, &mod)) {
        char path[1024];
        unsigned long n = GetModuleFileNameA(mod, path, sizeof path);
        if (n > 0 && n < sizeof path) {
            char* slash = strrchr(path, '\\');
            if (slash) {
                *slash = '\0';
                snprintf(strata_host_libdir_buf, sizeof strata_host_libdir_buf, "%s/lib", path);
                if (strata_host_is_libdir(strata_host_libdir_buf)) return strata_host_libdir_buf;
                snprintf(strata_host_libdir_buf, sizeof strata_host_libdir_buf, "%s/../lib", path);
                if (strata_host_is_libdir(strata_host_libdir_buf)) return strata_host_libdir_buf;
                strata_host_libdir_buf[0] = '\0';
            }
        }
    }
#endif
    return "lib";
}

#endif /* STRATA_HOST_H */
