# Changelog

All notable changes to Strata are recorded here. Versions follow
[Semantic Versioning](https://semver.org/); the project is pre-1.0, so the API and language
may still change between minor versions. Each version has a matching `vX.Y.Z` git tag and a
GitHub Release.

## [Unreleased]
- nothing yet.

## [0.11.0] - 2026-09-22
### Added
- **Enums** now compile to C enums (`enum State { Idle, Walk, Jump }`).
- **`switch`** statements: `switch x { case A: ... case B, C: ... default: ... }`, with
  **no fall-through** (each case breaks) and multi-value cases. Works on ints and enums.
  This is the AST-dispatch pattern needed to eventually self-host the compiler in Strata.

## [0.10.0] - 2026-09-22
### Added
- **Dynamic arrays** (`T[dynamic]`): array literals `[a, b, c]`, `.push(v)`, `.len`,
  indexing (including as an lvalue: `xs[i].field = ...`), and `for x in xs` iteration.
- Works for struct elements, so **entity lists** work (`examples/balls.strata` — 60
  bouncing balls, a `Ball[dynamic]` with vec2 physics + raylib).
- Type-erased `Array` runtime (`compiler/lib/sarr.h`).

## [0.9.0] - 2026-09-22
### Added
- **C library linking:** the `link "name"` directive adds `-lname` to the build.
- Unknown *names* resolve as external C symbols once a header is imported (e.g. raylib
  constants like `RAYWHITE`).
- `examples/window.strata` — a raylib window written in Strata, built to a single native
  binary. (`examples/sprite.strata`, an arrow-key-driven sprite, followed.)

## [0.8.0] - 2026-09-21
### Added
- **Strings:** `string` (a C `const char*`) with concatenation (`+`), `.len`, and `==`/`!=`
  (`compiler/lib/sstr.h`).
- **C interop:** `import "h.h"` / `import <h.h>` emits `#include` and enables calling
  external C functions directly (verified against libc `puts`/`abs`).

## [0.7.0] - 2026-09-21
### Added
- **Matrices and quaternions:** `mat4` (mul, transform, `translate`/`scale`/`rotate`/
  `perspective`/`look_at`), `quat` (mul, axis-angle, rotate, `to_mat4`, normalize).
- **Vector swizzles:** `v.xy`, chained (`a.zw.y`).
### Project
- SPDX + copyright headers on all source; CLA, commercial-license offer, contributing guide.

## [0.6.0] - 2026-09-21
### Added
- **First-class vectors:** `vec2/3/4` with constructors, component-wise `+`/`-`/`*`, scalar
  scale, `.x/.y/.z/.w`, and `dot`/`cross`/`length`/`normalize` (`compiler/lib/smath.h`).
- The flagship `examples/hello.strata` (struct + vec3 + arena, no GC) runs end-to-end.

## Pre-release (milestone 1) - 2026-09-20 to 2026-09-21
Built before the first tagged release:
- Lexer (significant newlines, `..` ranges, types-first tokens).
- Parser (recursive descent; structs, functions, `var`/typed decls, paren-free control flow).
- Type checker (name resolution, type checking, `var` inference).
- C codegen; arena/region memory (`compiler/lib/arena.h`).
- The compiler (`stratac`), written in D--, split into a reusable core + front-ends
  (`stratac` CLI, `console`, `libstrata.dll`), with a byte-for-byte golden test suite.

[Unreleased]: https://github.com/UseStrata/Strata/compare/v0.11.0...HEAD
[0.11.0]: https://github.com/UseStrata/Strata/releases/tag/v0.11.0
[0.10.0]: https://github.com/UseStrata/Strata/releases/tag/v0.10.0
[0.9.0]: https://github.com/UseStrata/Strata/releases/tag/v0.9.0
[0.8.0]: https://github.com/UseStrata/Strata/releases/tag/v0.8.0
[0.7.0]: https://github.com/UseStrata/Strata/releases/tag/v0.7.0
[0.6.0]: https://github.com/UseStrata/Strata/releases/tag/v0.6.0
