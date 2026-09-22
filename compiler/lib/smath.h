/* smath.h - Strata's first-class vector math runtime (milestone 2).
 *
 * vec2/vec3/vec4 as plain float structs, with component-wise + - *, scalar scale, dot,
 * cross (vec3), length, normalize. The compiler lowers Strata's `a + b`, `a * s`, etc. to
 * these calls using the operand types the checker resolved. Header-only, `static inline`.
 *
 * Named smath.h (not math.h) so it never shadows the C standard <math.h> it pulls in.
 *
 * Part of the Strata RUNTIME (compiler/lib): GPL-3.0 WITH the runtime linking exception
 * (see ../../LICENSE-RUNTIME.md) - programs built with Strata are not covered by the GPL.
 */
#ifndef STRATA_SMATH_H
#define STRATA_SMATH_H

#include <math.h>

typedef struct { float x, y; }       vec2;
typedef struct { float x, y, z; }    vec3;
typedef struct { float x, y, z, w; } vec4;

/* ---- vec2 ---- */
static inline vec2  vec2_add(vec2 a, vec2 b)   { return (vec2){ a.x+b.x, a.y+b.y }; }
static inline vec2  vec2_sub(vec2 a, vec2 b)   { return (vec2){ a.x-b.x, a.y-b.y }; }
static inline vec2  vec2_mul(vec2 a, vec2 b)   { return (vec2){ a.x*b.x, a.y*b.y }; }
static inline vec2  vec2_scale(vec2 a, float s){ return (vec2){ a.x*s, a.y*s }; }
static inline float vec2_dot(vec2 a, vec2 b)   { return a.x*b.x + a.y*b.y; }
static inline float vec2_length(vec2 a)        { return sqrtf(vec2_dot(a, a)); }
static inline vec2  vec2_normalize(vec2 a)     { float l = vec2_length(a); if (l == 0.0f) { return a; } return vec2_scale(a, 1.0f/l); }

/* ---- vec3 ---- */
static inline vec3  vec3_add(vec3 a, vec3 b)   { return (vec3){ a.x+b.x, a.y+b.y, a.z+b.z }; }
static inline vec3  vec3_sub(vec3 a, vec3 b)   { return (vec3){ a.x-b.x, a.y-b.y, a.z-b.z }; }
static inline vec3  vec3_mul(vec3 a, vec3 b)   { return (vec3){ a.x*b.x, a.y*b.y, a.z*b.z }; }
static inline vec3  vec3_scale(vec3 a, float s){ return (vec3){ a.x*s, a.y*s, a.z*s }; }
static inline float vec3_dot(vec3 a, vec3 b)   { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline vec3  vec3_cross(vec3 a, vec3 b) { return (vec3){ a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x }; }
static inline float vec3_length(vec3 a)        { return sqrtf(vec3_dot(a, a)); }
static inline vec3  vec3_normalize(vec3 a)     { float l = vec3_length(a); if (l == 0.0f) { return a; } return vec3_scale(a, 1.0f/l); }

/* ---- vec4 ---- */
static inline vec4  vec4_add(vec4 a, vec4 b)   { return (vec4){ a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w }; }
static inline vec4  vec4_sub(vec4 a, vec4 b)   { return (vec4){ a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w }; }
static inline vec4  vec4_mul(vec4 a, vec4 b)   { return (vec4){ a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w }; }
static inline vec4  vec4_scale(vec4 a, float s){ return (vec4){ a.x*s, a.y*s, a.z*s, a.w*s }; }
static inline float vec4_dot(vec4 a, vec4 b)   { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
static inline float vec4_length(vec4 a)        { return sqrtf(vec4_dot(a, a)); }
static inline vec4  vec4_normalize(vec4 a)     { float l = vec4_length(a); if (l == 0.0f) { return a; } return vec4_scale(a, 1.0f/l); }

#endif /* STRATA_SMATH_H */
