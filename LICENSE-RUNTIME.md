# Strata Runtime Library Exception

**The Strata *compiler* is licensed under GPL-3.0 (see [`LICENSE`](LICENSE)). The Strata
*runtime library* is not — it carries the additional permission below, so that programs
you build with Strata are NOT covered by the GPL.**

This is the same split the GNU Compiler Collection uses (the "GCC Runtime Library
Exception"): copyleft protects the compiler, but never reaches into the software you
compile with it.

## What this means in plain terms

- **If you modify the Strata compiler and distribute it**, that modified compiler must be
  released under GPL-3.0 — you publish your source. *(The "Linux approach": your version of
  Strata stays open.)*
- **If you use Strata to build a game or program**, the result is **entirely yours**. You
  may license and sell it under any terms you like, open or closed. The GPL does not
  reach your game just because the compiler emitted its code or it links the runtime.

## Scope

The exception applies to the files under [`compiler/lib/`](compiler/lib/) — the runtime
the compiler emits calls into and that compiled programs link against (arena allocator,
prelude, math). As this runtime is written, each file will carry the exception notice.

## The exception (additional permission under GPLv3 §7)

> As a special exception, you have permission to propagate a work of Target Code formed by
> combining the Runtime Library with Independent Modules, even though such propagation
> would otherwise violate the terms of GPLv3, provided that all Target Code was generated
> by an Eligible Compilation Process. You may then convey such a combination under terms of
> your choice, consistent with the licensing of the Independent Modules.
>
> "Target Code" is output from any compiler for a target architecture. "Independent
> Modules" means source code that is not derived from the Runtime Library. An "Eligible
> Compilation Process" is one that produces Target Code from code written in Strata (or C
> the compiler emitted) using the Strata compiler, and does not incorporate compiler
> intermediate representations into the Target Code beyond what is required to run it.

*(This exception is modeled on the GCC Runtime Library Exception. Its final wording will be
confirmed — including by legal review — before the runtime library ships.)*
