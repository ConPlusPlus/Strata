# Strata — Handoff

> A complete, pick-up-cold guide to the Strata project: what it is, how the compiler
> works, how to build/test/run it, what's implemented, what's in progress, and what's
> planned. If you read one document, read this one, then dip into
> [`compiler/ARCHITECTURE.md`](compiler/ARCHITECTURE.md) (compiler internals) and
> [`website/design/DESIGN.md`](website/design/DESIGN.md) (language design) for detail.

Current version: **stratac 1.1.0** · tests: **43/43** · repo: **https://github.com/UseStrata/Strata**

---

## 1. What Strata is

A statically-typed, **compiled** programming language **for games and real-time software**.
It compiles to **plain C**, then to a native binary via a C compiler (gcc today; tcc
planned for instant runs). Pitch: *"safer than C, simpler than Rust — the control C gives
games, without the footguns or the borrow-checker fight."*

Four ideas:
- **Arena / region memory** — no garbage collector, no manual `free`. *Everything in a
  region dies together.*
- **First-class data-oriented & math types** — `vec2/3/4`, `mat4`, quaternions (SoA planned).
- **Seamless C interop** — it *is* C underneath; `import` a header, `link` a library, call C.
- **Hot-reload** — planned runtime (not built yet).

The compiler is **`stratac`**, and it is **written in Strata** (`compiler/src/`): it
compiles itself. The build starts from a pinned, released `stratac` (see §3). It was
originally written in **D--**, a separate C-compiling language (`C:\DMinusMinus`). That
version is retired to `archive/dminusminus-seed/`.

---

## 2. Repository layout

```
Strata/
├─ README.md              project front page
├─ HANDOFF.md             this file
├─ CHANGELOG.md           per-version history (v0.6.0 .. v1.1.0)
├─ Strata.md              the original founding plan
├─ LICENSE                GPL-3.0 (the compiler)
├─ LICENSE-RUNTIME.md     runtime linking exception (so games aren't GPL)
├─ CLA.md                 contributor license agreement (enables dual licensing)
├─ COMMERCIAL.md          the paid commercial-license offer
├─ TRADEMARK.md           the "Strata" name policy
├─ compiler/
│  ├─ ARCHITECTURE.md     how the compiler is built (READ before touching src/)
│  ├─ build.ps1           build all artifacts into bin/
│  ├─ install.ps1         install the toolchain + add to PATH
│  ├─ package.ps1         build a release .zip into dist/
│  ├─ src/                THE COMPILER, written in Strata (see §4)
│  ├─ bootstrap.txt       the pinned release version the build bootstraps from
│  ├─ build/              bootstrap compilers: cached release + stage1/2 (gitignored)
│  ├─ lib/                the RUNTIME the compiled programs link against (see §7)
│  ├─ examples/           sample .strata programs (see §8)
│  ├─ tests/              golden-file tests + run.ps1 (see §6)
│  ├─ bin/                build output (gitignored)
│  └─ dist/               release archives (gitignored)
├─ website/design/DESIGN.md   the language design ("proposal to react to")
├─ editors/              syntax highlighting: VS Code + Visual Studio (see editors/README.md)
└─ archive/               retired: the D-- compiler + the D--→Strata translator
```

---

## 3. Toolchain & how to build / run / test

**Prerequisites (already set up on this machine):**
- **gcc** — via MSYS2 at `C:\msys64\mingw64\bin` (on PATH). Strata shells out to it.
- **raylib** — installed via MSYS2 (`C:\msys64\usr\bin\pacman.exe -S mingw-w64-x86_64-raylib`),
  for the graphical examples.

**Build the compiler** (produces `stratac.exe`, `console.exe`, `libstrata.dll` in `bin/`):
```
powershell -ExecutionPolicy Bypass -File compiler\build.ps1
```
This is a **bootstrap**. stage0 is the pinned release named in `compiler/bootstrap.txt`,
downloaded from GitHub once and cached in `build/`. Pass `-Bootstrap <stratac.exe>` to
use another compiler; if the download fails, the installed `stratac` on PATH is used.
Then stage0 builds `src/stratac.strata` → `stage1.exe`, and stage1 builds it again →
`stage2.exe`. The build **fails unless stage1 and stage2 emit byte-identical C** (the
fixpoint). stage2 is copied to `bin/stratac.exe`.

Quick iteration on the compiler: `compiler\bin\stratac.exe build compiler\src\stratac.strata`
(produces `src\stratac.exe`), then run the full `build.ps1` + tests before committing.

**Use the compiler:**
```
stratac tokens <file.strata>   # lexer output
stratac ast    <file.strata>   # parsed AST
stratac check  <file.strata>   # type-check only
stratac emit   <file.strata>   # print the generated C
stratac build  <file.strata>   # compile to a native .exe
stratac run    <file.strata>   # compile and run
```
`stratac` finds its runtime `lib/` next to the executable (it passes `-I <lib>` to gcc).

**Test** (rebuilds `stratac`, runs every golden byte-for-byte):
```
powershell -ExecutionPolicy Bypass -File compiler\tests\run.ps1
```

**Install** to `%LOCALAPPDATA%\Programs\strata` and add to PATH: `compiler\install.ps1`.
**Editor highlighting:** `editors\vscode\install.ps1` (VS Code, no admin) and
`editors\visualstudio\install.ps1` (Visual Studio, asks for admin). The grammar is generated by
`python editors\build_grammar.py`; when you add a keyword, type or built-in, add it there too.
**Package** a release zip: `compiler\package.ps1 -Version X.Y.Z` (into `dist/`).

---

## 4. How the compiler works

A strict, one-way pipeline (the anti-spaghetti design — see ARCHITECTURE.md):

```
file.strata → lexer → tokens → parser → AST → checker → typed AST → codegen → C → gcc → .exe
```

Each phase is one file in `compiler/src/`, communicating only through data structures:

| File | Role |
|---|---|
| `token.strata` | token kinds + `Token` (shared data) |
| `ast.strata` | AST node types (`TypeNode`/`Expr`/`Stmt`/`Decl`), tag + fat-struct + `new_*` ctors; `Module`/`Program` |
| `lexer.strata` | text → tokens. Significant newlines (Go-style), `..` range, types-first |
| `parser.strata` | tokens → AST. Recursive descent; paren-free control flow via a `no_brace` flag |
| `modules.strata` | the module loader: parses each file, builds the module table |
| `checker.strata` | AST → validated/typed AST. Name resolution + module visibility, types, `var` inference; writes each expression's resolved type onto `Expr.rtype` for codegen |
| `codegen.strata` | typed AST → C. Reads `rtype` for operator overloading (`vec3_add`, `.`/`->`, etc.) |
| `core.strata` | umbrella module: `export import`s every phase = the compiler CORE, **no `main`** (the "library") |
| `dump.strata` | renders tokens/AST to text (front-end utility) |
| `stratac.strata` | front-end #1: the CLI + orchestration (gcc invocation) |
| `console.strata` | front-end #2: a tokens+AST explorer (proves the core is reusable) |

**Key ideas:**
- **The core has no `main`.** `stratac`, `console`, and `libstrata.dll` are thin front-ends
  over the same core — so the same code ships as an exe, a DLL, or (later) an LSP server.
- **Compiles to C** (not C++). Every language feature must lower to plain C; all the
  ergonomic sugar is resolved in the checker and gone by codegen (zero runtime cost).
- **`Expr.rtype`** — the checker annotates every expression with its resolved type. Codegen
  uses this to pick the right C (e.g. `a + b` → `vec3_add(a,b)` when `a` is a vec3, `.` vs
  `->` for field access, `sizeof` for `alloc`, element casts for dynamic arrays).
- **Modules** (`src/modules.strata`): `load_program` parses the root file and each
  imported module *separately* (each file once), tags every declaration with its module,
  and builds the module table. The checker enforces visibility from that table and gives
  colliding private names module-prefixed C names.

---

## 5. The language today (what's implemented)

**Feel:** types-first (C#-like), `var` inference, newline-terminated (`;` optional, as a
separator for several statements on one line; newlines inside `(`/`[` don't count), braces for
blocks, paren-free control flow. Source files: `.strata` (canonical) / `.str` (alias).

**Types:** `int` (=i64), `float` (=f32), `bool`, `char`, `string` (a C `const char*`),
sized ints (`i8..u64`, `f32/f64`), `vec2/3/4`, `mat4`, `quat`, `structs`, `enums`,
pointers (`T*`), and dynamic arrays (`T[dynamic]`).

**Declarations & functions** (types-first):
```strata
struct Entity { vec3 pos; int hp }
enum State { Idle, Walk, Jump }
int add(int a, int b) { return a + b }
var x = 5                 // inference
vec3 v = vec3(1, 0, 0)    // explicit
const PI2 = 6.283
```

**Control flow:** `if`/`else`, `while`, `for x in 0..n`, `for x in array`, and
`switch x { case A: ... case B, C: ... default: ... }` (no fall-through).

**Memory:** `world = arena()`, `world.new(Entity)` (zeroed, region-scoped),
`region name { ... }` (scoped, freed at block end), `alloc(value)` (boxes a value in a
global heap → pointer), `null`. `.` auto-dereferences pointers.

**Math:** first-class operators on `vec`/`mat`/`quat` (`+ - *`, scalar scale), swizzles
(`v.xy`, chained), and builtins `dot`, `cross`, `length`, `normalize`,
`mat4_translate/scale/rotate/perspective/look_at`, `quat_axis_angle/rotate/to_mat4/normalize`.

**Casts & sizes:** `cast<T>(x)` (scalar↔scalar, pointer↔pointer, pointer↔int; anything
else is a checker error) and `sizeof(T)` (an `int`). Both are keywords.

**Strings:** `+` (concat), `.len`, `==`/`!=`, indexing `s[i]` (a `char`), and built-ins
`substr(s, start, len)`, `int_to_str(n)`, `cstr(s)`.

**Files & process:** `read_file(path)` ("" on failure), `write_file(path, data)` (bool),
`args()` (the command line, `string[dynamic]`).

**Dynamic arrays:** literals `[a, b, c]`, `.push(v)`, `.len`, indexing (incl. lvalue
`xs[i].field = ...`), `for x in xs`.

**Prelude (always in scope):** `min`, `max`, `clamp`, `lerp`, `PI`.

**C interop:** `import <raylib.h>` / `import "foo.h"` (emit `#include`), `link "raylib"`
(add `-lraylib`), then call C functions and reference C constants directly.

**Modules:** every file is a module; top-level declarations are **private** unless marked
`export`. `import gfx.Renderer` loads `<project root>/gfx/Renderer.strata` (the root is
the main file's folder) and makes its exports visible to *this* file only (imports aren't
transitive). `export import X` re-exports X (umbrella modules, like `core`). Private names
may repeat across modules (they get module-prefixed C names); exported names must be
unique. Modules may contain only declarations. Tests: `examples/modules2.strata` +
`examples/mods/`, and the error goldens `check/modvis`, `check/modload`, `check/modpriv`.

**Designed but NOT yet implemented:** expression-bodied functions (`f(x) = expr`), default
& named arguments, region-escape safety checking.

---

## 6. Tests

`run.ps1` runs **43 checks**:
1. **bootstrap**: runs `build.ps1` (pinned release → stage1 → stage2 + the fixpoint check).
2. **goldens**: `compiler/tests/<stage>/<name>.expected`, compared **byte-for-byte**
   against `bin/stratac.exe <stage> examples/<name>.strata`, using the self-hosted
   compiler. Stages: `tokens`, `ast`, `check`, `run`, and `emit`. The `emit` goldens pin
   the generated C for every example.
   Add a test by dropping a `.expected` file in the right folder.

---

## 7. The runtime (`compiler/lib/`)

Header-only C the *compiled program* links against (not the compiler). Carries the GPL
**runtime linking exception**, so programs built with Strata are not GPL-covered.

| File | Provides |
|---|---|
| `arena.h` | the arena/region allocator + the global heap for `alloc` (`strata_heap`) |
| `smath.h` | `vec2/3/4`, `mat4`, `quat` and their operations |
| `sstr.h` | string concat/eq/len/`substr`/`int_to_str` (a lazy global string arena) |
| `sio.h` | `read_file`, `write_file`, `args()` |
| `sarr.h` | the type-erased dynamic array (`Array`) |
| `sprelude.h` | prelude helpers (`sp_min`/`max`/`clamp`/`lerp`, `SP_PI`) |
| `crossplatform.h` | platform layer (window, ...), single-header; auto-implemented in Strata programs via `STRATA_PROGRAM`. See its top comment for adding sections |

---

## 8. Examples (`compiler/examples/`)

`hello` (structs + vec3 + arena, the flagship), `run1` (functions/recursion),
`arena` (regions), `vectors`, `matrix` (mat4/quat/swizzles), `arrays` (`T[dynamic]`),
`switch` (enums + switch), `strings`, `interop` (calling libc), `list` (linked list via
alloc+null), `casts` (`cast<T>`/`sizeof`), `prelude`, `modules`+`greetlib` (import). Graphical (raylib): `window`,
`sprite` (arrow-key movement), `balls` (60 bouncing entities in a `Ball[dynamic]`).

---

## 9. Self-hosting — how to work on the compiler

**The compiler source is `compiler/src/*.strata`.** It has been self-hosted since v1.0.0
and moved to `src/` after v1.1.0. The D-- original is in `archive/dminusminus-seed/`.

How it got here: a translator (now archived) ported every D-- module mechanically, and every
failure was fixed in Strata or the translator rather than by hand-editing the port. That
turned up about a dozen real Strata bugs and gaps. The Strata compiler then reproduced
itself byte-for-byte (the fixpoint), and the build now always checks that.

**The bootstrap rule:** stage0 is the release pinned in `compiler/bootstrap.txt`, so the
compiler's *own source* may only use language features that release supports. You can add
any feature *to* the language freely. But before the compiler's own code *uses* one:
release a version containing it, then bump `bootstrap.txt` to that version. That's how Go
and Rust bootstrap.

**Style debt:** the port is a literal translation, so it still reads like D-- (parenthesized
conditions, `;` inside one-line blocks, `0 - 1`). It's all valid Strata; clean it up
opportunistically. The emit goldens and the fixpoint check catch mistakes.

**Strings:** Strata strings are NUL-terminated C strings and `.len` is `strlen` (O(n)). The
lexer keeps literal text as written, so the compiler never needs a NUL in a string. Watch
performance if the compiler starts building very large strings.

---

## 10. Roadmap

- **Tagged unions + pattern matching** — model AST nodes / game events cleanly (pairs with `switch`).
- **M4: SoA / `#soa` arrays** — the data-oriented layout layer.
- **M5: hot-reload runtime** — DLL + host + persistent arena (the Handmade/Jai pattern).
- **M6: Godot GDExtension target.**
- Smaller: a bundled **tcc** for zero-dependency `stratac run`; the designed-but-unbuilt
  sugars (expression-bodied functions, default/named args); qualified names (`shapes.area`)
  to disambiguate imports; module-level constants.

---

## 11. Project / process facts

- **Git:** commit to `main` (solo, linear); every version gets a `vX.Y.Z` tag and a GitHub
  Release; `package.ps1` builds the release `.zip` an installer could pull. `.gitattributes`
  forces LF; `.gitignore` excludes `bin/`, `dist/`, generated `.c`/`.exe`, `.vs/`.
- **Versioning:** Semantic Versioning (1.0.0 = self-hosting). The `stratac_version()` string
  (in `src/stratac.strata`) is bumped in the same commit as the tag. Build each
  release zip from a worktree of its tag so the binaries match the version.
- **License:** GPL-3.0 on the compiler; runtime linking exception on `lib/` (games built
  with Strata are yours); a commercial license ($100, may go dynamic) lifts copyleft for
  private compiler forks (enabled by the CLA). "Strata™" is a common-law trademark (no ®).
- **Detection:** `.gitattributes` maps `.strata` (and the archived `.dmm`/`.hmm`) to C for GitHub highlighting.

---

## 12. Gotchas (things that will bite you)

- **The compiler's own code can only use what the pinned bootstrap release supports** (§9).
- **Imports aren't transitive.** If a file uses something, it must import the module that
  declares it (or an umbrella that `export import`s it). The error message says which.
- **GUI examples block** (a window stays open until closed) — don't run them in an
  automated/headless step; `stratac build` them instead.
- **Line endings:** goldens are LF; the PowerShell test runner normalizes CRLF/LF.
- **Strata strings are NUL-terminated C strings**, unlike D--'s (pointer, length) strings. A
  string can't hold a `\0` byte, and `.len` is `strlen` (O(n)). The compiler avoids the
  problem by keeping literal text as written (never decoding `\0`); keep it that way.

---

## 13. See also
- [`compiler/ARCHITECTURE.md`](compiler/ARCHITECTURE.md) — compiler internals + build order.
- [`website/design/DESIGN.md`](website/design/DESIGN.md) — language design & open decisions.
- [`CHANGELOG.md`](CHANGELOG.md) — what changed in each version.
- [`Strata.md`](Strata.md) — the original founding plan and scope.
