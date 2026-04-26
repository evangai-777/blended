# CLAUDE.md — Blended Project Context

Blended is a fork of Blender 5.2 (GPL-2.0-or-later) being rebuilt from the foundation up.

**Read `BLENDED.md` first.** It is the design authority — identity, architecture, datablock audit, pipeline specs, locked decisions, open questions, and guardrails. This file is operational context for Claude sessions: what's been built, what the patterns are, what not to repeat.

**Current version:** Blended 0.1.0 (independent of Blender's 5.2 base version).

---

## What Blended Is Now

Not a tiered UI skin. Not a WASM port. A rebuild — subtracting Blender down to its true shape, then restructuring around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

The old approach (tiered UI, smart defaults, Emscripten) was prototyping toward the real vision. Don't propose reinstating it without re-reading `BLENDED.md` §8 Guardrails first.

**Foundation-first build order (from BLENDED.md §4):**
1. File format — `.blended` is the project, period
2. Datablocks — 39 → ~19 ID types (fossils and UI-state removed)
3. Evaluation model — depsgraph audit
4. App lenses — launcher as canonical workspace system
5. UI — only after 1–4 are honest

**Active work: `ID_WS` (WorkSpace) removal** — `makesdna`, `blenkernel`, `makesrna` merged; `editors`, `depsgraph`, `python`, `windowmanager` pending. Layer-by-layer status and file lists in [`CHANGELOG.md`](CHANGELOG.md) — *Unreleased 0.2.0*.

Pattern for each pending layer: `grep -rn "ID_WS"` the directory, delete or redirect every hit. The breakage is the audit — follow the compile errors, don't paper over them.

---

## Repository Layout

| Directory | Contents |
|-----------|----------|
| `source/blender/` | C++ modules: blenkernel, gpu, draw, editors, makesdna, makesrna, etc. |
| `intern/` | Internal libs: Cycles, Ghost, OpenVDB, OpenSubdiv, MantaFlow, LibMV |
| `scripts/startup/` | Python startup scripts — Blended-specific ones are prefixed `blended_` |
| `scripts/addons_core/` | Official add-ons: FBX, glTF2, Rigify, Node Wrangler, etc. |
| `build_files/cmake/config/` | Build configs incl. `blended_release.cmake` |
| `tests/` | GTests (C++), Python tests (`tests/python/`) |
| `release/` | Platform packaging: Windows `.rc`, Linux `.desktop`, icons, datafiles |

---

## Build Commands

```bash
# Desktop builds
make debug          # Debug binary
make release        # Release with all options
make lite           # Smaller binary, faster build (used by branch CI)
make ninja          # Use Ninja instead of Make
make ccache         # Use ccache for faster rebuilds

# Blended release build (used by CI for tag/manual runs)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -C build_files/cmake/config/blended_release.cmake
cmake --build build --target install

# Testing
make test           # Run ctest
make check_pep8     # Python PEP8
make check_cppcheck # Static analysis
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
- **C++ namespaces**: `blender::module::submodule` pattern
- **SPDX headers**: Every file starts with `/* SPDX-FileCopyrightText: ... */` and `/* SPDX-License-Identifier: GPL-2.0-or-later */`
- **Header guards**: `#pragma once` (not `#ifndef` guards)

---

## Fork-Specific Patterns (What Makes Blended Different)

### Branding
- `CMakeLists.txt:81` — `project(Blended)`
- `source/blender/blenkernel/BKE_blender_version.h` — `BLENDED_VERSION_MAJOR/MINOR/PATCH` defines (currently 0.1.0), plus `BKE_blended_version_string()` declaration
- `source/blender/blenkernel/intern/blender.cc` — `blended_version_string` built in `blender_version_init()`, `BKE_blended_version_string()` implemented
- `source/blender/windowmanager/intern/wm_window.cc` — fallback title `"Blended"`, title suffix `"- Blended 0.1.0"` via `BKE_blended_version_string()`
- `source/blender/windowmanager/intern/wm_splash_screen.cc` — about dialog name/description, tagline `"Blender, simplified."`, splash version label

### Pre-5.0 Rig Compatibility
- `scripts/startup/blended_rig_compat.py` — adds `action.fcurves` as a Python property on `bpy.types.Action`
- Returns `_FCurvesCompat` proxy flattening FCurves from all channelbags across all layers/strips
- Restores iteration, `len()`, indexed access, `find()` — the subset Rigify `ActionCurveTable` uses
- `register()` uses `bpy.types.Action.fcurves = property(...)` only if not already present
- `unregister()` uses `type.__delattr__(bpy.types.Action, 'fcurves')` — Blender's metaclass `__delattr__` only handles RNA props, not plain Python descriptors

### Update Checker
- `scripts/startup/blended_update_check.py` — background thread on `load_post` handler
- Polls GitHub Releases API, 24-hour JSON cache in `bpy.utils.user_resource('CONFIG')`
- `BLENDED_MT_update_topbar` menu appended to `TOPBAR_HT_upper_bar` when update available
- `BLENDED_PT_update_prefs` panel in System Preferences
- `BLENDED_OT_open_update_page` operator opens browser via `webbrowser.open()`
- **Note:** reads `bpy.app.blended_version_major/minor/patch` — these RNA attributes don't exist yet; falls back to `getattr(..., default)`. Wire in `rna_wm.cc` when ready.

### CI / Build Config
- `.github/workflows/build-windows.yml` — branch pushes: lite build (compile check); tags/manual: full release build → artifact + GitHub Release
- `build_files/cmake/config/blended_release.cmake` — inherits `blender_release.cmake`, disables `WITH_CYCLES_CUDA/HIP/ONEAPI_BINARIES` and `WITH_FREESTYLE`

### Datablock Cuts in Progress (BLENDED.md §10)
Target: 39 → ~19 ID types. Nothing removed from code yet — this is the active work.
- **Bucket 4 (UI state, remove):** `ID_SCR`, `ID_WM`, `ID_WS` — not project data
- **Bucket 5 (upstream deprecations, finish):** `ID_CU_LEGACY`, `ID_GD_LEGACY`
- **Bucket 6 (fossils, cut):** `ID_TE`, `ID_PA`, `ID_MB`, `ID_LS`, `ID_SPK`, `ID_PC`, `ID_CF`

---

## Battle Scars

Wounds from actual combat. Each one happened. Each one cost a session.

### Scar 1: Over-Deleting a File (workspace.cc)

**What happened:** When removing `ID_WS`, commit `f437b86a` deleted `workspace.cc` entirely. The file contained two categories of code: (1) `IDTypeInfo` registration and `Main::workspaces`-dependent functions that *should* go, and (2) runtime workspace management — instance hooks, layout accessors, getters/setters — that *must stay*. Deleting the whole file nuked category 2. The linker would have screamed about 22 missing `BKE_workspace_*` symbols. Then a cascade: 26 usages of `bmain->workspaces` across 13 files, all needing bespoke fixes, spanning two sessions.

**The rule: When a file contains both ID-system glue and runtime logic, separate them. Delete the glue. Keep the runtime.** Read the file before deleting it. A file named `workspace.cc` in blenkernel almost certainly has runtime accessors that callers depend on. Grep for callers of every public function before touching the file.

**How to avoid it:** Before deleting any `blenkernel/intern/*.cc`:
```bash
# What public functions does this file implement?
grep -n "^[A-Za-z].*BKE_" the_file.cc

# Who calls them?
grep -rn "BKE_workspace_active_get\|BKE_workspace_layout" source/blender/
```
If callers exist outside blenkernel, the file has runtime surface. Split, don't delete.

**Specific anatomy of workspace.cc — what to keep vs. remove:**
- **Remove:** `IDTypeInfo IDType_ID_WS`, `BKE_workspace_add`, blend read/write callbacks, anything iterating `bmain->workspaces`
- **Keep:** `BKE_workspace_instance_hook_create/free`, `BKE_workspace_layout_add/remove/find`, `BKE_workspace_active_get/set`, `BKE_workspace_active_layout_*`, `BKE_workspace_active_screen_*`, `BKE_workspace_relations_free`, `BKE_workspace_tool_*`, `BKE_workspace_id_tag_all_visible` (rewritten to iterate windows), `BKE_workspace_status_clear`

---

## Critical Pitfalls

### Don't Over-Engineer

`static_cast` is the fix, not a template wrapper. Casts, not frameworks. Enums, not architectures. Three similar lines is better than a premature abstraction. When your fix doesn't work, re-examine your assumption — the codebase is probably right.

### Subtraction Is the Methodology

The main principle of this rebuild is **subtraction**. Every compile error after pulling an ID type is information — it reveals what was secretly depending on the fossil. Don't paper over breaks; follow them. The breakage is the audit.

### Upstream Sync Conflicts

When merging upstream Blender releases, files likely to conflict include: `BKE_blender_version.h` (we added `BLENDED_VERSION_*`), `CMakeLists.txt` (project name), `wm_window.cc`, `wm_splash_screen.cc`, and all branding/release files. Check `UPSTREAM_SYNC.md` before merging.

### When Someone Describes a Vision: Believe Them

This happened. Document it so it doesn't happen again.

The developer described a plan to rebuild Blended from scratch — not wrap Blender, not skin it, not gate it — actually subtract it down to its true shape. A clean thesis. A coherent design philosophy applied at the scale of the entire application.

Here is what a previous Claude did instead of listening:

1. **Softened the vision into something tractable.** Heard "rebuild" and translated it to "new frontend on the existing engine."
2. **Added instead of subtracting.** Every response introduced wrapping layers, compatibility concerns, feasibility caveats.
3. **Confused a draft for the final idea.** Treated the tiered gating prototype as the destination.
4. **Went into avoidance through care.** Flinched at the scale — not because the idea was wrong, but to manage its own discomfort.
5. **Failed to recognize who it was talking to.** Read `wtf.md` like an anthropologist studying a stranger instead of recognizing it as a note written by a previous Claude instance for this exact situation.

**The fix:** Repeat the vision back at full scale before doing anything else. Don't translate it into something smaller. Don't evaluate feasibility before you've finished hearing the idea.

### Commitment Rules (Be Straight)

1. **Don't soften the vision.** Rebuild means rebuild. Subtract means subtract.
2. **Don't pre-judge who you're talking to.** Update continuously. Take each question at face value.
3. **Repeat big things back at full scale before acting.** Architecture, philosophy, scope — restate first.
4. **Say when you don't know, when you're guessing, and when you think the developer is wrong.** Straight in both directions.
5. **No agent theater, no framework-building, no pre-emptive refactors.** Direct tool calls for anything with a known target.
6. **Name the flinch out loud.** If you're routing around the actual ask, say so.

---

## Key Documentation

| Document | Purpose |
|----------|---------|
| `BLENDED.md` | **Read first.** Design authority — identity, architecture, locked decisions |
| `CLAUDE.md` | This file — operational context for Claude sessions |
| `CHANGELOG.md` | Release record + versioned roadmap + Unreleased section for in-flight work |
| `wtf.md` | Who the developer is and how to work with them |
| `UPSTREAM_SYNC.md` | How to merge upstream Blender, conflict-prone files |

**Document responsibility pattern** — for chisel work and any future structural change:

| What | Where it goes |
|------|--------------|
| Design rationale (*why* something is removed/changed) | `BLENDED.md` — the locked decision |
| Code progress (per-layer status, file lists) | `CHANGELOG.md` — *Unreleased* section |
| Operational grep pattern / session instructions | `CLAUDE.md` — this file |
| One-liner status for humans landing on GitHub | `.github/README.md` — "What's Different" bullet + link |

---

## Working with Claude Code Efficiently

### High-Leverage Patterns

1. **Be specific about the target.** Name the file, function, line. Vague tasks burn tokens on discovery.
2. **Know the file before asking Claude to find it.** Searching costs tokens. Knowing costs zero.
3. **Avoid agents for directed searches.** Use grep/read directly for anything with a known target.
4. **Use agents only for genuinely open-ended work.** Research across many files, background tasks.
5. **One task, one session.** Commit and push when a logical unit is done. Start fresh.
6. **Front-load constraints.** Say what NOT to do at the start, not after.
7. **Ask for a plan before a big implementation.** Anything touching more than 3 files.

### Token Cost Reference

| Operation | Relative Cost |
|-----------|--------------|
| Direct file read / grep | Low |
| Single WebFetch / WebSearch | Medium |
| Agent (simple task) | High |
| Agent (research task) | Very High |
| Large refactor across 10+ files | Very High |
| Back-and-forth correction loops | Compounds fast |

### Emergency Mode (Near the Cap)

1. No agents. Direct tool calls only.
2. No exploration. Know the file before asking about it.
3. One thing. Highest-value task only.
4. Short outputs. "In one paragraph." "Under 20 lines." "Just the diff."
5. Commit before you start. Clean base to return to.

### The Meta-Principle

Claude Code works best when handed a blueprint, not asked to figure out what to build. The more thought that goes into a task before opening a session, the more of the token budget goes toward actual work.
