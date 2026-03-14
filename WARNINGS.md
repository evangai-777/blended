# Warnings Triage Plan

Post-build warning cleanup tracker for the Blended codebase. Once the
WebAssembly build compiles and links, use this plan to systematically
eliminate compiler warnings rather than suppressing them.

## Current State

The Emscripten build blanket-suppresses four warning categories in
`build_files/cmake/platform/platform_emscripten.cmake` (lines 41-42):

| Flag | Why it's suppressed | Estimated count |
|------|--------------------|----|
| `-Wno-sign-conversion` | 20,000+ implicit signed/unsigned conversions | massive |
| `-Wno-shorten-64-to-32` | 64-to-32-bit truncations (WASM is 32-bit) | large |
| `-Wno-implicit-int-conversion` | Implicit integer narrowing | large |
| `-Wno-c++11-narrowing` | Brace-init narrowing (uchar->char, int->float) | moderate |

These are not bugs introduced by Blended — they're inherited from
Blender's codebase and are mostly intentional. But they mask real
issues and should be fixed properly.

### Existing Warning Suppressions in Source

| Location | Count | Type |
|----------|-------|------|
| `source/blender/freestyle/intern/python/` | 120+ pragma lines | `-Wcast-function-type` (Python method casts) |
| `source/blender/blenlib/BLI_math_*.h` | 30 pragma lines | `-Wredundant-decls` (inline math) |
| `source/blender/blenlib/BLI_strict_flags.h` | 8 pragma lines | Intentional — enables strict mode |
| `source/blender/gpu/` (Metal) | 5 pragma lines | Metal/Obj-C specific |
| `source/blender/simulation/intern/eigen_utils.h` | 2 pragma lines | `-Wlogical-op` (Eigen) |
| NOLINT comments | 1 | Test file |

## How to Run a Warning Audit

### 1. Build with warning capture

```sh
# Configure with warnings report enabled
emcmake cmake -S . -B build-wasm \
  -C build_files/cmake/config/blended_wasm.cmake \
  -DWITH_WARNINGS_REPORT=ON

# Build and capture all output
emmake cmake --build build-wasm 2>&1 | tee warnings.log
```

### 2. Parse the log

```sh
# Full summary report
python3 build_files/utils/parse_warnings.py warnings.log

# Filter to GPU backend only
python3 build_files/utils/parse_warnings.py warnings.log --filter gpu

# Export to CSV for spreadsheet triage
python3 build_files/utils/parse_warnings.py warnings.log --csv > warnings.csv
```

### 3. Read the report

The parser groups warnings by:
- **Warning type** — which `-W` flag, ranked by count
- **Subsystem** — which `source/blender/*` directory
- **Top files** — worst offenders

## Fix Priority

Address in this order once the web build links:

### Phase 1: GPU / WebGL2 backend (highest impact)
- `source/blender/gpu/opengl/` — 6 files with Emscripten concerns
- `source/blender/gpu/intern/` — shader create info
- Focus: sign conversions, narrowing in GL calls, format mismatches
- **Status:** Epoxy shim now covers all desktop GL functions and constants
  used across the full `gpu/opengl/` backend (gl_state, gl_texture,
  gl_compute, gl_storage_buffer, gl_uniform_buffer, etc.). The code
  compiles against WebGL2 via no-op stubs. Remaining work is runtime
  correctness — e.g. `glGetTexImage` stub returns no data, `glMapBuffer`
  returns NULL, compute dispatch is a no-op.

### Phase 2: Draw engine + GLSL shaders
- `source/blender/draw/` — passes data to GPU, same warning classes
- Focus: implicit conversions in draw call setup
- **Note:** The draw engine was not fully audited for WebGL2 gaps, but it
  primarily passes data to the GPU layer which is now stubbed. Shaders
  need GLSL version/extension changes (runtime issue, not compilation) —
  e.g. hardcoded `#version 430`, `sampler1D`/`sampler1DArray` usage,
  compute shader dispatches. These are runtime fixes, not link errors.

### Phase 3: Core libraries
- `source/blender/blenlib/` — math headers already have some pragmas
- `source/blender/blenkernel/` — large, but mostly safe conversions
- Focus: replace pragmas with proper casts where appropriate

### Phase 4: Freestyle Python bindings
- `source/blender/freestyle/intern/python/` — 120+ `-Wcast-function-type`
- These are Python API casts, mostly boilerplate. Fix with proper
  `reinterpret_cast` or `PyCFunction` wrappers.
- Low priority — Freestyle is disabled in the WASM build anyway.

### Phase 5: Remove blanket suppressions
- Once per-file fixes reach critical mass, remove the `-Wno-*` flags
  from `platform_emscripten.cmake` one at a time
- Remove `-Wno-c++11-narrowing` first (smallest count)
- Remove `-Wno-sign-conversion` last (largest count)

## Rules

- **Fix, don't suppress.** Use explicit casts (`static_cast<>`, `int(x)`)
  where the conversion is intentional.
- **Don't add `(void)` casts** for unused parameters in Blended code —
  use `[[maybe_unused]]` or remove the parameter name.
- **Don't touch extern/.** Third-party code gets pragmas, not fixes.
- **Don't fix warnings in code paths disabled for WASM** (Cycles,
  Python, FFMPEG, etc.) — those are Blender's problem.
- **Each PR should remove warnings from one subsystem**, not scatter
  fixes across the whole tree.

## Files

| File | Purpose |
|------|---------|
| `build_files/cmake/warnings_report.cmake` | CMake option to enable warning capture |
| `build_files/utils/parse_warnings.py` | Log parser and report generator |
| `build_files/cmake/platform/platform_emscripten.cmake` | Where blanket suppressions live |
| `build_files/cmake/config/blended_wasm.cmake` | WASM build configuration |
