# Blended WebAssembly Roadmap

This document tracks the staged approach to bringing Blended's features to the browser via WebAssembly.

---

## Stage 1: Core Editor (Current)

**Status:** In progress

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

---

## Stage 2: Python Scripting

**Status:** Planned

Restore Python scripting via [Pyodide](https://pyodide.org/) (CPython compiled to WASM) or a lightweight alternative.

**What this unlocks:**
- Add-on support
- Scripted workflows and automation
- Property panels that depend on Python registration
- The "Standard" tier becomes fully functional

**Technical approach:**
- Pyodide provides a near-complete CPython 3.11+ in WASM
- Blender's Python API (bpy) would need to be compiled against Pyodide's Python
- Alternative: [MicroPython](https://micropython.org/) for a lighter-weight option with limited compatibility

---

## Stage 3: Cycles via WebGPU

**Status:** Future

Bring Cycles rendering to the browser using WebGPU compute shaders.

**What this unlocks:**
- Path-traced rendering in the browser
- Full material preview
- Production-quality output

**Technical approach:**
- WebGPU is maturing in Chrome/Firefox/Safari
- Cycles would need a WebGPU compute backend (similar to its Metal/HIP backends)
- Could start with CPU-only Cycles in WASM as an intermediate step

---

## Stage 4: Audio Support

**Status:** Future

Add audio playback and editing via the Web Audio API.

**What this unlocks:**
- Audio scrubbing in the timeline
- Sound strip support in the Video Sequence Editor
- Audio-reactive workflows

**Technical approach:**
- Emscripten has a Web Audio backend for SDL_audio
- Enable `WITH_AUDASPACE` with the Emscripten audio driver
- Relatively straightforward compared to other stages

---

## Stage 5: Video/FFMPEG

**Status:** Future

Add video codec support via [ffmpeg.wasm](https://ffmpegwasm.netlify.app/) or the browser's MediaRecorder API.

**What this unlocks:**
- Video rendering output (MP4, WebM)
- Video Sequence Editor with video playback
- Image sequence rendering

**Technical approach:**
- ffmpeg.wasm provides a full FFMPEG build in WebAssembly
- Alternatively, use MediaRecorder API for WebM output directly
- Could also support frame-by-frame PNG/JPEG export as a simpler first step

---

## Stage 6: Collaborative Editing

**Status:** Future / Experimental

Real-time multi-user editing via WebRTC or WebSocket.

**What this unlocks:**
- Multiple users editing the same scene simultaneously
- Shared viewport / shared edit mode
- Real-time collaboration without file exchange

**Technical approach:**
- Operational transforms or CRDT-based sync for scene data
- WebRTC for peer-to-peer, WebSocket for server-mediated
- Blender's undo system provides a foundation for change tracking

---

## Tier System Mapping

Blended's tiered UI is uniquely well-suited for progressive web deployment:

| Tier | Stage Required | Features |
|------|---------------|----------|
| **Simple** | Stage 1 | Basic 3D modeling, sculpting, Eevee rendering |
| **Standard** | Stage 1 + 2 | Full panels, add-ons, Python scripting |
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

| Browser | WebGL2 | SharedArrayBuffer | WASM | Status |
|---------|--------|-------------------|------|--------|
| Chrome 91+ | Yes | Yes | Yes | Full support |
| Firefox 89+ | Yes | Yes | Yes | Full support |
| Edge 91+ | Yes | Yes | Yes | Full support |
| Safari 16.4+ | Yes | Yes | Yes | Full support |
| Mobile Chrome | Yes | Yes | Yes | Works (performance limited) |
| Mobile Safari | Yes | Partial | Yes | May work with limitations |

SharedArrayBuffer requires COOP/COEP headers, handled by `coi-serviceworker.min.js` on GitHub Pages.
