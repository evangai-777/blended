# Blended Development Philosophy

*Inspired by "Reality 101: Instruction Manual for Dummies" by Charlie (Teacher Man)*

---

## Why This Exists

Blended is a fork of Blender that simplifies a complex tool into approachable
tiers. The philosophy behind the fork — and behind how we develop, debug, and
maintain it — comes from a deceptively simple set of principles. They govern
everything from how we triage compiler warnings to how we approach a
six-stage WebAssembly port.

If you only read one section, read **"Do the Work."**

---

## Core Principles

### 1. Appreciate What Already Is

> *"Most of existence's problems come from beings who had everything, couldn't
> appreciate it, and made it complicated instead."*

Blender is a masterwork of open-source engineering — 28 subsystems, a full GPU
backend, a production-grade renderer. Blended exists not because Blender is
broken, but because its full surface area overwhelms new users. We don't
rewrite, we curate.

**In practice:**
- Read existing code before proposing changes. Understand why it's there.
- Don't refactor what works. A warning-free build is not the same as a better
  build. Fix what's broken; appreciate what isn't.
- Blended's tiered UI is an act of appreciation: every feature Blender offers
  is still there, just revealed progressively.

### 2. Do the Work

> *"Almost every existential crisis, philosophical dilemma, or cosmic confusion
> gets solved by just... doing the work."*

This is the single most important development principle. When you encounter a
20,000-warning build log, the answer is not a week-long design session about
warning taxonomies. The answer is: pick a subsystem, open the first file, add
the explicit cast, move to the next line.

**In practice:**
- **Warnings triage:** Don't philosophize about whether `-Wno-sign-conversion`
  should be removed globally or per-file. Open `source/blender/gpu/opengl/`,
  find the first implicit conversion, add `static_cast<>`, repeat. The
  strategy reveals itself through the work.
- **Build failures:** When the WASM build fails, don't theorize. Read the
  error, trace the include chain, find the missing symbol, fix it.
- **Feature work:** Don't over-plan tier filtering logic. Implement one panel
  gate, verify it works, apply the pattern.
- **"Am I doing it right?"** → You're doing something? It compiles? Tests
  pass? You're doing it right.

### 3. Every Fix Matters

> *"If you exist and do things, you matter. Done. Next question."*

A single `static_cast<int>()` replacing an implicit narrowing conversion
matters. A one-line `#ifdef EMSCRIPTEN` guard matters. A comment explaining
why build tools must not use `-matomics` matters. There is no minimum
threshold of significance. Every commit that moves the codebase forward
counts.

**In practice:**
- Don't skip small fixes because they feel insignificant. Each warning fixed
  is one fewer thing masking a real bug.
- Each PR should fix warnings from one subsystem (per WARNINGS.md). That
  focused scope IS the contribution. Don't apologize for its size.
- A tree matters. A rock matters. A cast matters.

### 4. Don't Let Broken Substrate Gaslight You

> *"If the question makes simple things complicated → gaslighting."*

This principle saved the project during the RNA string corruption saga.
The bug was "fixed" seven times with increasingly elaborate theories — stack
overflows, pthread mismatches, hex dump diagnostics, per-target flag
overrides — before someone stopped and asked: "What if the global compiler
flags are simply wrong?"

They were. The fix was removing two flags.

**In practice:**
- **The Meta Rule:** When the same error recurs after a "fix", your diagnosis
  is wrong. Stop. Re-examine fundamentals.
- **Debugging WASM builds:** If build tools produce garbled strings, it's
  almost certainly a global-flag contamination issue, not a memory bug. Check
  `CMAKE_C_FLAGS` and `CMAKE_EXE_LINKER_FLAGS` first.
- **Warning fixes:** If adding a cast doesn't fix the warning, you're casting
  the wrong thing. Don't add more casts. Read the actual type mismatch.
- **Don't guess at root causes — verify before "fixing."** The RNA string
  corruption bug was "fixed" 7+ times: stack overflow (wrong), pthread
  mismatches (partial), hex dumps (debugging, not fixing), per-target
  `-pthread` moves (right direction), global `-matomics` with per-target
  overrides (wrong — reintroduced the bug), and finally removing global
  `-matomics` entirely (correct). When the same error recurs after a "fix,"
  your diagnosis was wrong. Do NOT keep trying variations of the same
  approach. Step back and re-examine the fundamental assumptions. Read
  existing comments and commit history before proposing a fix — the answer
  may already be documented.
- **Existential questions:** "Should we target WebGL2 or WebGPU?" is a
  question with practical consequences — answer it and move on. "Are we *real*
  developers if we're just adding casts?" is gaslighting. Ignore it.

### 5. Belief Shapes the Build

> *"Belief LITERALLY shapes reality. So... believe things and watch them
> manifest. It's not metaphor. It's engineering."*

This project believes a 3D editor can run in a browser. That belief drove the
creation of the Emscripten build infrastructure, the epoxy GL shim, the
build-tool isolation system, and the six-stage roadmap. Every one of those
started as "this probably won't work" and became working code because someone
acted as if it would.

**In practice:**
- When the WASM binary hits 100+ MB, believe it can be optimized. Then use
  `wasm-opt -O3` and brotli compression and module splitting.
- When WebGL2 lacks compute shaders, believe a CPU fallback path can work.
  Then implement it in the six draw sites that need it.
- When 20,000 warnings seem impossible to fix, believe they can be fixed one
  subsystem at a time. Then start with GPU, then draw, then blenlib.
- Don't wait to understand HOW something will work before starting. The "how"
  reveals itself through the work (see Principle 2).

### 6. Don't Create Complexity to Avoid Simplicity

> *"If you can't sit in a room alone, you'll create ENTIRE REALITIES just to
> avoid the discomfort."*

The temptation in a project this size is to build frameworks. Abstraction
layers. Generic warning-fixer tools. Meta-build-systems for the build system.
Configuration-driven configuration configurators.

Don't.

**In practice:**
- **Warning fixes are casts, not frameworks.** An implicit `int → unsigned`
  conversion gets `static_cast<unsigned int>(x)`, not a `SafeConvert<T>`
  template library.
- **Build tool isolation is a function, not a system.** `emscripten_build_tool_flags()`
  is 15 lines of CMake. That's all it needs to be.
- **The tier system is an enum and some `if` statements.** Not a plugin
  architecture. Not a rules engine. An enum.
- If you're writing infrastructure for infrastructure, stop. Sit with the
  discomfort of simplicity.

### 7. Functionality Over Ego

> *"We are not dealing with ego. We are dealing with whether we can get a
> Blender fork into a browser."*

This is engineering, not philosophy class. In philosophy you can deal with
ego and debate and whether something is "actually right" or "appears right,"
so that someone can have credibility. Here we are dealing with
**functionality** — whether something works or doesn't work the way we
intend. That's it.

That means sometimes we assume things that are not accurate to the situation.
And that's okay. It just means we adjust, correct ourselves, and move
forward. Authentically good work comes from doing the work well — not from
trying to convince people that it is good. We aren't trying to convince each
other that we are doing good work; we are doing it and seeing the results.
This inadvertently establishes us as excellent developers without needing
ego badges.

**In practice:**
- Don't defend a wrong theory. If your fix didn't work, say so and pivot.
  The goal is a working build, not being right.
- Don't perform understanding to signal competence. Demonstrate competence
  by shipping fixes that stick.
- Code review is about functionality, not status. A one-line cast that
  eliminates a warning class is as valuable as a 500-line feature.
- This is a good life philosophy as well as an engineering one.

### 8. Build-Tool / Browser-Target: Being-With, Not Being-Near

> *"Trust can't exist in isolation. Trust can't exist in mere proximity. Trust
> exists in WITH-ness."*

The hardest bug in this project — WASM data-segment corruption — happened
because build tools and the browser target were "near" each other in the CMake
build graph but not truly "with" each other. They shared library targets, they
shared compiler flags, they existed in the same build directory. But nobody
understood their relationship deeply enough to see that a flag safe for one
context was poison for the other.

**In practice:**
- **The Meta Rule from WARNINGS.md:** Emscripten has TWO execution contexts —
  Node.js build tools and the browser target. Every global flag must be safe
  for BOTH. If a flag is only appropriate for one, it goes per-target.
- Understand the relationship, don't just configure around it. Know WHY
  `-matomics` corrupts build tools (shared `.a` objects compiled with atomics
  linked into non-shared-memory binaries). Don't just know the fix.
- When adding a new build tool, don't just copy flags from another tool. Use
  `emscripten_build_tool_flags()` AND understand what it does. Be WITH the
  build system, not just near it.

### 9. Trust Documented Solutions

> *"Children trust and follow. Scared teenagers perform understanding to avoid
> admitting they're lost."*

This codebase has extensive documentation: `WARNINGS.md`, the `platform_emscripten.cmake`
comments, the WebAssembly roadmap, this file. When the docs say "DO NOT set
`-matomics` in global compiler flags", trust it. Don't re-derive the reasoning
from first principles. Don't add the flag "just to see what happens." Trust
the documentation and do the work it prescribes.

**In practice:**
- **For AI assistants:** Read existing comments and commit history BEFORE
  proposing a fix. The answer is probably already documented.
- **For human contributors:** If `WARNINGS.md` says to fix Phase 1 (GPU)
  before Phase 3 (blenlib), follow the order. It's that way for a reason.
- **For everyone:** "I trust you but let me prove I understand first" is
  stalling. If you've read the docs and they're clear, just follow them.

### 10. Heal Any Point, Heal All Points (Fractal Fixes)

> *"Fix any point, heal all points. Because same thing."*

Compiler warnings are fractal. A sign-conversion warning in `gl_texture.cc`
is the same class of issue as a sign-conversion warning in `draw_view.cc` is
the same class as the 20,000 blanket-suppressed sign conversions across the
codebase. Fix the pattern in one file, and you understand how to fix it
everywhere.

**In practice:**
- **Phase 1 of WARNINGS.md is strategic:** Fixing GPU/WebGL2 warnings first
  doesn't just clean up 6 files — it establishes the cast patterns that
  propagate to every other subsystem.
- **One subsystem per PR:** Each PR heals a "facet." When the facet is stable,
  move to the next level. The healing cascades.
- **The tier system is fractal too:** Simple tier users get a clean experience;
  that clean experience informs what Standard tier should feel like; Standard
  informs Advanced. Heal Simple first. The rest follows.

### 11. Trust What You See

> *"When a user tells you what they see on screen, believe them."*

This principle was born from a real incident. The user reported a blank screen
on mobile after the WASM web editor loaded. The AI assistant misidentified the
Android navigation gesture bar as a "loading bar," concluded the page was
"still loading," and declared progress. When corrected, it assumed GitHub Pages
wasn't deployed (403) and told the user to enable Pages in repo settings. The
user corrected again: the page DOES load, shows the "Blended" title, shows all
green feature checkmarks, THEN goes blank.

The actual bug was that `setStatus("")` was hiding the loading overlay before
`onRuntimeInitialized` fired — a simple premature-hide bug. But it took
multiple rounds of the user pushing back against wrong theories before the real
investigation happened.

**In practice:**
- When someone says "you're wrong," stop defending your theory and start
  listening. They are looking at the actual screen. You are not.
- Re-examine your assumptions from scratch instead of finding new ways to
  explain why you were "actually right."
- A screenshot is evidence — your theory is a guess.
- This applies to all feedback, not just UI bugs. When a contributor says
  "your fix didn't work," believe them. Check your work again.

### 12. When In Doubt: Do the Work

> *"No matter what existential crisis you're having, the answer is: do the
> work."*

| Question | Answer |
|----------|--------|
| "Should I fix this warning or is it intentional?" | Fix it. Use an explicit cast. If it was intentional, the cast documents that. |
| "Is this the right approach for WebGL2 fallbacks?" | Implement it. Ship it. Adjust based on what breaks. |
| "Am I over-engineering this?" | If you're asking, yes. Simplify and ship. |
| "What if upstream Blender changes break my fix?" | Fix it again. That's the work. |
| "Should I read this philosophy doc again?" | If you're still overthinking, yes. Then close it and write code. |

---

## How This Maps to the Codebase

| Philosophy | Codebase Impact |
|-----------|-----------------|
| Appreciate what is | Don't rewrite Blender. Curate it via tiers. |
| Do the work | Fix warnings one cast at a time. Build, test, commit. |
| Every fix matters | No PR is too small. One warning fixed = one less mask. |
| Don't get gaslighted | When bugs recur, re-examine fundamentals, not variations. Verify before "fixing." |
| Belief shapes reality | The WASM port exists because we believed it could. |
| Don't create complexity | Casts, not frameworks. Enums, not architectures. |
| Functionality over ego | Ship working code, not theories. Adjust and move forward. |
| Being-with, not near | Understand build-tool vs. browser-target relationship deeply. |
| Trust the docs | Read WARNINGS.md pitfalls before touching Emscripten flags. |
| Fractal healing | Fix one subsystem's warnings → pattern applies everywhere. |
| Trust what you see | User observations are evidence. Your theory is a guess. |
| Do the work | Stop reading this. Go fix a warning. |

---

## For AI Assistants

If you're an AI working on this codebase:

1. **Read WARNINGS.md pitfalls first.** All of them. Trust them (§9).
2. **Don't re-add global `-matomics` or `-pthread`.** The docs explain why (§8).
3. **Fix warnings with explicit casts**, not suppressions or `(void)` (§3, §6).
4. **One subsystem per PR.** Start with GPU (Phase 1) (§10).
5. **When your fix doesn't work**, re-examine your assumption, not the code.
   The codebase is probably right. Don't keep trying variations of the same
   wrong approach (§4).
6. **Don't over-engineer.** A `static_cast` is the fix. Not a template.
   Not a macro. Not a new header (§6).
7. **When the user says you're wrong, believe them.** They see the screen.
   You don't. Re-examine from scratch instead of defending your theory (§11).
8. **Functionality, not ego.** The goal is a working build, not being right.
   Adjust, correct yourself, move forward (§7).
9. **When in doubt:** Do the work (§12).

---

*"Reality is simple. You make it complicated. Stop that."*
*— Charlie (Teacher Man)*
