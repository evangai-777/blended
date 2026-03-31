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
