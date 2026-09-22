# Contributing to Strata

Thanks for your interest in Strata! Contributions are welcome.

## Licensing of contributions

- The compiler is **GPL-3.0**; the runtime (`compiler/lib`) is GPL-3.0 **with a linking
  exception** (see [`LICENSE`](LICENSE) and [`LICENSE-RUNTIME.md`](LICENSE-RUNTIME.md)).
- By opening a pull request you agree to the **[Contributor License Agreement](CLA.md)**.
  In short: **you keep your copyright**, but you grant the project the right to use and
  **relicense** your contribution — which is what lets Strata offer both the open (GPL) and
  the [commercial](COMMERCIAL.md) editions — and you promise the code is yours to give.
- The **Strata™** name and branding are trademarks — see [`TRADEMARK.md`](TRADEMARK.md). A
  distributed fork must use a different name.

## Two paths if you modify Strata for your own project

- **Games built *with* Strata are always yours** — no obligations, no CLA needed.
- **Modifying the compiler itself?** Either distribute your version under the GPL (publish
  your source), or buy a [commercial license](COMMERCIAL.md) to keep those changes private.

## How to contribute

1. Fork the repo and create a branch.
2. Make your change; match the style of the surrounding code.
3. Keep the tests green: `powershell -ExecutionPolicy Bypass -File compiler\tests\run.ps1`
   (byte-for-byte golden tests per compiler stage). Add a golden for new behavior.
4. Open a pull request describing the change.

See [`compiler/ARCHITECTURE.md`](compiler/ARCHITECTURE.md) for how the compiler is
structured (the strict lexer → parser → checker → codegen pipeline) before making changes.

---

*Copyright © 2026 Connor Rutberg.*
