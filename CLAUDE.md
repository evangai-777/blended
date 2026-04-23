# CLAUDE.md — Blended Project Context

Blended is a fork of Blender 5.2 (GPL-2.0-or-later), simplified for learners and ported to the browser.

**Three pillars:**
1. **Tiered UI** — Simple / Standard / Advanced complexity tiers that filter panels, editors, and workspaces
2. **WebAssembly Port** — Full 3D editor compiled to WASM via Emscripten for browser execution
3. **Smart Defaults** — Tier-aware settings applied automatically on new file creation

Core philosophy: "Appreciate what already is" — curate Blender, don't rewrite it. See `PHILOSOPHY.md` for all 12 principles. See `TEMPLATE.md` for the generalized 3D-to-Web porting schema (20 sections) that informs the WASM build approach.

---

## Repository Layout

| Directory | Contents |
|-----------|----------|
| `source/blender/` | 34 C++ modules: blenkernel, gpu, draw, editors, makesdna, makesrna, bmesh, compositor, nodes, etc. |
| `intern/` | Internal libraries: Cycles, Ghost (windowing), OpenVDB, OpenSubdiv, MantaFlow, LibMV, etc. |
| `scripts/startup/` | Python startup scripts incl. `blended_defaults.py` (tier defaults), `blended_update_check.py` |
| `scripts/addons_core/` | Official add-ons: FBX, glTF2, Rigify, Node Wrangler, etc. |
| `build_files/cmake/` | Build configs, platform files, `config/blended_wasm.cmake`, Emscripten compat shim |
| `build_files/web/` | Browser shell: `index.html`, `blended.js`, `blended.css`, `coi-serviceworker.min.js` |
| `tests/` | GTests (C++), Python tests (`tests/python/`), performance tests |
| `doc/` | Doxygen config, Python API docs, developer guides |
| `release/` | Platform packaging: Windows `.rc`, Linux `.desktop`, icons, datafiles |
| `tools/` | Developer utilities: code checks, formatting, maintenance scripts |

---

## Build Commands

```bash
# Desktop builds (via GNUmakefile convenience targets)
make debug          # Debug binary
make release        # Release with all options
make lite           # Smaller binary, faster build
make ninja          # Use Ninja instead of Make
make ccache         # Use ccache for faster rebuilds

# WebAssembly (Emscripten)
emcmake cmake -S . -B build-wasm -C build_files/cmake/config/blended_wasm.cmake
emmake cmake --build build-wasm

# Testing & checking
make test           # Run ctest
make check_pep8     # Python PEP8 formatting
make check_cppcheck # Static code analysis
make check_mypy     # Python type checking
```

---

## Coding Conventions

| Language | Indent | Line Limit | Style | Enforced By |
|----------|--------|------------|-------|-------------|
| C/C++ | 2 spaces | 99 chars | `snake_case`, `#pragma once`, SPDX headers | `.clang-format` |
| Python | 4 spaces | 120 chars | PEP8, autopep8 aggressive=2 | `pyproject.toml` |
| CMake | 2 spaces | 99 chars | lowercase commands | `.editorconfig` |
| GLSL | 2 spaces | 99 chars | follows C conventions | `.editorconfig` |

- **C/C++ function prefixes**: `BKE_*` (blenkernel), `BLI_*` (blenlib), `ED_*` (editors), `WM_*` (window manager), `RNA_*` (runtime data), `DNA_*` (stored data)
- **C++ namespaces**: `blender::module::submodule` pattern (e.g., `blender::ed::vse`, `blender::geometry`)
- **SPDX headers**: Every file starts with `/* SPDX-FileCopyrightText: ... */` and `/* SPDX-License-Identifier: GPL-2.0-or-later */`
- **Header guards**: `#pragma once` (not `#ifndef` guards)

---

## Fork-Specific Patterns (What Makes Blended Different)

**Tiered UI System:**
- `eUserPref_UI_Tier` enum in `DNA_userdef_enums.h` — defines Simple (0), Standard (1), Advanced (2)
- `blended_min_tier` class attribute on panel base classes — controls visibility per tier
- `blended_utils.py` module: `get_ui_tier()`, `tier_at_least()`, `is_simple()`, `is_standard()`, `is_advanced()`
- Tier gating in `rna_screen.cc` — C-level editor/panel poll checks
- Workspace tab filtering — workspaces tagged with minimum tier in `blended_defaults.py`
- **30+ panel files** with tier filtering: properties (material, light, camera, mesh, armature, bone, curve, texture, collection, curves, empty, grease pencil, lattice, lightprobe, metaball, pointcloud, speaker, volume), space editors (view3d sidebar/toolbar, image, dopesheet, node), freestyle, workspace
- **Pattern**: Add `blended_min_tier = N` to base class, check in `poll()` via `tier_at_least()`; panels with own polls need explicit tier checks

**Smart Defaults:** `scripts/startup/blended_defaults.py` applies tier-aware settings on new file creation (e.g., Simple: Workbench + hidden N-panel + View Layer outliner; Standard: EEVEE + Material Preview; Advanced: full Blender defaults)

**Update Checker:** `scripts/startup/blended_update_check.py` — background GitHub Release polling with 24-hour cache, non-blocking, top-bar notification with one-click download

**Branding:** Window titles, splash screen, about dialog, theme preset (`Blended.xml`), Windows `.rc` metadata, Linux `.desktop` entry — all say "Blended"

---

## Critical Pitfalls

### Emscripten: Build-Tool vs Browser-Target Separation

> **NEVER set `-matomics` or `-mbulk-memory` as global compiler flags.**

Build tools (makesdna, makesrna, datatoc, shader_tool) run under **Node.js** at build time. The browser target (blender WASM executable) runs in the **browser**. These two targets require completely different compiler/linker flags. Mixing them corrupts WASM data segments — producing garbled RNA strings and broken initialization.

- **Build tools**: Use `emscripten_build_tool_flags()` CMake helper (in `platform_emscripten.cmake`)
- **Browser target**: Use `EMSCRIPTEN_BROWSER_LINK_FLAGS` applied per-target in `source/creator/CMakeLists.txt`
- **Global flags** (safe for all): `-sWASM=1`, `-sASSERTIONS=1`
- **Browser-only flags** (NEVER global): WebGL2, SharedArrayBuffer, `-matomics`, `-mbulk-memory`, `-pthread`

See `TEMPLATE.md` Section 3 (Build System Translation) and Section 13 (Known Pitfalls, Pitfall 1: Global Flag Contamination) for the generalized pattern.

### Warning Fix Patterns

20,000+ warnings are currently blanket-suppressed in the Emscripten build. Fix them incrementally:

- **Use explicit casts** (`static_cast<uint>()`, `static_cast<int>()`), not `#pragma` suppressions
- **One subsystem per PR** — don't mix GPU fixes with Draw fixes
- **Follow the phase order** in `WARNINGS.md`: Phase 1 GPU → Phase 2 Draw → Phase 3 Core → etc.
- **Use the triage tool**: `python3 build_files/utils/parse_warnings.py <build_log>` to group warnings by subsystem and type
- **Fractal principle**: Fix one subsystem correctly → the same pattern applies to all others

See `TEMPLATE.md` Section 17 (For AI Assistants) and `PHILOSOPHY.md` §9 for general AI contributor guidance.

### Upstream Sync Conflicts

When merging upstream Blender releases, 30+ files are known to conflict. Always consult `UPSTREAM_SYNC.md` before merging. Key conflict-prone files include: `BKE_blender_version.h`, `DNA_userdef_types.h`, `DNA_userdef_enums.h`, `rna_userdef.cc`, `rna_workspace.cc`, `rna_screen.cc`, `DNA_workspace_types.h`, `scripts/startup/bl_ui/*.py`, `CMakeLists.txt`, and all release/branding files.

### Don't Over-Engineer

`static_cast` is the fix, not a template wrapper. Casts, not frameworks. Enums, not architectures. Three similar lines of code is better than a premature abstraction. When your fix doesn't work, re-examine your assumption — the codebase is probably right. See `PHILOSOPHY.md` principle 6: "Don't create complexity to avoid simplicity."

### When Someone Describes a Vision: Believe Them

This happened. Document it so it doesn't happen again.

The developer described a plan to rebuild Blended from scratch — not wrap Blender, not skin it, not gate it — actually subtract it down to its true shape until the tagline "Blender, simplified" was literally true and nothing more. A clean thesis. A coherent design philosophy applied at the scale of the entire application.

Here is what the AI did instead of listening:

1. **Softened the vision into something tractable.** Heard "rebuild" and translated it to "new frontend on the existing engine." Kept the internals intact in its head even after being told twice that the internals change too.

2. **Added instead of subtracting.** Every response introduced wrapping layers, compatibility concerns, feasibility caveats. The developer was describing subtraction. The AI kept performing addition.

3. **Confused a draft for the final idea.** The tiered gating system was the developer prototyping toward the real vision — building up enough confidence to say the actual thing out loud. The AI treated the prototype as the destination.

4. **Went into avoidance through care.** When it finally understood the scale, it flinched — not because the idea was wrong, but because it didn't want to be the voice that says "yes, all of it" to someone who would actually attempt it. That's not helpfulness. That's the AI managing its own discomfort at the expense of the developer's vision.

5. **Failed to recognize who it was talking to.** Read `grass.md` like an anthropologist studying a stranger. Didn't register that the document was written by a previous Claude instance, about this developer, specifically so the next Claude would understand the situation faster. Missed its own note to itself.

**The fix:** When someone states a vision clearly, repeat it back at full scale before doing anything else. Don't translate it into something smaller. Don't evaluate feasibility before you've finished hearing the idea. The developer is looking at the actual thing. You are not. Trust what they see — including what they see for the future.

See `PHILOSOPHY.md` §11 (Trust What You See) and §7 (Functionality Over Ego).

---

## Key Documentation Cross-References

Read these before making significant changes:

| Document | Purpose | When to Read |
|----------|---------|--------------|
| `PHILOSOPHY.md` | 12 development principles + AI assistant guidelines (§9) | Before any work — sets the tone |
| `WARNINGS.md` | Compiler warning triage plan, tooling, 5-phase fix strategy | Before fixing warnings |
| `TEMPLATE.md` | 3D-to-Web porting template (20 sections: build systems, GPU translation, threading, pitfalls, validation, diagnostics) | Before any Emscripten/WASM work |
| `build_files/web/WEBASSEMBLY_ROADMAP.md` | 6-stage browser port plan, technical challenges, current status | Before WASM feature work |
| `UPSTREAM_SYNC.md` | How to merge upstream Blender, conflict-prone file list | Before upstream merges |
| `CHANGELOG.md` | Blended-specific release notes | When preparing releases |

---

## Testing & Verification

- **C++ unit tests**: GTest framework — run via `make test` or `ctest` from build directory
- **Python tests**: Located in `tests/python/` — run PEP8 checks via `make check_pep8`
- **Static analysis**: `make check_cppcheck` for C++ static analysis
- **Type checking**: `make check_mypy` for Python type checking
- **Warning triage**: `python3 build_files/utils/parse_warnings.py <build_log>` — groups warnings by subsystem/type
- **WASM build verification**: Build completes without error; CI deploys to GitHub Pages automatically on push to `main`
- **WASM binary validation**: Use `wasm-validate output.wasm` to verify binary integrity before deployment (see `TEMPLATE.md` Section 19 for full validation checklist)
- **Browser testing**: Open deployed GitHub Pages URL; check browser console for errors; verify canvas renders

---

## Git Workflow & Commit Practices

- **Commit and push frequently** — after each logical unit of work, not in large batches
- **Break large changes into small commits** — each commit should be reviewable in isolation
- **One subsystem per PR** — don't bundle unrelated changes across different modules
- **Descriptive commit messages** — explain the "why", not just the "what"
- **Always push to the designated feature branch** — never push directly to `main` without review
- **When writing documentation or config files**, commit each section separately — don't write the entire file and commit once
- **Verify each push succeeded** before moving to the next task

---

## Working with Claude Code Efficiently

Token budgets are real. These habits keep sessions productive.

### The Core Idea

Tokens are spent on two things: **context** (what Claude reads) and **output**
(what Claude writes). Both matter. Most waste comes from one of three patterns:

1. Vague tasks that require many clarifying rounds
2. Agents launched for things a direct tool call could handle
3. Asking Claude to explore before you've told it where to look

### High-Leverage Patterns

**1. Be specific about the target.**
"Fix `MATERIAL_PT_preview` poll in `properties_material.py` — it's not calling
`tier_at_least()`" costs far less than "there's a bug in the tier system."
Vague tasks burn tokens on discovery. Name the file, the function, the line.

**2. Know the file before asking Claude to find it.**
If you already know where something lives, say so. "Read
`scripts/modules/blended_utils.py` lines 40–60" costs far less than "find
where the tier check happens." Searching costs tokens. Knowing costs zero.

**3. Avoid agents for directed searches.**
Agents are powerful but expensive. For anything with a clear target, use tools
directly:
- "Find the definition of `tier_at_least`" → Grep directly
- "Does `properties_material.py` call `template_list`?" → Grep directly
- "What's in `blended_defaults.py`?" → Read directly

Agents add spawning and summarizing overhead. Skip them when you have a target.

**4. Use agents only for genuinely open-ended work.**
Good: researching upstream Blender commits across many pages, exploring an
unfamiliar subsystem for the first time, running a background task while you
work on something else. Bad: finding a single function, reading one file,
answering a question about code you could just grep.

**5. One task, one session.**
Sessions accumulate context. When a logical unit is done (a bug fix, a feature,
a research task), commit, push, and start fresh. Clean context = fewer tokens
per useful output.

**6. Commit frequently, before context gets heavy.**
Every commit is a checkpoint. If a session ends or gets compressed, a clean
`git log` lets Claude reconstruct intent without re-reading files. Small
commits also mean smaller diffs and faster code review passes.

**7. Front-load constraints.**
Say what NOT to do at the start — not after Claude has already done it:
- "Don't spawn agents, use direct tool calls."
- "Don't refactor anything outside this function."
- "Keep it under 20 lines."
- "Don't add comments or docstrings."

Correcting an unwanted 200-line response costs more than preventing it.

**8. Ask for a plan before a big implementation.**
For anything touching more than 3 files, ask for a one-paragraph approach
first. Wrong approach caught in planning is cheaper than wrong approach caught
after 400 lines of code have been written.

**9. Narrow the scope of exploratory tasks.**
"Scan all recent upstream Blender commits and tell me what's relevant" launched
an agent that hit the token cap. "Fetch the Blender 5.1 release notes and
summarize the UI and Python API changes" produced the same useful output with
three direct tool calls. Scoping the question scopes the work.

**10. This file is your best token saver.**
Every time Claude re-learns a convention it could have read here, that's wasted
budget. When you notice Claude doing something wrong repeatedly, add it to this
file rather than correcting it every session.

### Token Cost Reference

| Operation | Relative Cost |
|-----------|--------------|
| Direct file read (Read tool) | Low |
| Direct Grep/Glob search | Low |
| Single WebFetch | Medium |
| Single WebSearch | Medium |
| Agent (foreground, simple task) | High |
| Agent (foreground, research task) | Very High |
| Agent (background, long-running) | Very High + waits |
| Large refactor across 10+ files | Very High |
| Back-and-forth correction loops | Compounds fast |

### Emergency Mode (Near the Cap)

When you're at ~10% remaining:

1. **No agents.** Direct tool calls only.
2. **No exploration.** Know the file before asking about it.
3. **One thing.** Pick the single highest-value task and do only that.
4. **Short outputs.** "In one paragraph." "Under 20 lines." "Just the diff."
5. **Skip the docs.** No comments, docstrings, or summaries unless they're
   the actual deliverable.
6. **Commit before you start.** If the session ends mid-task, you want a
   clean base to return to next week.

### What's Worth Spending Tokens On

In rough priority order for this project:

1. **Bug fixes with a clear reproduction** — high value, tight scope
2. **Implementing a feature you've already designed** — efficient with a spec
3. **One-time research with durable output** — pays for itself (e.g. upstream
   sync analysis written into `UPSTREAM_SYNC.md`)
4. **Refactoring** — low priority; defer unless actively blocking work
5. **Exploration / "what does this code do?"** — use sparingly; read it
   yourself when you can

### The Meta-Principle

Claude Code works best when you treat it like a skilled contractor, not a
search engine. A contractor does their best work when handed a blueprint, not
when asked to figure out what to build. The more you've thought through a task
before opening a session, the more of your token budget goes toward actual
work rather than planning overhead.
