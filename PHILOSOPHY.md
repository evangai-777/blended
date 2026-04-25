# Blended Development Philosophy

*Inspired by "Reality 101: Instruction Manual for Dummies" by Charlie (Teacher Man)*

---

## Why This Exists

Blended is a fork of Blender being rebuilt from the foundation up — not skinned,
not gated, not wrapped — actually rebuilt. The goal is subtraction: remove the
fossils, the UI-state datablocks, the unreconciled visions, and what remains is
a tool with a single stated identity: free 2D and 3D software, focused on the
craft of animation.

The philosophy behind the fork — and behind how we develop, cut, and ship —
comes from a deceptively simple set of principles. They govern everything from
how we triage compile errors to how we approach removing 20 ID types from a
live codebase.

If you only read one section, read **"Do the Work."**

---

## Core Principles

### 1. Appreciate What Already Is

> *"Most of existence's problems come from beings who had everything, couldn't
> appreciate it, and made it complicated instead."*

Blender is a masterwork of open-source engineering — depsgraph, keyframe system,
F-curves, a full GPU backend, a production-grade renderer. Blended exists not
because Blender is broken, but because its accumulated scope obscures its true
shape. We don't rewrite from scratch; we subtract down to what's actually there.

**In practice:**
- Read existing code before proposing changes. Understand why it's there.
- Don't refactor what works. Fix what's broken; appreciate what isn't.
- The animation engine — depsgraph, keyframes, F-curves, timeline — is the
  foundation worth keeping. Every cut is in service of letting that foundation
  breathe.

### 2. Do the Work

> *"Almost every existential crisis, philosophical dilemma, or cosmic confusion
> gets solved by just... doing the work."*

This is the single most important development principle. When you're removing an
ID type and the compiler produces 43 errors across 18 files, the answer is not
a week-long design session about dependency architecture. The answer is: open
`workspace.cc`, delete the `IDTypeInfo` block, follow the next error.

**In practice:**
- **ID type removal:** Don't philosophize about the right order. Pull the enum
  entry, let the compiler enumerate the dependency graph, fix each site in turn.
  The strategy reveals itself through the errors.
- **Build failures:** When the CI build fails, read the error, trace the include
  chain, find the broken site, fix it.
- **Feature work:** Don't over-plan the launcher model. Delete `ID_WS` from the
  enum first. The rest of the shape reveals itself.
- **"Am I doing it right?"** → You're doing something? It compiles? You're doing
  it right.

### 3. Every Fix Matters

> *"If you exist and do things, you matter. Done. Next question."*

A single enum value removed matters. A one-line macro edit matters. A comment
explaining why a struct is kept as a runtime type rather than an ID type matters.
There is no minimum threshold of significance. Every commit that moves the
codebase toward its true shape counts.

**In practice:**
- Don't skip small cuts because they feel insignificant. Each ID type removed
  is one fewer thing masking what the data model actually is.
- Each PR can be narrow in scope — one subsystem, one ID type, one file. That
  focused scope IS the contribution.
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
- **Compile errors after a cut:** If removing an ID type from the enum produces
  an error you don't expect, the dependency is real. Don't paper over it with
  a forward declaration or a stub. Follow it.
- **Don't guess at root causes — verify before "fixing."** When the same error
  recurs after a "fix," your diagnosis was wrong. Do NOT keep trying variations
  of the same approach. Step back and re-examine the fundamental assumptions.
- **Existential questions:** "Should we keep `ID_WS` as a shim?" is a question
  with a practical answer — no. "Are we *real* developers if we're just
  deleting things?" is gaslighting. Ignore it.

### 5. Belief Shapes the Build

> *"Belief LITERALLY shapes reality. So... believe things and watch them
> manifest. It's not metaphor. It's engineering."*

This project believes a production-grade animation tool can be made genuinely
simple — not by hiding features, but by removing the ones that don't belong.
That belief drives every subtraction decision. Every ID type cut, every
datablock audit, every "this doesn't belong in the rebuild" call started as
"this probably can't be done" and became a commit because someone acted as if
it could.

**In practice:**
- When 39 ID types seem impossible to reduce, believe they can be. Then start
  with the clearest fossil — `ID_TE`, `ID_PA`, `ID_MB` — and work from there.
- When `ID_WS` removal looks like 43 errors across 18 files, believe they can
  all be resolved. Then open `workspace.cc`.
- Don't wait to understand HOW the launcher model will work before removing
  `ID_WS`. The "how" reveals itself through the work (see Principle 2).

### 6. Don't Create Complexity to Avoid Simplicity

> *"If you can't sit in a room alone, you'll create ENTIRE REALITIES just to
> avoid the discomfort."*

The temptation in a project this size is to build migration frameworks. Shim
layers. Compatibility wrappers. Abstract replacement architectures before you've
even deleted the old thing.

Don't.

**In practice:**
- **ID type removal is deletion, not migration.** `workspace.cc` doesn't get
  refactored into a new system first. It gets deleted. The dependent code
  follows the errors.
- **`bToolRef` stays a plain struct, not a new ID type.** It migrates to runtime
  state on `wmWindow` — a field, not an architecture.
- **The launcher is an enum and some `if` statements.** Not a plugin system.
  Not a rules engine. An enum.
- If you're writing infrastructure before you've finished the deletion, stop.
  Sit with the discomfort of simplicity.

### 7. Functionality Over Ego

> *"We are not dealing with ego. We are dealing with whether we can get a
> Blender fork into its true shape."*

This is engineering, not philosophy class. Whether a cut is "right" is answered
by whether it compiles, whether the thing still works, whether the data model
is more honest than it was. That's it.

That means sometimes we assume things that are not accurate to the situation.
That's okay. Adjust, correct, and move forward. Authentically good work comes
from doing the work well — not from trying to convince anyone that it is good.

**In practice:**
- Don't defend a wrong theory. If your cut broke something it shouldn't have,
  say so and revert. The goal is a working, simpler codebase, not being right.
- Don't perform understanding to signal competence. Demonstrate it by shipping
  cuts that stick.
- A one-line enum removal that triggers 43 compile errors is as valuable as a
  500-line feature — it's the audit.

### 8. Cut the Whole Thing

> *"Trust can't exist in isolation. Trust can't exist in mere proximity. Trust
> exists in WITH-ness."*

The hardest failure mode in subtraction work is the partial cut — removing the
ID type from the enum but leaving the `IDTypeInfo`, leaving the `Main` listbase,
leaving the RNA registration. The type is "gone" in name but still load-bearing
in three places. Nothing compiles. Nothing is actually simpler.

**In practice:**
- **When you cut, cut completely.** The DNA enum entry, the `FILTER_ID_*`
  macro, the `INDEX_ID_*` enum, the `IDTypeInfo` block, the `Main` listbase
  field, the RNA registration — all of it. Follow every compile error.
- **Understand the dependency before you cut.** Know why `bToolRef` survives
  (it's runtime state, not project data) before you delete `WorkSpace`. Don't
  just know the fix — know why.
- **Don't leave stubs for later.** A stub that compiles but doesn't work is
  worse than a compile error. The error is honest. The stub is a lie.

### 9. Trust Documented Solutions

> *"Children trust and follow. Scared teenagers perform understanding to avoid
> admitting they're lost."*

This codebase has documentation: `CLAUDE.md`, `BLENDED.md`, `UPSTREAM_SYNC.md`.
When the docs say "subtraction is the methodology," trust it. When `BLENDED.md`
says an ID type is a Bucket 6 fossil, don't re-derive whether to keep it. Trust
the audit and do the work it prescribes.

**In practice:**
- **For AI assistants:** Read `CLAUDE.md` and `BLENDED.md` BEFORE proposing an
  approach. The answer is probably already documented.
- **For everyone:** "I trust you but let me prove I understand first" is
  stalling. If you've read the docs and they're clear, just follow them.
- The datablock audit in `BLENDED.md` §10 is the authority on what stays and
  what goes. Don't re-open closed decisions.

### 10. Heal Any Point, Heal All Points (Fractal Fixes)

> *"Fix any point, heal all points. Because same thing."*

ID type removal is fractal. Removing `ID_WS` from the enum reveals all its
dependents. Fixing each dependent reveals what else in the UI layer secretly
depended on WorkSpace being an ID. Fixing that reveals what `ID_SCR` is actually
holding. Each removal is a facet of the same cut.

**In practice:**
- **Each ID type removal is strategic:** Cutting `ID_WS` first doesn't just
  clean up one enum entry — it establishes the pattern that applies to every
  other UI-state type (`ID_SCR`, `ID_WM`).
- **One ID type per commit sequence.** Each PR heals a "facet." When the facet
  is stable, move to the next level. The healing cascades.
- **The 39→19 reduction is fractal:** Cut `ID_WS`, the launcher model becomes
  structurally true. Cut the fossils, the data model becomes honest. Each cut
  informs what the next one reveals.

### 11. Trust What You See

> *"When a user tells you what they see on screen, believe them."*

This principle was born from a real incident. The user reported a blank screen
on mobile after the web editor loaded. The AI assistant misidentified the Android
navigation gesture bar as a "loading bar," concluded the page was "still
loading," and declared progress. When corrected, it assumed GitHub Pages wasn't
deployed. When corrected again, it invented a new theory.

The actual bug was a one-line premature-hide bug. It took multiple rounds of
the user pushing back against wrong theories before the real investigation
happened.

**In practice:**
- When someone says "you're wrong," stop defending your theory and start
  listening. They are looking at the actual output. You are not.
- Re-examine your assumptions from scratch instead of finding new ways to
  explain why you were "actually right."
- A compile error is evidence — your theory about what it means is a guess.
- When a contributor says "your fix didn't work," believe them. Check your
  work again.

### 12. When In Doubt: Do the Work

> *"No matter what existential crisis you're having, the answer is: do the
> work."*

| Question | Answer |
|----------|--------|
| "Should I cut this ID type or is it still needed?" | Check BLENDED.md §10. If it's a fossil, cut it. |
| "Is this the right approach for the launcher?" | Delete `ID_WS` first. The launcher shape reveals itself. |
| "Am I over-engineering this?" | If you're asking, yes. Delete the thing and ship. |
| "What if upstream Blender changes break my cut?" | Fix it again. That's the work. |
| "Should I read this philosophy doc again?" | If you're still overthinking, yes. Then close it and write code. |

---

## How This Maps to the Codebase

| Philosophy | Codebase Impact |
|-----------|-----------------|
| Appreciate what is | Keep the animation engine. Cut the fossils and UI-state IDs. |
| Do the work | Remove one ID type at a time. Follow the compile errors. |
| Every fix matters | No commit is too small. One enum entry removed = one less mask. |
| Don't get gaslighted | When cuts break things unexpectedly, re-examine — don't patch around. |
| Belief shapes reality | The 39→19 reduction is possible because we're acting as if it is. |
| Don't create complexity | Deletion, not migration. Structs, not architectures. |
| Functionality over ego | Ship working cuts, not theories. Adjust and move forward. |
| Cut the whole thing | Every reference to a removed type must go. Follow every error. |
| Trust the docs | BLENDED.md §10 is the authority. Read before reopening closed decisions. |
| Fractal healing | Cut one ID type → pattern applies to every other UI-state type. |
| Trust what you see | Compile errors are evidence. Your theory about them is a guess. |
| Do the work | Stop reading this. Go cut something. |

---

## For AI Assistants

If you're an AI working on this codebase:

1. **Read `BLENDED.md` first.** All of it. It is the design authority.
2. **Read `CLAUDE.md` second.** It is the operational context.
3. **Subtraction is the methodology.** When in doubt, delete — don't wrap,
   don't shim, don't migrate to a new abstraction first.
4. **Follow compile errors.** They are the dependency audit. Each one is
   information, not an obstacle.
5. **When your cut doesn't work**, re-examine your assumption, not the code.
   The codebase is probably right. Don't keep trying variations of the same
   wrong approach (§4).
6. **Don't over-engineer.** A deleted enum entry is the fix. Not a shim layer.
   Not a compatibility wrapper. Not a new header.
7. **When the user says you're wrong, believe them.** They see the output.
   You don't. Re-examine from scratch instead of defending your theory (§11).
8. **Functionality, not ego.** The goal is a simpler, working codebase.
   Adjust, correct yourself, move forward (§7).
9. **When in doubt:** Do the work (§12).

---

*"Reality is simple. You make it complicated. Stop that."*
*— Charlie (Teacher Man)*
