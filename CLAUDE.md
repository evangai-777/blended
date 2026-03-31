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
