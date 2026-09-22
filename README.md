# Strata™

A statically-typed, compiled programming language **for games and real-time software**.

> *"Safer than C, simpler than Rust — the control C gives games, without the footguns
> or the borrow-checker fight."*

Strata compiles to **plain C** (then to native code via tcc/gcc/clang), so it inherits the
whole C toolchain and drops into any C engine. Its four ideas:

- **Arena / region memory** — no garbage collector, no manual `free`. *Everything in a
  region dies together.*
- **First-class data-oriented & math types** — `vec2/3/4`, `mat4`, quaternions, opt-in SoA.
- **Seamless C interop** — it *is* C underneath; targets any C engine + Godot (GDExtension).
- **Hot-reload** — a pluggable runtime, following the Handmade/Jai host-module pattern.

```strata
struct Entity { vec3 pos; vec3 vel; int hp }

vec3 update(Entity* e, float dt) = e.pos + e.vel * dt

var world = arena()
var e = world.new(Entity)
e.vel = vec3(1, 0, 0)

for i in 0..60 {
    e.pos = update(e, 0.016)
}
```

## Status

Early but real — programs **compile and run today** (compiler at v0.9.0):

- ✅ lexer, parser, type checker, C codegen
- ✅ arena/region memory, structs, functions, control flow
- ✅ first-class math: `vec2/3/4`, `mat4`, quaternions, swizzles
- ✅ strings, and **C interop** (`import` a header, `link` a library, call C directly)
- ✅ **dynamic arrays** (`T[dynamic]`: literals, `push`, `len`, indexing, `for x in xs`) — entity lists
- ✅ **games written in Strata** — a raylib window ([`window.strata`](compiler/examples/window.strata))
  and an **arrow-key-driven sprite** ([`sprite.strata`](compiler/examples/sprite.strata), movement
  computed with Strata's own `vec2` math) build to single native binaries
- ⏳ next: a small prelude (input/time), C struct/enum binding, then SoA arrays and hot-reload

The compiler is called **`stratac`** and is itself written in D-- (a self-hosting
C-compiling language), which produces plain C.

## Layout

```
compiler/   the stratac compiler (written in D--)  — see compiler/ARCHITECTURE.md
website/    design + documentation                 — see website/design/DESIGN.md
Strata.md   the founding project plan
```

## Build & run

Requires the D-- compiler (`dec`) and a C compiler (`gcc`). See
[`compiler/ARCHITECTURE.md`](compiler/ARCHITECTURE.md) for the toolchain.

```powershell
# build stratac.exe, console.exe, libstrata.dll into compiler\bin\
powershell -ExecutionPolicy Bypass -File compiler\build.ps1

# run a program
compiler\bin\stratac.exe run compiler\examples\run1.strata

# install to %LOCALAPPDATA%\Programs\strata and add to PATH
powershell -ExecutionPolicy Bypass -File compiler\install.ps1
```

`stratac` subcommands: `run`, `build`, `check`, `emit`, `ast`, `tokens`.

Tests (byte-for-byte golden files per compiler stage):

```powershell
powershell -ExecutionPolicy Bypass -File compiler\tests\run.ps1
```

## License

The Strata **compiler** is licensed under **GPL-3.0** ([`LICENSE`](LICENSE)) — modify it
and distribute your version, and you publish your source (the "Linux approach").

The Strata **runtime** carries a linking exception ([`LICENSE-RUNTIME.md`](LICENSE-RUNTIME.md)),
so **programs you build with Strata are entirely yours** — license and sell them under any
terms you like, open or closed. The GPL covers the compiler, never what you make with it.

**Commercial license:** to modify the *compiler* and keep your changes private (no GPL
disclosure), a commercial license is available from **$100** — see [`COMMERCIAL.md`](COMMERCIAL.md).
Games built with Strata never need this.

**Strata™** — the name and branding are trademarks of the project author (common-law).
The code is GPL; the *name* is not — forks must use a different name. See
[`TRADEMARK.md`](TRADEMARK.md).

**Contributing:** by contributing you agree to the [CLA](CLA.md) — you keep your copyright,
and grant the project the right to use and relicense your contribution. See
[`CONTRIBUTING.md`](CONTRIBUTING.md).

Copyright © 2026 Connor Rutberg. Strata is free software under GPL-3.0; see [`LICENSE`](LICENSE).
