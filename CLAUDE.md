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
