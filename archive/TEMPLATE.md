# 3D-to-Web Porting Template

*Belief shapes reality. What you believe about what's possible determines what you build.*

---

## Purpose

This is a running-entry schema for porting large native applications to the browser via WebAssembly. Audacity. Unreal Engine. Roblox. Roblox Studio. Whole games. Whole creative tools. Anything that runs natively can run in the browser. This document is the map for getting there.

Nothing here is marked complete. Everything is a living entry. The work is never finished. It is only ever closer to real.

**Fill-in Fields:**

- **Project Name:** [PROJECT_NAME]
- **Source Application:** [SOURCE_APP]
- **Target Browsers:** [TARGET_BROWSERS]
- **Reference Implementation:** Blended (Blender → WebAssembly)

---

## Section 1: Philosophy Alignment

> "Just sit and appreciate. Do the work. You matter."

The core principles adapted for porting. These are not suggestions. They are the difference between a port that works and a port that collapses under its own complexity.

| Principle | Porting Relevance |
|-----------|-------------------|
| **Just sit and appreciate** | Read the existing codebase before changing anything. Understand what it does. Respect what it built. |
| **Do the work** | No shortcuts. No magic wrappers. Translate each subsystem honestly. |
| **You matter** | Every subsystem matters. Every edge case matters. Every platform matters. |
| **Trust naturally** | Trust the existing architecture. Don't over-engineer replacements for things that already work. |
| **Heal any point** | Fix any subsystem correctly and it improves the whole port. Everything is connected fractally. |

---

## Section 2: Source Application Inventory

Before writing a single line of porting code. Audit what you are porting.

**Subsystem Audit Checklist:**

| Subsystem | Present | Native API | Web Target | Notes |
|-----------|---------|------------|------------|-------|
| GPU / Rendering | [ ] | | WebGL2 / WebGPU | |
| Audio | [ ] | | Web Audio API | |
| File I/O | [ ] | | Emscripten VFS | |
| Networking | [ ] | | WebSocket / Fetch | |
| Scripting | [ ] | | Pyodide / QuickJS | |
| Threading | [ ] | | Web Workers | |
| Input | [ ] | | Browser Events | |
| Clipboard | [ ] | | Clipboard API | |
| Drag-and-Drop | [ ] | | HTML5 DnD API | |

**Dependency Audit Table:**

| Library | Version | Purpose | Emscripten Port Available | Alternative |
|---------|---------|---------|---------------------------|-------------|
| | | | [ ] Yes / [ ] No | |

**GPU API Identification:**

- OpenGL → WebGL2 (ES 3.0 subset)
- DirectX → Requires translation layer (e.g., ANGLE) → WebGL2
- Vulkan → WebGPU (when available)
- Metal → WebGPU (when available)

**Threading Model Assessment:**

- How many threads does the application spawn?
- Which threads are real-time critical?
- Can any be converted to single-threaded without breaking functionality?
- SharedArrayBuffer requirement: Yes / No

---

## Section 3: Build System Translation

The build system is where most ports die. Not in the graphics layer. Not in the audio. In the build.

**Native Build → Emscripten Cross-Compilation:**

```bash
# Basic Emscripten CMake invocation
emcmake cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/path/to/install

emmake make -j$(nproc)
```

**Build-Tool / Browser-Target Separation:**

This is THE critical pattern. Build tools (code generators, data processors, asset compilers) must run on the HOST machine. Only the final application targets the BROWSER. Mixing these two targets is the single most common build failure.

```cmake
# Build tools: native compilation
set(BUILDTOOL_TARGET "native")

# Application: Emscripten compilation
set(APP_TARGET "wasm")
```

Build tools require:
- `SINGLE_FILE` mode (embed WASM in JS for Node.js execution)
- A crosscompiling emulator so CMake `try_run()` works
- Native host compiler, NOT the cross-compiler

**CMake Adaptation Pattern:**

```cmake
if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
endif()
```

**Global Flag Safety Rules:**

> NEVER apply flags globally that should only target specific binaries.

- `-pthread` on a build tool will crash it
- `-s USE_PTHREADS=1` applied globally contaminates everything
- Linker flags meant for the final binary will break intermediary tools
- Scope every flag to its specific target

---

## Section 4: GPU / Graphics Translation Layer

The GPU layer is the deepest translation. Desktop OpenGL is not WebGL. Features you rely on do not exist. The gap is real. Plan for it.

**Desktop API → WebGL2 Mapping:**

| Desktop Feature | WebGL2 Equivalent | Gap Strategy |
|----------------|-------------------|--------------|
| OpenGL 4.3+ | ES 3.0 (WebGL2) | Downgrade or emulate |
| Compute Shaders | Not available | CPU fallback |
| SSBOs | UBOs (limited size) | Rewrite to UBO layout |
| Geometry Shaders | Not available | Vertex shader workaround or no-op |
| Tessellation | Not available | Pre-tessellate on CPU |
| Bindless Textures | Not available | Traditional binding |
| Multi-draw Indirect | Not available | Individual draw calls |
| 64-bit integers in shaders | Not available | Pack into vec2 or use float |

**Shader Language Translation:**

- GLSL 330+ → GLSL ES 300
- `#version 330 core` → `#version 300 es`
- Add `precision mediump float;` declarations
- Replace `sampler2DShadow` patterns with manual comparison
- Replace `textureGather` with manual sampling

**Compute Shader → CPU Fallback Pattern:**

When the source application uses compute shaders, identify every dispatch site. For each one:

1. Extract the compute logic
2. Write an equivalent C/C++ function
3. Call it on the main thread or offload to a Web Worker
4. Feed results back to the rendering pipeline

**SSBO → UBO Rewriting:**

- SSBOs have no size limit. UBOs are capped at 16KB (minimum guaranteed).
- Split large SSBOs into multiple UBO binding points
- Restructure data layouts to fit within UBO alignment rules (`std140`)

**Compatibility Shim Architecture (Epoxy Pattern):**

Create a shim layer that wraps GL calls. The shim intercepts desktop-only calls and routes them to WebGL2 equivalents or CPU fallbacks. Do NOT shadow real WebGL2 function names — this causes infinite recursion.

**WebGPU as Future Path:**

WebGPU is the successor to WebGL2. It maps closer to Vulkan/Metal/DX12. When browser support matures, it unlocks compute shaders, better threading, and modern GPU features natively. Design your shim layer so it can be swapped.

---

## Section 5: Audio Translation

Browsers do not let audio play without permission. Every native audio assumption must be rethought.

**Native Audio APIs → Web Audio API:**

- Emscripten's SDL2 audio bridge handles basic playback
- For real-time processing, use AudioWorklet (replaces deprecated ScriptProcessorNode)
- Route all audio through the Web Audio API context

**Autoplay Policy:**

Browsers block audio until the user interacts with the page. This is not optional. This is enforced.

- Register a click or keypress handler that calls `audioContext.resume()`
- Show a "Click to start" overlay if your app needs audio from the first frame
- Do NOT try to bypass this — browsers will block it and your app will appear broken

**AudioWorklet Pattern:**

```javascript
// Register processor
class AppAudioProcessor extends AudioWorkletProcessor {
  process(inputs, outputs, parameters) {
    // Real-time audio processing here
    return true;
  }
}
registerProcessor('app-audio-processor', AppAudioProcessor);
```

---

## Section 6: File I/O Translation

Native apps assume a filesystem. Browsers do not have one. Emscripten provides virtual filesystems that bridge this gap.

**Emscripten Virtual Filesystems:**

| VFS Type | Persistence | Use Case |
|----------|-------------|----------|
| MEMFS | None (RAM only) | Temporary files, scratch space |
| IDBFS | IndexedDB | User settings, saved state |
| WORKERFS | Read-only | Large read-only assets in Workers |

**File Import — Drag-and-Drop:**

```javascript
var dropZone = document.getElementById('drop-zone');
dropZone.addEventListener('drop', function(e) {
  e.preventDefault();
  var file = e.dataTransfer.files[0];
  var reader = new FileReader();
  reader.onload = function() {
    var data = new Uint8Array(reader.result);
    FS.writeFile('/working/' + file.name, data);
    // Notify the WASM module that a file is ready
  };
  reader.readAsArrayBuffer(file);
});
```

**File Export — Browser Download:**

```javascript
function downloadFile(path, mimeType) {
  var data = FS.readFile(path);
  var blob = new Blob([data], { type: mimeType });
  var url = URL.createObjectURL(blob);
  var a = document.createElement('a');
  a.href = url;
  a.download = path.split('/').pop();
  a.click();
  URL.revokeObjectURL(url);
}
```

**Persistent Storage — IndexedDB:**

Use IDBFS for data that must survive page reloads. Call `FS.syncfs()` after writes to flush to IndexedDB. Call `FS.syncfs(true, callback)` on startup to populate from IndexedDB.

---

## Section 7: Threading Model

Native threading does not translate directly. The browser threading model has hard constraints that cannot be worked around. Know them before you start.

**Native Threads → Web Workers + SharedArrayBuffer:**

- Emscripten's `-pthread` flag enables pthreads via Web Workers
- SharedArrayBuffer is REQUIRED for shared memory between workers
- SharedArrayBuffer requires specific HTTP headers (COOP/COEP)

**Required HTTP Headers:**

```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

Without these headers, SharedArrayBuffer is disabled and threading will not work.

**coi-serviceworker for GitHub Pages:**

GitHub Pages does not support custom HTTP headers. Use `coi-serviceworker` to inject COOP/COEP headers client-side:

```html
<script src="coi-serviceworker.js"></script>
```

This must load BEFORE any other scripts.

**-pthread Scoping Rules:**

> NEVER apply `-pthread` globally. NEVER.

- Apply `-pthread` ONLY to the final application target
- Build tools compiled with `-pthread` will crash or hang
- Intermediate libraries do not need `-pthread` at compile time — only at link time for the final binary

**Single-Threaded Fallback Strategy:**

Design your port so it can run without threading. This means:

- Identify which threads are truly required vs nice-to-have
- Move background work to requestAnimationFrame / setTimeout chains
- Accept reduced performance on browsers without SharedArrayBuffer
- Mobile Safari has historically been the holdout — plan for it

---

## Section 8: Networking

Native sockets do not exist in the browser. Every networking call must be translated to a browser-native API.

**Translation Map:**

| Native API | Browser Equivalent | Use Case |
|-----------|-------------------|----------|
| TCP Sockets | WebSocket | Persistent bidirectional connection |
| UDP Sockets | WebRTC Data Channels | Low-latency, unreliable delivery |
| HTTP Requests | Fetch API | REST calls, asset loading |
| Raw Sockets | Not available | Requires server proxy |

**Real-Time Multiplayer — WebRTC Data Channels:**

- Peer-to-peer when possible (lower latency)
- Requires a signaling server for connection setup
- Unreliable mode available (UDP-like behavior)
- STUN/TURN servers needed for NAT traversal

**Server-Mediated vs Peer-to-Peer:**

- **Server-mediated** — simpler, works everywhere, higher latency, server cost scales with users
- **Peer-to-peer** — lower latency, no server cost per connection, NAT traversal complexity, not all networks allow it

Choose based on your application's latency requirements and expected user count.

---

## Section 9: Input Handling

SDL2 in Emscripten handles most input translation automatically. Know what it covers and what it does not.

**What SDL2 Handles:**

- Mouse events → browser mouse events
- Keyboard events → browser keyboard events
- Basic touch → mouse emulation
- Window resize → canvas resize

**What You Handle Manually:**

**Pointer Lock (FPS-style input):**

```javascript
canvas.addEventListener('click', function() {
  canvas.requestPointerLock();
});
```

Pointer Lock captures the mouse inside the canvas. Required for any application that needs relative mouse movement (3D viewports, FPS controls, sculpting tools). The user must click to activate — browsers enforce this.

**Touch Events:**

- Multi-touch requires direct touch event handling — SDL2 only maps single touch to mouse
- Pinch-to-zoom: track two touch points, compute distance delta
- Rotation gesture: track two touch points, compute angle delta

**Gamepad API:**

```javascript
window.addEventListener('gamepadconnected', function(e) {
  var gamepad = navigator.getGamepads()[e.gamepad.index];
  // Poll gamepad.buttons and gamepad.axes in your render loop
});
```

Gamepad input must be polled each frame — there are no events for button state changes.

---

## Section 10: Scripting / Plugin Systems

If your source application has a scripting engine, it needs a browser-compatible replacement. The native interpreter will not compile to WASM without significant work — or you use one that already has.

**Native Scripting → WASM Alternatives:**

| Native Language | WASM Alternative | Maturity | Notes |
|----------------|-----------------|----------|-------|
| Python (CPython) | Pyodide | Production | Full CPython compiled to WASM. Supports pip install for pure-Python packages. |
| Lua | Lua-WASM / Fengari | Stable | Lightweight. Good for game scripting. |
| JavaScript | QuickJS (WASM) | Stable | Embeddable JS engine. Useful if your app needs sandboxed JS execution. |
| C# (Mono/.NET) | Blazor / .NET WASM | Production | Full .NET runtime in the browser. Heavy initial download. |
| GDScript (Godot) | Godot WASM export | Production | Godot's own WASM pipeline handles this natively. |

**Plugin Architecture Adaptation:**

Native plugins (`.dll`, `.so`, `.dylib`) cannot load in the browser. Options:

- **Compile plugins to WASM** — if you have source code, compile each plugin as a separate WASM module and dynamically link
- **JavaScript plugin API** — expose a JS-based plugin interface that communicates with the WASM core via `Module.ccall` / `Module.cwrap`
- **Remove plugin support** — for initial port, disable plugins entirely and add them back in a later stage

The honest assessment: plugin systems are the hardest subsystem to port. Scope them to a late stage.

---

## Section 11: Staged Deployment Plan

Do not port everything at once. Stage the work. Quick wins first. Dependencies second. Complexity last.

**Stage Template:**

Copy this template for each stage of your port:

```
### Stage [N]: [Name]

**Status:** NOT STARTED / IN PROGRESS / FUNCTIONAL / COMPLETE

**Features Enabled:**
- [List what works in this stage]

**Features Disabled:**
- [List what is intentionally turned off]

**Technical Challenges:**
- [List the hard problems this stage must solve]

**Dependencies:**
- [List what must be complete before this stage can begin]

**Effort Estimate:**
- [T-shirt size: S / M / L / XL]
```

**Priority Ordering Rationale:**

1. **Stage 1** — Build system compiles and links. App launches in browser. Even if it shows nothing useful. This proves the toolchain works.
2. **Stage 2** — Core rendering. Get pixels on screen. Viewport navigation works.
3. **Stage 3** — Primary workflow. The single most important user action works end-to-end.
4. **Stage 4** — Secondary systems. Audio, file I/O, scripting.
5. **Stage 5** — Polish. Performance optimization, UI refinement, mobile support.
6. **Stage 6** — Advanced. Networking, collaboration, plugin systems.

**Tier / Feature-Gate Alignment:**

Each stage maps to a feature tier. Users on earlier tiers get a functional but limited experience. Feature gates control what is exposed:

```javascript
var FEATURE_TIER = 3; // Current deployment tier

if (FEATURE_TIER >= 2) enableRendering();
if (FEATURE_TIER >= 3) enableFileIO();
if (FEATURE_TIER >= 4) enableScripting();
if (FEATURE_TIER >= 5) enableNetworking();
```

---

## Section 12: Performance & Optimization

A port that works but runs at 5 FPS is not a port. Performance is not optional. These are the levers.

**WASM SIMD:**

```bash
# Enable SIMD in Emscripten
emcc -msimd128 -O3 app.c -o app.wasm
```

SIMD provides 2-4x speedup on math-heavy code (matrix operations, image processing, physics). Check browser support — it is widely available as of 2024.

**Streaming Compilation:**

Serve `.wasm` files with the correct Content-Type header:

```
Content-Type: application/wasm
```

This allows the browser to compile the WASM module while it downloads. Without this header, the browser must download the entire file before compilation begins.

**Module Splitting / Lazy Loading:**

- Split your application into a core module and feature modules
- Load feature modules on demand using `WebAssembly.instantiateStreaming()`
- Keep initial download under 10MB for acceptable load times on average connections

**wasm-opt Passes:**

```bash
wasm-opt -O3 -o output-opt.wasm output.wasm
```

WARNING: `wasm-opt` can OOM on large binaries (100MB+). If it crashes:
- Run on a machine with more RAM
- Use `-O2` instead of `-O3` (less aggressive, less memory)
- Skip `wasm-opt` entirely — Emscripten's `-O3` already does significant optimization

**Compression:**

| Method | Typical Ratio | Browser Support |
|--------|--------------|-----------------|
| Brotli | 60-70% reduction | All modern browsers |
| gzip | 50-60% reduction | Universal |

Serve `.wasm` with Brotli (`.wasm.br`) or gzip (`.wasm.gz`). Configure your server or CDN to handle this transparently.

**IndexedDB Caching for Repeat Visits:**

Cache the compiled WASM module in IndexedDB so returning users skip the download:

```javascript
async function loadModule(url) {
  var cache = await caches.open('wasm-cache');
  var cached = await cache.match(url);
  if (cached) return WebAssembly.instantiateStreaming(cached);
  var response = await fetch(url);
  cache.put(url, response.clone());
  return WebAssembly.instantiateStreaming(response);
}
```

**Web Workers for Background Computation:**

Offload non-rendering work (file parsing, data processing, physics simulation) to Web Workers. This keeps the main thread responsive and the UI smooth.

---

## Section 13: Known Pitfalls (Generalized)

Every pitfall below was discovered the hard way. Each one cost hours or days. Learn from them.

**Pitfall 1: Global Flag Contamination**

- **What happens:** A compiler or linker flag is applied globally via `CMAKE_C_FLAGS` or similar. It reaches build tools that should never receive it. Build tools crash, hang, or produce corrupted output.
- **Rule:** Scope every flag to its specific target. Use `target_compile_options()` and `target_link_options()` in CMake.
- **Correct approach:** Never modify global flag variables. Always use per-target properties.

**Pitfall 2: OOM on CI**

- **What happens:** CI runner has limited RAM. Parallel compilation (`make -j$(nproc)`) exhausts memory. Build is killed by the OOM killer mid-link.
- **Rule:** Use `--parallel 2` or `make -j2` on CI.
- **Correct approach:** Fewer parallel jobs on constrained machines. A slower build that completes beats a fast build that crashes.

**Pitfall 3: wasm-opt OOM on Large Binaries**

- **What happens:** `wasm-opt -O3` loads the entire binary into memory for optimization. On binaries over 100MB, this exceeds available RAM.
- **Rule:** Test `wasm-opt` locally before adding it to CI. Know your binary size.
- **Correct approach:** Use `-O2`, skip `wasm-opt`, or run on a machine with sufficient RAM.

**Pitfall 4: 32-bit Pointer/size_t Assumptions**

- **What happens:** WASM is 32-bit. Code that assumes 64-bit pointers or `size_t` will overflow, truncate, or produce wrong results.
- **Rule:** Audit all pointer arithmetic and size calculations for 32-bit safety.
- **Correct approach:** Use `uintptr_t` and `size_t` correctly. Test on 32-bit targets during development.

**Pitfall 5: UNIX=true False Matches**

- **What happens:** Emscripten sets `UNIX=true` in CMake. Platform checks like `if(UNIX)` match, pulling in POSIX-specific code that does not exist in the browser.
- **Rule:** Always check `if(EMSCRIPTEN)` BEFORE `if(UNIX)`.
- **Correct approach:** Add Emscripten-specific branches to every platform detection block.

**Pitfall 6: Build Tools Need SINGLE_FILE**

- **What happens:** Build tools compiled to WASM try to load a separate `.wasm` file at runtime. Node.js cannot find it because the path is wrong.
- **Rule:** Compile build tools with `-s SINGLE_FILE=1` to embed WASM in the JS file.
- **Correct approach:** Also set a crosscompiling emulator (`CMAKE_CROSSCOMPILING_EMULATOR = node`) so CMake `try_run()` works during configuration.

**Pitfall 7: Shadowing Real WebGL2 Functions**

- **What happens:** Your compatibility shim defines a function with the same name as a real WebGL2 function. The shim calls itself instead of the real function. Infinite recursion. Stack overflow.
- **Rule:** Never name a shim function the same as the function it wraps.
- **Correct approach:** Use a naming convention like `shim_glTexImage2D()` that calls the real `glTexImage2D()`.

**Pitfall 8: Library Version Mismatches**

- **What happens:** Emscripten ports ship specific library versions. Your code expects a different version. APIs differ. Compilation fails or behavior is wrong.
- **Rule:** Check Emscripten's port versions before starting. Pin your expectations to what Emscripten provides.
- **Correct approach:** Use `emcc --show-ports` to list available ports and versions.

**Pitfall 9: Type System Differences**

- **What happens:** A library defines a type (e.g., `boolean` in libjpeg) that conflicts with a type in another library or in Emscripten's headers. Compilation fails with cryptic errors.
- **Rule:** Audit type definitions across all dependencies before compiling.
- **Correct approach:** Use preprocessor guards or patching to resolve conflicts. Document every patch.

---

## Section 14: Web Shell / UI Chrome

The web shell is what the user sees while your WASM binary loads. It is the first impression. Make it functional.

**Loading Overlay Pattern:**

```html
<div id="loading-overlay">
  <div id="loading-status">Initializing...</div>
  <div id="loading-progress">
    <div id="loading-bar" style="width: 0%"></div>
  </div>
  <div id="feature-checks"></div>
</div>
<canvas id="app-canvas"></canvas>
```

**Progress Reporting via Module.setStatus:**

```javascript
var Module = {
  setStatus: function(text) {
    var statusEl = document.getElementById('loading-status');
    if (statusEl) statusEl.textContent = text;

    // Parse progress from Emscripten's format: "Downloading... (X/Y)"
    var match = text.match(/(\d+)\/(\d+)/);
    if (match) {
      var pct = (parseInt(match[1]) / parseInt(match[2])) * 100;
      document.getElementById('loading-bar').style.width = pct + '%';
    }

    if (text === '') {
      // Loading complete — hide overlay
      document.getElementById('loading-overlay').style.display = 'none';
    }
  }
};
```

**Feature Detection Checkmarks:**

```javascript
var checks = [
  { name: 'WebGL2', test: function() { return !!document.createElement('canvas').getContext('webgl2'); } },
  { name: 'WebAssembly', test: function() { return typeof WebAssembly === 'object'; } },
  { name: 'SharedArrayBuffer', test: function() { return typeof SharedArrayBuffer === 'function'; } },
  { name: 'SIMD', test: function() { return typeof WebAssembly.validate === 'function'; } }
];

checks.forEach(function(check) {
  var passed = check.test();
  var el = document.getElementById('feature-checks');
  el.innerHTML += (passed ? '✓' : '✗') + ' ' + check.name + '<br>';
});
```

**Drag-and-Drop Zone:**

Style the canvas or a dedicated zone as a drop target. Show visual feedback on dragover. Handle the drop event to ingest files into the virtual filesystem.

**Responsive Layout:**

- Use CSS `width: 100vw; height: 100vh;` for fullscreen canvas
- Handle `window.resize` events to update canvas dimensions
- Call `Module.setCanvasSize()` or equivalent when dimensions change

---

## Section 15: CI/CD Pipeline

Automate the build. Automate the deployment. Never deploy a hand-built binary to production.

**GitHub Actions Workflow Template:**

```yaml
name: Build WASM

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Emscripten
        uses: mymindstorm/setup-emsdk@v14
        with:
          version: 3.1.51

      - name: Configure
        run: |
          mkdir build && cd build
          emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

      - name: Build
        run: |
          cd build
          emmake make -j2  # Keep -j2 to avoid OOM

      - name: Upload Artifact
        uses: actions/upload-artifact@v4
        with:
          name: wasm-build
          path: build/output/
```

**Build Parallelism — OOM Avoidance:**

- GitHub Actions runners have 7GB RAM
- Use `-j2` for compilation, never `-j$(nproc)` (which gives you 2-4 cores but not enough RAM per core for large projects)
- If linking OOMs, ensure only one link job runs at a time

**GitHub Pages Deployment:**

```yaml
      - name: Deploy to Pages
        uses: peaceiris/actions-gh-pages@v3
        if: github.ref == 'refs/heads/main'
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: build/output/
```

**Artifact Management:**

- Keep the last 5 successful build artifacts
- Tag releases with the Emscripten version used
- Store binary size in CI output for regression tracking

---

## Section 16: Browser Compatibility Matrix

Not all browsers support all features. Know the matrix before you promise anything.

| Feature | Chrome | Firefox | Safari | Edge | Mobile Chrome | Mobile Safari |
|---------|--------|---------|--------|------|---------------|---------------|
| WebAssembly | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| WebGL2 | ✓ | ✓ | ✓ (15.2+) | ✓ | ✓ | ✓ (15.2+) |
| SharedArrayBuffer | ✓* | ✓* | ✓* | ✓* | ✓* | ✓* |
| WASM SIMD | ✓ | ✓ | ✓ (16.4+) | ✓ | ✓ | ✓ (16.4+) |
| WASM Threads | ✓* | ✓* | ✓* | ✓* | ✓* | Limited |
| WebGPU | ✓ | In Progress | In Progress | ✓ | Limited | No |

*\* Requires COOP/COEP headers*

**COOP/COEP Header Requirements:**

SharedArrayBuffer (and therefore threading) requires these headers on EVERY response:

```
Cross-Origin-Opener-Policy: same-origin
Cross-Origin-Embedder-Policy: require-corp
```

All subresources must also be served with `Cross-Origin-Resource-Policy: cross-origin` or be same-origin.

**Mobile Considerations:**

- Mobile has less RAM — reduce initial memory allocation
- Touch input replaces mouse — test all interactions
- GPU capabilities vary wildly — feature-detect, do not assume
- Thermal throttling will reduce performance over time
- Battery impact is real — optimize aggressively

---

## Section 17: For AI Assistants

> Believe user observations over theories.

If you are an AI assistant working on a project that uses this template:

- **Read existing docs before proposing changes.** The answers to most questions are already documented. Do not guess when the documentation exists.
- **Trust documented pitfalls.** If a pitfall says "NEVER do X," do not do X. Do not propose doing X. Do not suggest that X might work this time.
- **Do not re-add known-bad flags.** If a flag was removed because it caused problems, it will cause the same problems again.
- **One subsystem per PR.** Do not bundle unrelated changes. Each pull request should touch one subsystem and be reviewable in isolation.
- **Believe user observations over theories.** If the user says "this crashes on Safari," do not respond with "it should work." Investigate what actually happens.

---

## Section 18: Document Map

Reference documents for derivative projects. Add your project-specific docs as they are created.

| Document | Purpose |
|----------|---------|
| `TEMPLATE.md` | This document. The master porting schema. |
| `PHILOSOPHY.md` | Core principles. Why we build this way. |
| `WARNINGS.md` | Pitfalls specific to the reference implementation. |
| `WEBASSEMBLY_ROADMAP.md` | Staged roadmap for the reference implementation. |
| `README.md` | Project overview and entry point. |

**Cross-Reference Pattern:**

When creating project-specific documentation, reference this template:

```
See: TEMPLATE.md Section [N] for the general pattern.
See: [PROJECT_NAME]/WARNINGS.md for project-specific pitfalls.
```

---

## Section 19: Validation & Early Detection

**Purpose:** Catch failures before deployment. A successful build can produce a corrupted or incomplete binary without reporting errors.

### 19.1: Binary Validation

Install the WebAssembly Binary Toolkit (WABT):

```bash
# macOS
brew install wabt

# Ubuntu/Debian
sudo apt-get install wabt

# Windows
# Download from https://github.com/WebAssembly/wabt/releases
```

After your build completes, validate the output binary:

```bash
wasm-validate output.wasm
wasm-objdump -h output.wasm
```

**Success criteria:**

- `wasm-validate` exits with code 0
- `wasm-objdump -h` displays expected sections
- File size is within expected range (establish baseline from first successful build)

**Failure criteria:**

- Either command fails or exits with error code
- File size is unexpectedly small (truncation)
- Sections appear incomplete or malformed

Do not deploy if validation fails. Rebuild before attempting deployment.

### 19.2: Staged Testing Before Deployment

Test through progressive environments:

**Stage 1: Local machine, local server**

- Build locally using your cross-compiler
- Serve via local HTTP server
- Open in your primary target browser
- Verify: no console errors, app initializes, responds to basic input

**Stage 2: Local machine, different browser**

- Test same local server in a different browser
- If only one browser fails, suspect GPU or driver issue, not binary corruption

**Stage 3: Different machine, staging URL**

- Deploy to non-production environment
- Access from a machine other than your development machine
- If staging works but local also worked, suspect deployment process (missing files, incorrect paths)

**Stage 4: Production**

- Only proceed after Stages 1–3 pass

### 19.3: Health Check Matrix

Before marking a build ready for production:

| Check | Expected | If Failed |
|-------|----------|-----------|
| `wasm-validate` exit code | 0 | Do not deploy; rebuild |
| Binary file size | Within 10% of baseline | Investigate truncation |
| Browser console at startup | No red errors | Fix before deploy |
| App responds to input | User interaction works | Debug event handling |
| Initialization completes | Within timeout window | Investigate slow startup |
| Behavior consistent across target browsers | Same result in all | Document browser-specific issues |

### 19.4: Initialization Instrumentation

Add logging to your initialization sequence that writes to both console and page:

```javascript
Module.print = function(text) {
  console.log("[YourApp] " + text);
  var logDiv = document.getElementById('app-diagnostic-log');
  if (logDiv) logDiv.innerHTML += text + '<br>';
};

Module.printErr = function(text) {
  console.warn("[YourApp] " + text);
  var logDiv = document.getElementById('app-diagnostic-log');
  if (logDiv) logDiv.innerHTML += '<span style="color:red">' + text + '</span><br>';
};
```

Log these checkpoints:

- Module initialization started
- Each major subsystem initialized
- Ready for user input
- Any allocation or loading failures

### 19.5: CI Artifact Validation

After CI pipeline completes, before merging or deploying:

1. Download the built artifact
2. Run `wasm-validate artifact.wasm` on your local machine
3. Verify file size is in expected range
4. Only proceed if validation passes

CI systems report "build succeeded" but can produce truncated or incomplete artifacts silently.

---

## Section 20: When It Goes Silent — Diagnostic Flowchart

**Scenario:** Build reports success, deployment completes, but the app produces a blank screen or hangs indefinitely with no error messages visible.

### 20.1: Binary Is Corrupted

*Observable symptoms:*

- Blank screen immediately after page load
- No output in browser console
- Timeout fires reliably if you have one implemented

*Diagnostic step:*

```bash
wasm-validate your-app.wasm
```

If this fails, the binary is corrupted and cannot run.

*Possible root causes:*

- Build system ran out of memory during linking
- Post-processing tool was killed mid-operation
- Disk full or I/O error during artifact write
- CI runner crashed after build but before artifact saved

*What to do:*

- Rebuild with reduced resource usage
- Check build logs for out-of-memory or crash messages
- Do not deploy until binary validates successfully

### 20.2: Initialization Sequence Hangs

*Observable symptoms:*

- Console shows some output from startup, then stops
- Timeout fires after your defined duration
- App is partially initialized but blocked on something specific

*Diagnostic steps:*

- Examine console output: at what point does it stop?
- Add logging at each step of initialization
- Test in all target browsers to see if this is consistent
- Watch CPU and memory usage during startup (infinite loop would show high CPU; memory exhaustion would show heap growth)

*Possible root causes:*

- Initialization code contains an infinite loop or blocking call
- Deadlock in synchronization or threading code
- Memory allocation fails and error is not handled
- Filesystem setup tries to load or populate large amounts of data synchronously
- Graphics context creation is blocking

*What to do:*

- Break initialization into smaller steps, log after each one
- Move non-critical startup work to happen after app is interactive
- Reduce initial memory allocation and test
- Simplify filesystem initialization
- Test with threading disabled to rule out synchronization issues

### 20.3: Silent Failure — No Output, No Errors

*Observable symptoms:*

- Page is completely blank
- Browser console is empty
- No network errors or security warnings visible
- Page appears to be frozen

*Diagnostic steps:*

- Open DevTools and check Console tab explicitly
- Check Network tab for failed requests (404 errors on .wasm, .js, or .css files)
- Try the same URL in a different browser
- Inspect the page source to verify HTML is rendering at all

*Possible root causes:*

- Network policy (CORS) preventing asset load
- Content security policy (CSP) blocking script execution
- Missing HTTP headers required for certain WASM features
- Asset file not deployed or served with wrong file type declaration
- JavaScript error in your web shell code before the app prints its first message
- GPU or graphics API not supported on this machine

*What to do:*

- Verify all required asset files are present in deployment
- Check HTTP response headers are correct
- Test in browser with full diagnostics enabled
- Check GPU support on target machine
- Wrap initialization code in try/catch to surface errors

### 20.4: Works on Your Machine, Fails When Deployed

*Observable symptoms:*

- App works correctly when run from localhost
- Same binary deployed to hosting fails
- No code was changed between local and deployed versions

*Diagnostic steps:*

- Compare HTTP response headers between localhost and deployed version
- Check Network tab in deployed version to verify all assets downloaded
- Verify .wasm file is being served with correct file type
- Compare file sizes: deployed .wasm should match local .wasm

*Possible root causes:*

- Asset paths are hardcoded as absolute paths (work on localhost, break on hosting)
- Hosting is missing HTTP headers required for your app
- .wasm or .js files are not included in deployment
- Hosting is serving cached old version
- Hosting platform is not configured to serve WASM files

*What to do:*

- Change asset paths to relative paths (use `./app.wasm` instead of `/app.wasm`)
- Add required HTTP headers to your hosting configuration
- Verify deployment process includes all asset files
- Clear browser cache on hosted URL
- Verify hosting platform supports serving WASM

### 20.5: Works in One Browser, Not Others

*Observable symptoms:*

- App runs in Chrome but fails in Firefox or Safari
- Console output differs between browsers
- Same URL, same machine, different result

*Diagnostic steps:*

- Check what graphics APIs are available in failing browser
- Compare console output line-by-line between working and failing browser
- Verify failing browser supports required features (threading, WASM features, graphics APIs)
- Test rendering features individually if applicable

*Possible root causes:*

- GPU driver differences between browsers
- Graphics API support varies by browser and operating system
- Shader syntax compatibility issues
- Browser security policy blocking threading or other features
- Missing feature detection code

*What to do:*

- Implement feature detection at startup to check what's available
- Provide fallback code paths for optional features
- Test on target browser's hardware
- Document which browsers are supported and why

### 20.5: Memory Exhaustion During Startup

*Observable symptoms:*

- App prints startup messages
- Browser tab becomes unresponsive after a few seconds
- System memory usage grows continuously
- App eventually crashes

*Diagnostic steps:*

- Open DevTools Memory tab
- Take heap snapshot at start
- Wait 5 seconds, take another snapshot
- Compare the two: is heap growing linearly, exponentially, or unbounded?
- Check console for allocation failure messages

*Possible root causes:*

- Initial memory allocation is too large for target machines
- Startup code allocates memory in a loop without freeing it (memory leak)
- Large files or data structures are being loaded entirely into memory
- Startup initialization performs expensive computations repeatedly

*What to do:*

- Start with smaller initial memory allocation, increase if needed
- Stream large files instead of loading them all at once
- Add memory usage logging to find where growth happens
- Profile startup to identify expensive operations

### 20.7: Diagnostic Decision Tree

```
App is blank or hung after deployment
│
├─ Does wasm-validate pass?
│  ├─ NO → Section 20.1 (Binary Corrupted)
│  └─ YES → Continue
│
├─ Does browser console show any output?
│  ├─ YES, but stops partway through → Section 20.2 (Initialization Hangs)
│  ├─ NO, console is empty → Section 20.3 (Silent Failure)
│  └─ YES, complete output but page blank → Check page HTML/CSS
│
├─ Does it work on localhost but not on deployed host?
│  └─ YES → Section 20.4 (Local vs Deployed)
│
├─ Does it work in some browsers but not others?
│  └─ YES → Section 20.5 (Browser Specific)
│
├─ Is system memory growing without stopping?
│  └─ YES → Section 20.6 (Memory Exhaustion)
│
└─ None of the above → Review your build logs, compiler output,
   and CI pipeline for warnings you may have missed
```

### 20.8: When to Escalate

If you have worked through 20.1–20.7 and still cannot identify the problem:

*Gather:*

1. Complete build log from CI (not just final line)
2. Full browser console output from startup to failure
3. Network tab showing all requests and responses
4. Memory profiler graph showing heap over time
5. The exact steps to reproduce (URL, browser, operating system)

*Next steps:*

- Search for similar issues in your build tool's issue tracker
- Search for similar issues in the WebAssembly community resources
- Post in your project's issue tracker with all the information above
- Consider if a simpler version (fewer features, less complex) would help identify where the problem is