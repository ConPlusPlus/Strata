# archive/

Retired code, kept for history. Nothing here is built or tested.

## `dminusminus-seed/`

The original Strata compiler, written in **D--** (`src/*.dmm`, `*.hmm`), and the
translator (`dmm2strata.py`) that ported it to Strata for self-hosting (v1.0.0).

It was the build's bootstrap seed until the compiler moved to `compiler/src/` (after
v1.1.0). Since then, `compiler/build.ps1` bootstraps from a pinned **release** of `stratac`
instead (`compiler/bootstrap.txt`), and D-- is no longer needed.

**Rebuilding from nothing but D--** is still possible:
1. Check out tag `v1.1.0`. Its `compiler/build.ps1` bootstraps from this seed with the
   D-- compiler `dec`.
2. That produces a v1.1.0 `stratac.exe`; pass it to the current build with
   `compiler\build.ps1 -Bootstrap <that stratac.exe>`.

This folder can be deleted once that path is no longer wanted (it stays in git history).
