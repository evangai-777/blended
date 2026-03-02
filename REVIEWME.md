# Blended on GitHub Pages — WebAssembly Port Plan

## Context

Blended (a Blender 5.2 fork with a tiered UI system) is currently a native desktop application for Windows/macOS/Linux. The goal is to make it run **in the browser via WebAssembly**, served from GitHub Pages — a first-of-its-kind achievement for a full Blender-based 3D editor online.

**Why this is feasible:** The codebase already has an SDL2 windowing backend (`intern/ghost/intern/GHOST_SystemSDL.cc`) and an OpenGL rendering backend (`source/blender/gpu/opengl/`). SDL2 has native Emscripten support, and Emscripten maps OpenGL to WebGL2 automatically. Many heavy features (Cycles, Python, FFMPEG, etc.) can be disabled via CMake flags.

**What we're building:** An Emscripten-compiled WASM build of Blended with the full UI editor (3D viewport, mesh editing, sculpting, Eevee rendering, modifiers, UV editing), served from GitHub Pages with a web shell (HTML/JS/CSS). Threading via `coi-serviceworker` for SharedArrayBuffer support. Plus documentation about the roadmap for bringing the rest of Blended's features online in future stages.

---

## Phase 1: Emscripten CMake Toolchain & Config

**Goal:** Add build infrastructure for cross-compiling to WebAssembly.

### Files to create:

1. **`build_files/cmake/config/blended_wasm.cmake`** — Minimal WASM build config
   - Based on `blender_headless.cmake` but with UI enabled via SDL
   - Disables: Python, Cycles, FFMPEG, audio, VDB, fluid sim, Bullet, NDOF, XR, LibMV, Alembic, USD, Hydra
   - Enables: `WITH_GHOST_SDL=ON`, `WITH_OPENGL_BACKEND=ON`
   - Disables: `WITH_VULKAN_BACKEND=OFF`, `WITH_METAL_BACKEND=OFF`
   - Sets Emscripten-specific flags (WASM=1, pthreads, memory settings)

2. **`build_files/cmake/platform/platform_emscripten.cmake`** — Platform detection & Emscripten-specific settings
   - Handles `CMAKE_SYSTEM_NAME STREQUAL "Emscripten"` detection
   - Sets linker flags: `-s USE_SDL=2`, `-s USE_WEBGL2=1`, `-s FULL_ES3=1`, `-s ALLOW_MEMORY_GROWTH=1`
   - Sets initial memory (512MB), max memory (4GB)
   - Enables SharedArrayBuffer for threading (`-s USE_PTHREADS=1`, `-s PTHREAD_POOL_SIZE=4`)
   - Configures Emscripten virtual filesystem (MEMFS + IDBFS for persistence)

### Files to modify:

3. **`CMakeLists.txt`** — Add Emscripten platform detection
   - Add `elseif(EMSCRIPTEN)` branch in platform detection section (~line 600-700 area)
   - Include `platform_emscripten.cmake`
   - Gate out incompatible targets

4. **`intern/ghost/CMakeLists.txt`** — Ensure SDL path works for Emscripten
   - May need minor tweaks to SDL detection when using Emscripten's built-in SDL2

---

## Phase 2: GHOST/SDL Emscripten Adaptations

**Goal:** Make the windowing layer work with Emscripten's event loop model.

### Key challenge:
Blender uses a blocking main loop (`while(running) { processEvents(); draw(); }`). Emscripten requires `emscripten_set_main_loop()` — a callback-based non-blocking loop.

### Files to modify:

5. **`intern/ghost/intern/GHOST_SystemSDL.cc`** — Add Emscripten main loop support
   - Wrap the main loop with `#ifdef __EMSCRIPTEN__` to use `emscripten_set_main_loop_arg()`
   - Adapt event processing for browser event model

6. **`source/blender/windowmanager/intern/wm_init_exit.cc`** — Adapt WM initialization
   - Add Emscripten-specific initialization path
   - Use `emscripten_set_main_loop()` instead of the blocking `WM_main()` loop

7. **`source/blender/windowmanager/intern/wm_window.cc`** — Canvas/window setup
   - Map Blender window to HTML canvas element
   - Handle browser resize events via Emscripten callbacks

---

## Phase 3: Web Shell (HTML/JS/CSS)

**Goal:** Create the web page that loads and runs the WASM module.

### Files to create:

8. **`build_files/web/index.html`** — Main web page
   - Loading screen with progress bar (WASM modules are large)
   - Full-viewport `<canvas>` element for Blender rendering
   - COOP/COEP headers meta tags (required for SharedArrayBuffer/threading)
   - Responsive layout

9. **`build_files/web/blended.js`** — Module loader & glue
   - Emscripten module configuration (canvas element, memory, filesystem)
   - File drag-and-drop support for loading .blend files
   - Virtual filesystem setup (preload essential Blender data files)
   - Browser feature detection (WebGL2, SharedArrayBuffer, WASM)

10. **`build_files/web/blended.css`** — Styling
    - Loading screen styling, canvas fullscreen, responsive design

11. **`build_files/web/coi-serviceworker.min.js`** — COOP/COEP header injection
    - Enables SharedArrayBuffer on GitHub Pages (which doesn't support custom HTTP headers)
    - Loaded from `index.html` before the main module
    - Source: `coi-serviceworker` npm package (MIT licensed, ~1KB)

---

## Phase 4: GitHub Actions Workflow

**Goal:** CI/CD pipeline that builds the WASM binary and deploys to GitHub Pages.

### Files to create:

12. **`.github/workflows/build-wasm.yml`** — WASM build & deploy workflow
    - Trigger: push to `main`, tags, manual dispatch
    - Steps:
      1. Checkout source
      2. Install Emscripten SDK (via `mymindstorm/setup-emsdk` action)
      3. CMake configure with `blended_wasm.cmake` and Emscripten toolchain
      4. Build with `emcmake cmake` + `emmake make`
      5. Copy web shell files + WASM output to deploy directory
      6. Deploy to GitHub Pages via `actions/deploy-pages@v4`
    - Caching: Emscripten SDK, build artifacts

---

## Phase 5: GitHub Pages Configuration

**Goal:** Enable GitHub Pages deployment.

### Files to create/modify:

13. **`.github/workflows/build-wasm.yml`** (same file from Phase 4)
    - Uses `actions/configure-pages`, `actions/upload-pages-artifact`, `actions/deploy-pages`

14. COOP/COEP handled by `coi-serviceworker.min.js` (file #11 above) — no `_headers` file needed since GitHub Pages doesn't support custom HTTP headers

---

## What Gets Disabled (and why)

| Feature | Flag | Reason |
|---------|------|--------|
| Python scripting | `WITH_PYTHON=OFF` | Requires full CPython; massive porting effort |
| Cycles renderer | `WITH_CYCLES=OFF` | Needs CUDA/GPU compute; Eevee works via WebGL |
| FFMPEG | `WITH_CODEC_FFMPEG=OFF` | Video codecs not needed for web editor |
| Audio | `WITH_AUDASPACE=OFF` | No audio needed initially |
| Fluid simulation | `WITH_MOD_FLUID=OFF` | Mantaflow is heavy |
| Motion tracking | `WITH_LIBMV=OFF` | Not a core modeling feature |
| VR/XR | `WITH_XR_OPENXR=OFF` | No WebXR bridge yet |
| Vulkan/Metal | `OFF` | WebGL2 only |

**What STAYS enabled:** 3D viewport, mesh editing, sculpting, Eevee real-time rendering, basic modifiers, UV editing, texture painting, file I/O for .blend files (via virtual FS).

---

## Known Risks & Mitigations

1. **Binary size** — WASM binary may be 50-150MB. Mitigation: aggressive dead code elimination, `wasm-opt`, gzip/brotli compression (GitHub Pages serves compressed), lazy loading.

2. **Threading/SharedArrayBuffer** — Requires COOP/COEP headers. GitHub Pages doesn't support custom headers natively. Mitigation: Use `coi-serviceworker` (a tiny JS file) that intercepts responses and injects the required headers client-side. Widely used by Emscripten web apps.

3. **Compilation errors** — Blender's codebase isn't tested against Emscripten. Expect `#ifdef` gaps, missing platform definitions, incompatible system calls. Mitigation: Iterative approach — fix compile errors one at a time.

4. **OpenGL compatibility** — Blender uses OpenGL 3.3 core; WebGL2 is based on OpenGL ES 3.0. Some features may not map 1:1. Mitigation: Blender's GPU abstraction layer already handles ES/desktop differences to some degree.

5. **Memory pressure** — Browsers limit WASM memory. Mitigation: Start with `INITIAL_MEMORY=512MB`, `ALLOW_MEMORY_GROWTH=1`, `MAXIMUM_MEMORY=4GB`.

---

## Phase 6: Roadmap Documentation

**Goal:** Document what's included in v1, what's excluded, and the path to bringing more features online.

### Files to create:

15. **`build_files/web/WEBASSEMBLY_ROADMAP.md`** — Future stages documentation
    - **Stage 1 (this PR):** Core editor — viewport, mesh editing, sculpting, Eevee, modifiers, UV editing
    - **Stage 2:** Python scripting via Pyodide/micropython — enables add-ons, scripted workflows, the property panels that rely on Python
    - **Stage 3:** Cycles rendering via WebGPU compute shaders (when browser WebGPU matures)
    - **Stage 4:** Audio support via Web Audio API + Emscripten audio backend
    - **Stage 5:** Video/FFMPEG via browser MediaRecorder API or ffmpeg.wasm
    - **Stage 6:** Collaborative editing via WebRTC/WebSocket
    - Notes on how Blended's tiered UI system (Simple/Standard/Advanced) maps well to progressive web feature loading — the "Simple" tier could work with Stage 1 alone
    - Performance optimization opportunities (WASM SIMD, streaming compilation, module splitting)

---

## Why This Works for Blended Specifically

Blended's **tiered UI system** is uniquely well-suited for a web port:

- **Simple tier** needs minimal UI elements — could work with Stage 1 alone (no Python needed for basic 3D editing)
- **Standard tier** adds more panels and options — maps to Stage 2 when Python scripting is restored
- **Advanced tier** unlocks everything — the end goal when all stages are complete

This means Blended can ship a *useful* web editor at Stage 1 that naturally targets the "Simple" tier audience, then progressively unlock tiers as web capabilities expand.

---

## Implementation Order

The work should be done in this order:

1. Create `blended_wasm.cmake` config (Phase 1)
2. Create `platform_emscripten.cmake` (Phase 1)
3. Modify `CMakeLists.txt` for Emscripten detection (Phase 1)
4. Create web shell files + `coi-serviceworker` (Phase 3)
5. Adapt GHOST SDL for Emscripten main loop (Phase 2)
6. Adapt window manager for Emscripten (Phase 2)
7. Create GitHub Actions workflow (Phase 4)
8. Iterate on compilation fixes (ongoing)
9. Configure GitHub Pages deployment (Phase 5)
10. Write `WEBASSEMBLY_ROADMAP.md` with future stages (Phase 6)

---

## Verification

1. **Local test:** Install Emscripten SDK, run `emcmake cmake` with WASM config, verify it configures without errors
2. **Build test:** Run `emmake make` and fix compilation errors iteratively
3. **Browser test:** Serve output with `emrun` or `python -m http.server` (with COOP/COEP headers) and verify canvas renders
4. **CI test:** Push to branch, verify GitHub Actions workflow runs and deploys to Pages
5. **End-to-end:** Visit the GitHub Pages URL and interact with Blended in the browser

---

## Key Codebase Entry Points (for reference)

| Component | Path | Relevance |
|-----------|------|-----------|
| SDL windowing | `intern/ghost/intern/GHOST_SystemSDL.cc` | Main loop adaptation for Emscripten |
| SDL window | `intern/ghost/intern/GHOST_WindowSDL.cc` | Canvas mapping |
| SDL context | `intern/ghost/intern/GHOST_ContextSDL.cc` | WebGL context creation |
| OpenGL GPU backend | `source/blender/gpu/opengl/` | Maps to WebGL2 via Emscripten |
| Window manager | `source/blender/windowmanager/` | Main loop, initialization |
| GHOST CMake | `intern/ghost/CMakeLists.txt` | SDL backend toggle (line 138) |
| Release config | `build_files/cmake/config/blended_release.cmake` | Template for WASM config |
| Headless config | `build_files/cmake/config/blender_headless.cmake` | Template for minimal build |
| Root CMake | `CMakeLists.txt` | Platform detection, feature flags |
