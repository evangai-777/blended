# Blended WebAssembly Roadmap

This document tracks the staged approach to bringing Blended's features to the browser via WebAssembly. Updated with findings from codebase analysis of the Blender 5.2 draw system, GPU backend, and subsystem dependencies.

> *"Belief LITERALLY shapes reality. So... believe things and watch them
> manifest. It's not metaphor. It's engineering."*
> — [PHILOSOPHY.md](../../PHILOSOPHY.md)
>
> This roadmap exists because we believe a full 3D editor can run in a
> browser. Every stage below started as "this probably won't work" and
> became working infrastructure. The approach: do the work, one stage at a
> time. Fix what's in front of you. The next stage reveals itself.

---

## Stage 1: Core Editor (Current)

**Status:** In progress — build infrastructure complete, compilation fixes ongoing

The initial WASM port includes the essential 3D editing experience:

- **3D Viewport** — Full orbit, pan, zoom with mouse/trackpad
- **Mesh editing** — Edit mode, vertex/edge/face selection, transforms
- **Sculpting** — Brush-based sculpting tools
- **Eevee rendering** — Real-time rendering via WebGL2
- **Basic modifiers** — Mirror, subdivision, array, boolean, etc.
- **UV editing** — UV unwrap, UV editor workspace
- **Texture painting** — Basic texture paint tools
- **File I/O** — Load/save .blend files via browser virtual filesystem
- **Import/Export** — OBJ, STL, PLY mesh formats
- **Drag-and-drop** — Drop .blend/.obj/.stl files onto the browser window

**What's disabled:**
Python scripting, Cycles renderer, FFMPEG/audio, physics simulations (fluid, cloth, particles via Mantaflow), motion tracking, VR/XR, Vulkan/Metal backends, USD/Alembic/MaterialX.

**Why this works for Blended:**
Blended's tiered UI system means the "Simple" tier works perfectly with Stage 1 alone — users get a clean, focused 3D editor without needing Python or advanced features.

### Critical Technical Challenges

#### GPU Backend: GL 4.3 Version Gate

`source/blender/gpu/opengl/gl_backend.cc:223` hard-requires `epoxy_gl_version() >= 43` (OpenGL 4.3). WebGL2 is based on OpenGL ES 3.0 (roughly equivalent to GL 3.3). **The GPU backend will refuse to initialize on WebGL2 without modification.**

#### Compute Shaders in Core Draw System

Blender 5.x's draw module uses `GPU_compute_dispatch` in **core rendering paths** — not just Eevee, but the draw infrastructure itself. WebGL2 does not support compute shaders. This affects ALL rendering:

| File | Usage | Emscripten Status |
|------|-------|-------------------|
| `source/blender/draw/intern/draw_view.cc:251,299` | Visibility culling (core) | **Guarded** — skipped, all-visible fallback |
| `source/blender/draw/intern/draw_command.cc:266,270,277,841` | Indirect draw command generation (core) | **CPU fallback implemented** |
| `source/blender/draw/intern/draw_manager.cc:140` | Resource management (core) | **Guarded** — skipped, local-space bounds |
| `source/blender/draw/intern/draw_cache_impl_subdivision.cc:978` | Mesh subdivision | **Guarded** — skipped, no GPU subdiv |
| `source/blender/draw/engines/eevee/eevee_shadow.cc:1269` | Eevee shadow visibility | **Guarded** — skipped, all-visible |
| `source/blender/draw/engines/workbench/workbench_shadow.cc:265` | Workbench shadow visibility | **Guarded** — skipped, all-visible |

**Resolution:** All 6 compute dispatch sites now have `#ifdef __EMSCRIPTEN__` guards.
The draw command generation has a full CPU fallback that generates indirect draw
commands on CPU when all objects are visible. Visibility culling, resource
finalization, and shadow culling are disabled on Emscripten — acceptable for
Stage 1 where all objects render and performance is secondary to correctness.

#### Missing WebGL2 Features

- ~~**SSBOs**~~ — **DONE** `gl_storage_buffer.cc` now uses `GL_COPY_READ_BUFFER` as
  buffer target and `GL_UNIFORM_BUFFER` for bind slots on Emscripten
- **Texture views** — `glTextureView()` (GL 4.3, not in WebGL2) — stubbed as no-op
- **Image load/store** — used in the storage buffer system — stubbed as no-op
- **`glCopyImageSubData()`** — efficient texture copies (GL 4.3) — stubbed as no-op

#### System Call Incompatibilities (4 files, low risk)

- `source/blender/blenlib/intern/BLI_subprocess.cc` — `fork()`/`execv()` must be stubbed
- `source/blender/blenlib/intern/BLI_mmap.cc` — limited `mmap()` support in Emscripten
- `source/blender/blenlib/intern/fileops_c.cc` — file ops work via Emscripten virtual FS
- `source/blender/gpu/tests/shader_preprocess_test.cc` — test only, not built for WASM

### Resolution Paths

**Path A: Target WebGPU instead of WebGL2**
- WebGPU supports compute shaders, SSBOs, and modern GPU features
- Chrome 113+ (stable since April 2023), Firefox Nightly, Safari 18+
- Blender already has a Vulkan backend; WebGPU's API is Vulkan-like
- Requires a Vulkan-to-WebGPU adaptation layer or new backend
- **Pros:** Full feature support, future-proof, no CPU fallbacks needed
- **Cons:** Narrower browser support, significant backend work

**Path B: CPU fallback paths for WebGL2** ← **IN PROGRESS**
- ~~Add `#ifdef __EMSCRIPTEN__` CPU implementations for the 6 compute dispatch sites~~ **DONE**
- ~~Replace SSBOs with UBO or texture-based fallbacks~~ **DONE** — `gl_storage_buffer.cc`
  uses `GL_COPY_READ_BUFFER` as buffer target on Emscripten, binds via
  `GL_UNIFORM_BUFFER` slots, clear emulated via CPU fill + `glBufferSubData`,
  async readback uses `glCopyBufferSubData` + `glMapBufferRange` instead of
  persistent mapping
- ~~Bypass GL 4.3 version check for Emscripten~~ **DONE** (in `gl_backend.cc`)
- ~~GLSL `#version 430` → `#version 300 es`~~ **DONE** — all 4 shader stage
  patches (vertex, geometry, fragment, compute) in `gl_shader.cc` now emit
  `#version 300 es` with `precision highp` qualifiers on Emscripten
- ~~Runtime GL function gap stubs~~ **DONE** — `glMapBuffer` emulated via
  `glMapBufferRange`, `glGetTexImage` emulated via FBO+`glReadPixels` (2D),
  `glGetBufferSubData` emulated via `glMapBufferRange`+memcpy,
  `glCopyNamedBufferSubData` / `glGetNamedBufferSubData` emulated via
  bind-to-target fallback
- **Pros:** Works on WebGL2, widest browser support
- **Cons:** Performance hit (no culling), some edge cases in texture readback

**Estimated effort:** 2-4 months for a compilable, renderable Stage 1

---

## Stage 2: Audio Support

**Status:** Planned (moved up — easiest win after Stage 1)

Add audio playback and editing via the Web Audio API.

**What this unlocks:**
- Audio scrubbing in the timeline
- Sound strip support in the Video Sequence Editor
- Audio-reactive workflows

**Technical details:**
- Audaspace library lives in `extern/audaspace/` with a clean device abstraction layer
- Has `NULLDevice` fallback, `SoftwareDevice`, `MixingThreadDevice`
- Blender integration gated by `WITH_AUDASPACE` in `source/blender/blenkernel/intern/sound.cc`
- Emscripten provides SDL_audio mapped to Web Audio API automatically
- `SDL_OpenAudioDevice()` works in Emscripten out of the box

**Estimated effort:** 2-4 weeks — enable `WITH_AUDASPACE=ON`, ensure SDL audio works via Emscripten's Web Audio bridge

---

## Stage 3: Frame & Video Export

**Status:** Planned

### Stage 3a: Frame Export

Export rendered frames as PNG/JPEG sequences via the browser virtual filesystem and download API.

**Estimated effort:** ~1 week

### Stage 3b: Video Encoding via ffmpeg.wasm

Add full video codec support via [ffmpeg.wasm](https://ffmpegwasm.netlify.app/) or the browser's MediaRecorder API.

**What this unlocks:**
- Video rendering output (MP4, WebM)
- Video Sequence Editor with video playback
- Image sequence rendering

**Technical approaches:**
- **ffmpeg.wasm** — Full FFMPEG in WASM (~25MB additional), provides encode/decode
- **MediaRecorder API** — Native browser encoding (WebM/VP8/VP9), no extra WASM, format limited
- FFMPEG integration is contained in `source/blender/imbuf/movie/`

**Estimated effort:** 2-3 months for full ffmpeg.wasm integration

---

## Stage 4: Python Scripting

**Status:** Planned

Restore Python scripting via [Pyodide](https://pyodide.org/) (CPython compiled to WASM) or a lightweight alternative.

**What this unlocks:**
- Add-on support
- Scripted workflows and automation
- Property panels that depend on Python registration
- The "Standard" tier becomes fully functional
- Blended's tier-aware smart defaults (`blended_defaults.py`)
- Update notifications (`blended_update_check.py`)

**Integration depth from codebase analysis:**
- 123 `WITH_PYTHON` conditional compilation guards across 43 C++ files
- Key files: `bpy_interface.cc` (10 guards), `context.cc` (10), `scene_edit.cc` (6), `fcurve_driver.cc` (6), `wm_init_exit.cc` (5)
- Most subsystems are well-guarded with `#ifdef WITH_PYTHON` and work without it
- The C++ tier infrastructure (DNA/RNA `ui_tier` field) works without Python — only the smart defaults and update check scripts are affected

**Technical approach — Pyodide integration:**
- Pyodide compiles CPython 3.11+ to WASM (~15-25MB additional payload)
- Blender's `bpy` C extension module needs WASM compilation against Pyodide's Python headers
- Pyodide includes NumPy and has its own pure-Python package manager
- Emscripten's `-s USE_PTHREADS` is compatible with Pyodide
- Challenge: Blender's Python API uses C extension modules that all need WASM compilation

**Estimated effort:** 3-6 months — complex integration requiring Pyodide-to-bpy bridge

---

## Stage 5: Cycles Rendering

**Status:** Future

### Stage 5a: CPU Cycles in WASM

Enable Cycles with CPU-only rendering compiled to WASM.

- Cycles already has a CPU device in `intern/cycles/device/cpu/`
- Can compile to WASM directly with the existing CPU path tracing kernels
- Performance will be slow (path tracing in WASM) but functional
- Good for previews and proof of concept

**Estimated effort:** 1-2 months

### Stage 5b: WebGPU Cycles

Bring Cycles rendering to the browser using WebGPU compute shaders.

**What this unlocks:**
- GPU-accelerated path-traced rendering in the browser
- Full material preview at interactive speeds
- Production-quality output

**Cycles device architecture** (`intern/cycles/device/`):
- Clean device abstraction with existing backends: CPU, CUDA, HIP, Metal, OneAPI, OptiX
- Would need new `intern/cycles/device/webgpu/` directory
- Implement `DeviceWebGPU` class following the existing device interface pattern
- Port Cycles kernel shaders to WGSL (WebGPU Shading Language)
- WebGPU compute dispatch for path tracing kernels
- Memory management via WebGPU buffer API

**Estimated effort:** 6-12 months — requires WebGPU maturity and kernel shader porting

---

## Stage 6: Collaborative Editing

**Status:** Future / Experimental

Real-time multi-user editing via WebRTC or WebSocket. This is essentially a new product feature, not a port of existing functionality.

**What this unlocks:**
- Multiple users editing the same scene simultaneously
- Shared viewport / shared edit mode
- Real-time collaboration without file exchange

**Existing infrastructure:**
- Blender has NO existing networking or collaboration code
- Undo system (`source/blender/blenkernel/intern/undo_system.cc`) tracks state changes — could serve as a change-tracking foundation
- Depsgraph tracks data dependencies and knows what changed per evaluation

**What this requires (new subsystem):**
1. State synchronization protocol (operational transforms or CRDT for scene graph)
2. Network transport layer (WebRTC for peer-to-peer, WebSocket for server-mediated)
3. Conflict resolution for concurrent mesh/scene edits
4. Cursor/selection awareness (showing other users' selections in viewport)
5. Server infrastructure (signaling server for WebRTC, relay for WebSocket)

**Estimated effort:** 6-12+ months — this is effectively building a new collaborative product on top of the web editor

---

## Recommended Implementation Order

Based on codebase analysis, the optimal order prioritizes quick wins and unlocks:

| Priority | Stage | Description | Effort |
|----------|-------|-------------|--------|
| 1st | **Stage 1** | Core Editor (GPU backend work) | 2-4 months |
| 2nd | **Stage 2** | Audio (Emscripten SDL audio) | 2-4 weeks |
| 3rd | **Stage 3a** | Frame Export (PNG/JPEG download) | ~1 week |
| 4th | **Stage 4** | Python/Pyodide | 3-6 months |
| 5th | **Stage 5a** | CPU Cycles | 1-2 months |
| 6th | **Stage 3b** | Video/ffmpeg.wasm | 2-3 months |
| 7th | **Stage 5b** | WebGPU Cycles | 6-12 months |
| 8th | **Stage 6** | Collaborative Editing | 6-12+ months |

---

## Critical Files Reference

| Area | File | Issue |
|------|------|-------|
| GL version gate | `source/blender/gpu/opengl/gl_backend.cc:223` | Requires GL 4.3; WebGL2 is GL ES 3.0 |
| Compute dispatch | `source/blender/gpu/opengl/gl_compute.cc:25` | `glDispatchCompute` not in WebGL2 |
| SSBOs | `source/blender/gpu/opengl/gl_storage_buffer.cc` | `GL_SHADER_STORAGE_BUFFER` not in WebGL2 |
| Texture views | `source/blender/gpu/opengl/gl_texture.cc` | `glTextureView` not in WebGL2 |
| Subprocess fork | `source/blender/blenlib/intern/BLI_subprocess.cc` | `fork()`/`execv()` not in WASM |
| Draw culling | `source/blender/draw/intern/draw_view.cc:251,299` | Uses compute dispatch |
| Draw commands | `source/blender/draw/intern/draw_command.cc:266,270,841` | Uses compute dispatch |
| Draw manager | `source/blender/draw/intern/draw_manager.cc:140` | Uses compute dispatch |
| Subdivision | `source/blender/draw/intern/draw_cache_impl_subdivision.cc:978` | Uses compute dispatch |
| Eevee shadows | `source/blender/draw/engines/eevee/eevee_shadow.cc:1269` | Uses compute dispatch |
| Workbench shadows | `source/blender/draw/engines/workbench/workbench_shadow.cc:265` | Uses compute dispatch |

---

## Tier System Mapping

Blended's tiered UI is uniquely well-suited for progressive web deployment:

| Tier | Stages Required | Features |
|------|----------------|----------|
| **Simple** | Stage 1 | Basic 3D modeling, sculpting, Eevee rendering |
| **Standard** | Stages 1 + 4 | Full panels, add-ons, Python scripting, tier smart defaults |
| **Advanced** | Stages 1-5 | Everything: Cycles, video, audio, full pipeline |

The "Simple" tier audience gets a fully functional web editor from day one.

---

## Performance Optimization Opportunities

- **WASM SIMD** — Enable 128-bit SIMD instructions for math-heavy operations (mesh transforms, sculpting). Emscripten supports `-msimd128`.
- **Streaming compilation** — Browsers can compile WASM while downloading. Ensure the server sends `Content-Type: application/wasm`.
- **Module splitting** — Split the WASM binary so the core editor loads first, with optional modules loaded on demand.
- **wasm-opt** — Run Binaryen's `wasm-opt -O3` on the output binary for size and speed improvements.
- **Compression** — GitHub Pages serves gzip/brotli automatically. A 100MB WASM binary compresses to ~30-40MB.
- **IndexedDB caching** — Cache the WASM binary in IndexedDB so repeat visits load instantly.
- **Web Workers** — Offload heavy computation (subdivision, boolean operations) to background workers.

---

## Browser Compatibility

| Browser | WebGL2 | SharedArrayBuffer | WASM | WebGPU | Status |
|---------|--------|-------------------|------|--------|--------|
| Chrome 113+ | Yes | Yes | Yes | Yes | Full support (WebGPU path) |
| Firefox 89+ | Yes | Yes | Yes | Nightly | WebGL2 path; WebGPU in nightly |
| Edge 113+ | Yes | Yes | Yes | Yes | Full support (WebGPU path) |
| Safari 18+ | Yes | Yes | Yes | Yes | Full support |
| Safari 16.4-17 | Yes | Yes | Yes | No | WebGL2 path only |
| Mobile Chrome | Yes | Yes | Yes | Partial | Works (performance limited) |
| Mobile Safari | Yes | Partial | Yes | No | May work with limitations |

SharedArrayBuffer requires COOP/COEP headers, handled by `coi-serviceworker.min.js` on GitHub Pages.
