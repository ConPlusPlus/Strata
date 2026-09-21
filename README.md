# Strata

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

Early. Milestone 1 (the compiler skeleton) is in progress:

- ✅ lexer, parser, type checker, and minimal codegen — small programs **run today**
- ⏳ next: the `lib/` runtime (arenas, strings), then first-class vector/matrix types

The compiler is called **`stratac`** and is itself written in [D--](https://github.com/)
(a self-hosting C-compiling language), which produces plain C.

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

Not yet chosen (candidates: MIT / Apache-2.0 / zlib). See `Strata.md` §6.
