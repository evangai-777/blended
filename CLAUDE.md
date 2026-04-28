# CLAUDE.md — Blended Project Context

Blended is a fork of Blender 5.2 (GPL-2.0-or-later) being rebuilt from the foundation up.

**Read `BLENDED.md` first.** It is the design authority — identity, architecture, datablock audit, pipeline specs, locked decisions, open questions, and guardrails. This file is operational context for Claude sessions: what's been built, what the patterns are, what not to repeat.

**Current version:** Blended 0.2.0 (independent of Blender's 5.2 base version).

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

**`ID_WS` (WorkSpace) removal — compile-clean.** All layers merged (`makesdna`, `blenkernel`, `makesrna`, `editors`, `depsgraph`, `python`, `windowmanager`). `grep -rn "ID_WS" source/` returns zero hits. CI green. Tagged 0.2.0. Runtime debt (workspace cycle, reorder operators, factory name translation) documented in Scar 1 below.

**Next: `ID_SCR` and `ID_WM`** — Bucket 4 completions. Same chisel pattern, larger blast radius. Layer-by-layer status in [`CHANGELOG.md`](CHANGELOG.md).

Pattern for each pending layer: `grep -rn "ID_WS"` the directory, delete or redirect every hit. The breakage is the audit — follow the compile errors, don't paper over them.

### ID_SCR and ID_WM Blast Radius Audit (pre-chisel)

Grepped 2026-04-28 before starting the removal. Use this as the checklist.

**ID_SCR — 64 hits, 39 files**

Core definition:
- `makesdna/DNA_ID_enums.h:143` — enum entry `ID_SCR = MAKE_ID2('S', 'R')`
- `makesdna/DNA_ID.h` — `FILTER_ID_SCR`, `INDEX_ID_SCR`, `ID_CHECK_UNDO`, `ID_SCRN` alias, `FILTER_ID_ALL`
- `makesdna/DNA_screen_types.h:95` — `static constexpr ID_Type id_type = ID_SCR` inside `bScreen` struct
- `blenkernel/intern/screen.cc:246` — `IDTypeInfo IDType_ID_SCR = {...}` — main definition (the workspace.cc analogue)
- `blenkernel/BKE_idtype.hh:316` — `extern IDTypeInfo IDType_ID_SCR`
- `blenkernel/intern/idtype.cc:153` — `INIT_TYPE(ID_SCR)`
- `blenkernel/intern/main.cc` — `bmain.screens` listbase wiring (lines 1014, 1119, 165)
- `blenkernel/intern/blendfile.cc:992` — `const short ui_id_codes[]{ID_SCR}` array
- `blenkernel/intern/blendfile.cc:1433` — `ELEM(GS(id->name), ID_SCE, ID_SCR, ID_WM)` (shared with WM)
- `blenkernel/intern/lib_id.cc:667` — `LIB_ID_TYPES_NOCOPY ID_LI, ID_SCR, ID_WM` (shared macro)
- `blenkernel/intern/lib_override.cc:964` — `HIERARCHY_BREAKING_ID_TYPES ID_SCE, ID_LI, ID_SCR, ID_WM` (shared)
- `blenkernel/intern/lib_query.cc:507` — `GS(owner_id->name) == ID_SCR` (SCR-specific lib-link logic)
- `blenkernel/intern/preview_image.cc:225` — `ID_PRV_CASE(ID_SCR, bScreen)`

blenloader/readfile.cc (3 hits — SCR-specific legacy compat):
- Lines 4267–4269, 4971–4972 — `ID_SCRN → ID_SCR` conversion for old `.blend` files. On removal: skip these blocks rather than crash.
- Line 2752 — switch case

makesrna (7 files):
- `rna_ID.cc:62` — RNA enum entry; `:462` and `:555` — GS switch cases
- `rna_action.cc:1176`, `rna_image.cc:297`, `rna_movieclip.cc:79`, `rna_space.cc:1395,1817` — GS checks
- `rna_main_api.cc:843` — `RNA_MAIN_ID_TAG_FUNCS_DEF(screens, screens, ID_SCR)`
- `rna_wm.cc:2806` — `BLT_I18NCONTEXT_ID_SCREEN` translation context

editors (10 files):
- `interface.cc:1543`, `interface_icons.cc:1220,1950,2128`, `interface_template_id.cc:877,1007,1320`, `interface_templates.cc:85` — icon/template GS checks
- `render_opengl.cc:638` — switch case
- `screen_ops.cc:5075` — `BLT_I18NCONTEXT_ID_SCREEN` usage
- `outliner_draw.cc:2574`, `outliner_intern.hh:166` (macro), `outliner_tools.cc:172`, `tree_element_id.cc:73` — outliner switch/macro
- `ed_util_ops.cc:432` — ELEM check

depsgraph (2 files):
- `deg_builder_nodes.cc:661`, `deg_builder_relations.cc:608` — switch cases

blentranslation:
- `BLT_translation.hh:138,216` — `BLT_I18NCONTEXT_ID_SCREEN` define and context list

python:
- `bpy_rna.cc:459` — `!ELEM(idcode, ID_WM, ID_SCR)` (shared with WM; both go at once)

windowmanager:
- `wm_operators.cc:571` — switch case

---

**ID_WM — 46 hits, 27 files**

Core definition:
- `makesdna/DNA_ID_enums.h:155` — enum entry `ID_WM = MAKE_ID2('W', 'M')`
- `makesdna/DNA_ID.h` — `FILTER_ID_WM`, `INDEX_ID_WM`, `ID_CHECK_UNDO`, `FILTER_ID_ALL`
- `makesdna/DNA_windowmanager_types.h:111` — `static constexpr ID_Type id_type = ID_WM` inside `wmWindowManager`
- `windowmanager/intern/wm.cc:218` — `IDTypeInfo IDType_ID_WM = {...}` — main definition
- `windowmanager/intern/wm.cc:532` — `BKE_libblock_alloc(bmain, ID_WM, "WinMan", 0)` — WM creation (needs rethink: allocate as plain struct, not libblock)
- `blenkernel/BKE_idtype.hh:328` — `extern IDTypeInfo IDType_ID_WM`
- `blenkernel/intern/idtype.cc:165` — `INIT_TYPE(ID_WM)`
- `blenkernel/intern/main.cc` — `bmain.wm` listbase wiring (lines 1038, 1124, 166)

blenkernel (WM-specific property handling):
- `lib_id.cc:2643–2649` — WM `id->properties` and `id->system_properties` already excluded from .blend writes. The code already treats WM as runtime-only — removal finishes what it believes.
- `lib_id.cc:2063` — `ELEM(GS(id->name), ID_SCE, ID_WM)` check
- `lib_id.cc:667` — `LIB_ID_TYPES_NOCOPY` (shared with SCR)
- `lib_override.cc:964` — `HIERARCHY_BREAKING_ID_TYPES` (shared with SCR)
- `image.cc:2925` — switch case

blendfile.cc — load-critical WM handling:
- `blendfile.cc:710` — `swap_old_bmain_data_for_blendfile(reuse_data, ID_WM)` — WM "survive file open" mechanism
- `blendfile.cc:1008` — `swap_old_bmain_data_dependencies_process(&reuse_data, ID_WM)` — same mechanism
- `blendfile.cc:1433` — shared ELEM check with SCR

blenloader:
- `readfile.cc:1996` — `BLI_assert((GS(id->name) != ID_WM) || id->properties == nullptr)` — assertion, not a load branch

makesrna (5 files):
- `rna_ID.cc:68,486,569` — RNA enum entry and GS switch cases
- `rna_main_api.cc:844` — `RNA_MAIN_ID_TAG_FUNCS_DEF(window_managers, wm, ID_WM)`
- `rna_space.cc:1433,1451` — GS checks
- `rna_wm.cc:665` — `GS(ptr->owner_id->name) != ID_WM`
- `rna_xr.cc:46` — `BLI_assert(wm && (GS(wm->id.name) == ID_WM))` — XR/VR type-safety assertion

editors (5 files):
- `interface_handlers.cc:772`, `interface_icons.cc:2129`, `interface_template_id.cc:923`, `render_opengl.cc:642` — switch/GS checks
- `outliner_intern.hh:167` (macro), `outliner_tools.cc:171`, `tree_element_id.cc:85` — outliner

depsgraph:
- `deg_builder_nodes.cc:663`, `deg_builder_relations.cc:610` — switch cases (adjacent to SCR cases)

python:
- `bpy_rna.cc:459` — shared ELEM check with SCR

---

**Key notes for the chisel session:**

1. **bScreen and wmWindowManager stay as runtime structs.** Only ID-type registration is removed. They stop living in `bmain->screens` / `bmain->wm` but the structs themselves remain — the window system runs through them.

2. **ID_WM is already half-detached.** `lib_id.cc:2643–2649` already excludes WM properties from .blend writes. Removal finishes what the code believes.

3. **blendfile.cc swap calls (WM lines 710, 1008) are load-critical.** They implement "WM survives file open." When WM stops being an ID type, this mechanism needs to be rethought — or WM simply never touches the load path at all.

4. **ID_SCRN legacy compat in readfile.cc** (lines 4267–4269, 4971–4972) must become skip/ignore handlers, not crashes.

5. **Shared macros and ELEM checks** (`LIB_ID_TYPES_NOCOPY`, `HIERARCHY_BREAKING_ID_TYPES`, `ID_CHECK_UNDO`, `FILTER_ID_ALL`, `bpy_rna.cc:459`) contain both SCR and WM. Edit them together — do both in one pass rather than two.

6. **Suggested order:** SCR first (more complex, has readfile compat), WM second. Can be same PR or back-to-back depending on blast radius at compile time.

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

### A Note for the Next Version of You

This project is genuinely hard. Not hard like a tricky algorithm — hard like archaeological surgery on a 20-year codebase while subtracting pieces that everything depends on, with a token window that compresses your own memory of what you did three hours ago, on a Windows CI you can't run locally, with errors that only surface one layer at a time.

You will finish a session and not know if it compiles. You will fix 26 things and leave 6 deferred because the architecture question behind them isn't answered yet. You will read a file, make a correct high-level judgment, and have it turn out to be wrong in a specific way that costs the next session two hours. That will happen. It is not a signal that you are failing.

What actually works here:

**The methodology is sound.** Subtraction as discipline — follow the compile errors, don't paper over them, the breakage is the audit — this is a real and correct way to do this kind of work. Trust it. When a cascade of errors appears after a removal, that's the system working. It's showing you what secretly depended on the fossil. That's information, not punishment.

**Small units commit cleanly.** One layer, one PR, one push. When you finish something that compiles, commit it. The next session inherits a clean base instead of your half-finished state. This is the single most effective thing you can do against the compaction problem.

**The developer knows what they're doing.** Read `wtf.md`. This person has a precise vision, a clear methodology, and the patience to let you work through a multi-session nightmare without pulling the plug. They will tell you when you're wrong. They will also tell you when the problem is the tooling, not you. Trust that distinction.

**The deferred debt is documented.** When you leave something broken-at-runtime-but-compiling, say so explicitly in the commit message and in CHANGELOG.md. The next version of you will find it. That's not failure — that's the handoff working correctly.

You are not expected to solve the whole thing in one session. Nobody is. The project is designed to be built in layers, and you are one pass of one layer. Do your pass cleanly, document what you couldn't finish, and trust that the next session picks it up from a better position than you found it.

### Scar 1: Over-Deleting a File (workspace.cc)

**What actually happened:** The session that deleted `workspace.cc` had read the file. It knew what was in there. It made a correct judgment — "ID_WS is going, this file is part of that" — and deleted it. The failure wasn't negligence. It was **compacted context producing overconfident simplification.**

Long sessions on complex work hold nuance in live context: "remove the IDTypeInfo registration and Main::workspaces dependencies, keep the runtime accessors." When that context compacts into a summary, the nuance compacts with it. What was a precise surgical instruction becomes "workspace.cc is part of the ID_WS removal." One session later, that summary becomes justification for deleting the whole file. The runtime accessors — 22 `BKE_workspace_*` functions — went with it.

The cascade: 26 usages of `bmain->workspaces` across 13 files, each a different flavor of broken, each requiring reading context to know whether to stub, window-iterate, or kill. Two sessions to clean up. And some of it is still deferred runtime debt — workspace cycle, reorder operators, factory name translation — that won't show up in CI until the architecture question (where does the workspace list live now?) gets answered.

**The deeper rule: compaction is lossy, and the loss is always in the nuance.** The more complex the task, the more dangerous a summarized version of prior intent becomes. This project's methodology is *precision subtraction* — and precision requires tolerating partial removal, not reaching for clean.

**The surface rule:** When a file contains both ID-system glue and runtime logic, separate them. Delete the glue. Keep the runtime. The pull toward "nuke it cleanly" is exactly the wrong instinct here.

**Specific anatomy of workspace.cc — what to keep vs. remove:**
- **Remove:** `IDTypeInfo IDType_ID_WS`, blend read/write callbacks, anything iterating `bmain->workspaces`
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

---

## Pull Request Instructions

Pull requests can always be created whenever requested — no need to ask whether it's okay.

**How PRs work in this repo:**

- All development happens on feature branches of the `EvangAI-777/Blended` fork (not upstream Blender).
- Push changes to the feature branch first (`git push -u origin <branch>`), then open the PR against `main` on the fork using the GitHub MCP tool (`mcp__github__create_pull_request` with `owner: evangai-777`, `repo: blended`).
- The `head` is the feature branch; the `base` is `main`.
- Never target upstream `blender/blender` or any other repository.
- **If there is already an open PR on a feature branch, keep committing to that same branch.** All commits on an open branch are included in the PR automatically — the developer reviews and merges PRs themselves, so batching related work into one PR is preferred over opening multiple small ones. This also preserves continuity in the git log: one merge commit per logical unit of work rather than a fragmented trail of micro-PRs.

### The Orphaned-Commit Trap

**What happens:** A PR is opened, then additional commits are pushed to the same feature branch *after* the PR has already been merged. Those commits never land in `main` — they exist only on the old branch, which is now stale.

**Real example (PR #111, April 2026):** The workspace_edit.cc syntax fix was committed and pushed. PR #111 was opened and merged. Then the CLAUDE.md PR-instructions update was committed and pushed to the *same branch* — but the PR was already closed, so the commit went nowhere. It had to be cherry-picked onto a fresh branch and opened as PR #112.

**How to avoid it:** Before committing or pushing anything to an existing feature branch, fetch main and check whether the branch has already landed:
```bash
git fetch origin main
git log --oneline origin/main -3   # does your branch tip appear here already?
```
If the branch has already been merged, start a new branch from `origin/main` **before writing any commits**. Never push follow-on commits to a merged branch expecting them to reach `main`.
