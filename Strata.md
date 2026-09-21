# Strata — Project Plan

> **What this file is.** The founding plan for **Strata**, a programming language for
> game development. It captures the vision, the honest scope of each feature, the
> architecture decisions, the competitive landscape, and a staged roadmap — written so
> the project can be picked up cold. Living document: update it as decisions are made.
>
> **Location note.** This file lives in `C:\Strata\`, deliberately **outside** the D--
> repo (`C:\DMinusMinus`), so it is never committed or pushed to D--. Strata gets its
> own repository when it starts.

---

## 1. What Strata is

A statically-typed, compiled language **for game and real-time development**. Positioning:
an **open, available** take on what Jai/Odin promise — the control of C for games, without
C++'s weight or a garbage collector's stutter. One-liner:

> *"C's speed and control for games — arena/region memory so no GC stutter, first-class
> math and data-oriented types, and it drops into any C engine."*

**Relationship to D--:**
- **Two separate languages, both open source.**
- **D--** stays a fun passion project (self-hosted C-compiling language). It could grow
  into more if it ever gains traction, but that's not the goal — it's the joy/learning.
- **Strata is its own language** with its own repo and identity. Its **compiler is written
  in D--** (dogfooding: D-- self-hosts, and now hosts a second language). Strata is the
  game-focused *product*; D-- is the workshop it's built in.
- Keeping them separate keeps each identity clean: D-- doesn't get bent toward games, and
  Strata isn't saddled with D--'s "learning project" framing.

> Tradeoff noted honestly: building Strata as a *new* language (rather than evolving D--)
> means re-doing a frontend (lexer/parser/checker) that D-- already has. The upside is a
> clean, purpose-built design and separation; the cost is more work. Decision: **separate**,
> accepted with eyes open. Strata's backend will (like D--) emit C, so it inherits the C
> toolchain and ecosystem for free.

---

## 2. The four pillars (with honest scope)

The vision has four headline features. Each is broken into *what's realistic* vs *what
oversells*, so the roadmap stays grounded.

### 2.1 Data-Oriented Native Types
*Goal: first-class vectors, matrices, packed arrays; layout that's friendly to modern CPUs.*
- ✅ **Realistic & high-value:** built-in `vec2/vec3/vec4`, `mat4`, quaternions, with
  operators (`+ - *`, dot, cross, swizzles like `v.xy`). Games use these constantly;
  making them *language* types (not library structs) is a real ergonomic win. **Do early.**
- ✅ **SoA / packed arrays as an opt-in type** (à la Odin's `#soa`): the programmer chooses
  struct-of-arrays layout for cache efficiency. Achievable.
- ⚠️ **Oversell to avoid:** "the compiler *automatically* optimizes data layout (AoS→SoA)."
  Automatic layout transformation is research-grade; even Odin/Jai make it **manual**. Ship
  *tools for control*, not magic. SIMD: emit via C compiler vector extensions
  (`__attribute__((vector_size)))`) or lean on `-O2` autovectorization — don't hand-roll
  intrinsics early.

### 2.2 Instantaneous Hot-Reloading
*Goal: reload code + state into a running game without crashing or resetting memory.*
- ✅ **Realistic (as an architecture, not magic):** the "Handmade Hero"/Jai pattern —
  compile game logic to a **DLL/.so**, run a thin **host** that loads it, keep all game
  **state in a host-owned arena** so it survives a hot-swap. On file change: rebuild the
  module, reload it, re-bind function pointers.
- ✅ **Decision: hot-reload ships as a pluggable open-source *library/runtime*, not a
  compiler feature.** You "plug it into Strata" — a host loader + persistent-arena
  discipline. This keeps the compiler simple and the feature optional.
- ⚠️ **Oversell to avoid:** "instantaneous, never crashes, never resets." State survives
  **only while struct layouts don't change**; add a field to a persistent struct and reload
  and you get corruption/crash unless you version + migrate state. Truthful claim:
  *"hot-reload of code with persistent state, following the host/module pattern."*
- **Milestone order: LAST.** It constrains the runtime architecture; build it once the rest
  is solid.

### 2.3 Zero Garbage Collection
*Goal: manual / region-based memory, predictable, stutter-free.*
- ✅ **Essentially free** — this is D--'s existing model (arena + pointers, no GC). Strata
  adopts it directly. The differentiator vs Odin/C: lean into **region/arena memory as the
  headline** — "the approachable middle between C's footguns and Rust's borrow checker."
  A rule anyone can learn: *everything in a region dies together; the checker stops a
  pointer from escaping its region.* (Region-enforced safety is a stretch goal, not v1.)

### 2.4 Seamless C/C++ Interoperability
*Goal: drop into existing engines via C headers, no wrapper code.*
- ✅ **C interop: yes** — compiling to C means Strata calls any C API directly (raylib, SDL,
  sokol, GLFW, Box2D, miniaudio, custom C engines).
- ✅ **Godot: yes** — via **GDExtension**, Godot's **C** API for native extensions. A
  C-compiling language can target it. This is the realistic "plug into a real engine" story.
- ❌ **Unreal: NOT "via C headers."** Unreal is **C++** (name-mangling, templates, classes,
  UCLASS/UPROPERTY reflection via Unreal Header Tool). There is no C API. *Every* non-C++
  language needs a hand-written **C++ shim** to touch Unreal — no language feature removes
  that. Honest pitch: *"any C engine + Godot's GDExtension instantly; C++ engines like
  Unreal need a shim, same as everyone."*
- ⚠️ **Known interop friction (inherited from the D-- experience):** calling C *functions*
  is seamless; consuming C *structs* and `#define` *constants* needs real interop work
  (`extern` structs, constant binding). "Seamless" is the target, not the starting state.

---

## 3. Assets already in hand
- **A math library** (existing) — reuse for the vec/mat foundation instead of starting cold.
- **The D-- compiler** — the implementation language + a proven lexer→parser→checker→codegen
  blueprint to mirror for Strata's own compiler.
- **The C-interop know-how** from D-- (compile-to-C, `extern`, header passthrough, the
  shim pattern for structs/constants).

---

## 4. Competitive landscape & differentiator

| Language | Where it sits | Gap Strata targets |
|---|---|---|
| **Odin** | Open, data-oriented, gamedev, C interop, raylib bindings — **mature** | Strata needs a real edge, not "another Odin" |
| **Jai** (Blow) | Games, arena memory, hot-reload — but **closed beta for years** | Proof of demand for an **open, available** version |
| **C# / Unity** | Huge, but **GC stutter** is a perennial complaint | no-GC + arena = the direct answer |
| **Lua / Luau** | Embedded gameplay scripting; dynamically typed | a **typed, fast** alternative |
| **Rust / Bevy** | Safe, but the borrow checker **fights** mutable, reference-heavy game code | region/arena memory = safety without the fight |

**Strata's intended differentiator:** *region/arena memory as the headline safety+simplicity
story* ("safer than C, simpler than Rust") **+** first-class data-oriented/math types **+**
genuine openness (vs Jai) **+** it's yours end-to-end.

---

## 5. Staged roadmap (prove the vision incrementally)

Each step is a demoable milestone; you have a shareable "make a game in Strata" story after
step 2, long before the hard stuff.

1. **Compiler skeleton in D--** — lexer → parser → checker → C codegen for a minimal core
   (functions, structs, control flow, arena memory). Mirror the D-- architecture.
2. **First-class vector/matrix types** — `vec2/3/4`, `mat4`, operators, swizzles; wire in the
   existing math library. This is what makes it "feel like a game language."
3. **raylib bindings + a real demo** — open a window, move a sprite using Strata's vectors.
   The flagship: "written in Strata, arena memory, no GC, single binary." Share with the
   indie/handmade crowd.
4. **SoA / packed array types** — the opt-in data-oriented layer.
5. **Hot-reload library** — the host + reloadable-module + persistent-arena runtime you plug
   in. The hard capstone.
6. **Godot GDExtension target** — plug Strata into a real, popular engine.

---

## 6. Open decisions
- **Syntax & identity:** how Strata reads vs D-- (it can diverge — cleaner keywords,
  game-oriented sugar). To design.
- **Versioning scheme:** reuse D--'s "element/atomic-number" idea, or something Strata-specific?
- **Memory model depth:** ship plain arena first; decide later whether to add *region-enforced*
  safety (checker stops pointers escaping a region) as the headline safety feature.
- **Stdlib scope:** math (have it), then what — a minimal game prelude (input, time, math,
  arenas)?
- **Repo/licensing:** pick an OSS license (MIT/Apache-2.0/zlib — zlib is popular in gamedev).

---

## 7. Guiding principles
- **Don't boil the ocean.** Two of the four pillars (zero-GC, C interop) are largely solved
  by the compile-to-C + arena approach; two (data layout, hot-reload) are the hard, easily
  over-sold ones — scope them to *manual control* + *architecture*, not magic.
- **Flagship over spec.** A small playable game written in Strata will convince people more
  than any feature list.
- **Honesty in the pitch.** Claim what's true (any C engine + Godot; no-GC; arena). Don't
  claim "drop into Unreal via C headers" or "automatic layout optimization" — both oversell.
- **Keep D-- separate and fun.** Strata is the product; D-- is the workshop and the passion.
