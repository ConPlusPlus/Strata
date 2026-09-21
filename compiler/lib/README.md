# Strata runtime (`lib/`)

This is the **runtime** the compiler installs beside `strata.exe`, and that your
*compiled programs* link against — **not** code the compiler itself runs. `strata`
passes `-I <install>/lib` to the C backend so generated C can `#include` these headers.

At install time (`install.ps1`) this whole folder is copied to
`%LOCALAPPDATA%\Programs\strata\lib\`, and `strata` finds it next to the exe.

## What lives here (fills in as milestones land)

| File | Provides | Milestone |
|---|---|---|
| `arena.h` | the arena/region allocator (zero-GC memory) — starts from D--'s model | M1 codegen |
| `math.h` (or similar) | `vec2/3/4`, `mat4`, `quat`, operators | M2 |
| `prelude.h` | `print`, input, time — the built-in prelude | M1–M3 |

Nothing is here yet because codegen (which decides the exact header names and contents)
hasn't landed. The first file arrives with the first `strata run`.
