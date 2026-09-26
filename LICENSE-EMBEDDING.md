# Strata Embedding Exception

**Any engine, editor or tool may embed the Strata compiler, whatever its own license.**

The Strata compiler is licensed under GPL-3.0 (see [`LICENSE`](LICENSE)). On its own, the
GPL would make any program that links the compiler a combined work covered by the GPL,
and that would rule out embedding Strata in closed-source engines. This exception
removes that obstacle for the compiler **library** (`libstrata`) and its **embedding
API**.

## What this means in plain terms

- **You may link or load `libstrata`** (the Strata compiler as a library) in your engine,
  editor, tool or game, and ship it, **under any license you like**, open or closed,
  free or paid. Your code stays yours.
- **You may use the bindings** in [`compiler/api/`](compiler/api/) (`strata.h`,
  `strata.hpp`, `Strata.cs`) in your program under the same terms.
- **If you modify Strata itself** (the compiler or `libstrata`) and distribute your
  modified version, *that modified Strata* is still covered by the GPL: you publish its
  source. To keep modifications to Strata private, see the
  [commercial license](COMMERCIAL.md).
- Programs **built** with Strata are covered separately, by the runtime exception
  ([`LICENSE-RUNTIME.md`](LICENSE-RUNTIME.md)): they are entirely yours too.

## Scope

This exception applies to the Strata compiler's source code (`compiler/src/`) as built
into `libstrata`, and to the embedding API in `compiler/api/`.

## The exception (additional permission under GPLv3 §7)

> Linking this library statically or dynamically with other modules is making a combined
> work based on this library. Thus, the terms and conditions of the GNU General Public
> License cover the whole combination.
>
> As a special exception, the copyright holders of this library give you permission to
> link this library with independent modules to produce an executable, regardless of the
> license terms of these independent modules, and to copy and distribute the resulting
> executable under terms of your choice, provided that you also meet, for each linked
> independent module, the terms and conditions of the license of that module. An
> independent module is a module which is not derived from or based on this library. If
> you modify this library, you may extend this exception to your version of the library,
> but you are not obligated to do so. If you do not wish to do so, delete this exception
> statement from your version.

*(This is the GNU Classpath exception, applied to Strata. Like the runtime exception, its
final wording will be confirmed, including by legal review, before it is relied on.)*
