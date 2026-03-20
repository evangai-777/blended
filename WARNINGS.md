# Warnings Triage Plan

Post-build warning cleanup tracker for the Blended codebase. Once the
WebAssembly build compiles and links, use this plan to systematically
eliminate compiler warnings rather than suppressing them.

> *"Just do the work. Whatever work is in front of you. That's it."*
> — [PHILOSOPHY.md](PHILOSOPHY.md)

**The approach is simple:** open the file, find the implicit conversion, add
the explicit cast, move to the next line. Don't build a framework. Don't
write a codemod. Don't debate taxonomy. Heal one subsystem at a time and let
the pattern cascade fractally through the rest of the codebase.

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

> For WebGL2 compatibility context (SSBO→UBO rewriting, sampler1DArray emulation,
> compute/geometry shader guards), see the
> [WEBASSEMBLY_ROADMAP.md Critical Technical Challenges](build_files/web/WEBASSEMBLY_ROADMAP.md#critical-technical-challenges--solutions)
> section. This phase focuses on fixing compiler warnings in those implementations.

- `source/blender/gpu/opengl/` — 6 files with Emscripten concerns
- `source/blender/gpu/intern/` — shader create info
- Focus: sign conversions, narrowing in GL calls, format mismatches
- **Status:** Epoxy shim now covers all desktop GL functions and constants
  used across the full `gpu/opengl/` backend (gl_state, gl_texture,
  gl_compute, gl_storage_buffer, gl_uniform_buffer, etc.). The code
  compiles against WebGL2 via no-op stubs. Runtime correctness is in
  progress — SSBO→UBO shader rewriting (`gl_shader.cc`), GL capability
  query guards (`gl_backend.cc`), geometry/compute shader stage guards,
  and `sampler1DArray`→`sampler2D` emulation are all complete. Remaining
  stubs: `glGetTexImage` returns no data, `glMapBuffer` returns NULL.
- **Warning fixes (in progress):**
  - Fixed `uint16_t |= 1 << slot` sign-conversion warnings across
    `gl_storage_buffer.cc`, `gl_uniform_buffer.cc`, `gl_immediate.cc`,
    `gl_index_buffer.cc`, `gl_vertex_buffer.cc` (bit-shift to `uint16_t`)
  - Fixed `uint8_t |= 1ULL << unit` narrowing in `gl_state.cc`
    image bind tracking
  - Fixed `GLenum + int` sign-conversion in cube map face calculations
    in `gl_texture.cc` and `gl_framebuffer.cc`
  - Relaxed GL 4.3 version gate in `gl_backend.cc` for Emscripten
    (WebGL2 is GL ES 3.0; compute/SSBO gaps handled separately)

### Phase 2: Draw engine + GLSL shaders

> WebGL2 draw engine solutions (GLSL `#version 300 es` patching, compute dispatch
> guards, CPU command fallback) are documented in the
> [WEBASSEMBLY_ROADMAP.md](build_files/web/WEBASSEMBLY_ROADMAP.md). This phase
> focuses on compiler warning fixes in those code paths.

- `source/blender/draw/` — passes data to GPU, same warning classes
- Focus: implicit conversions in draw call setup
- **Note:** The draw engine's WebGL2 gaps are largely resolved. GLSL
  `#version 300 es` patching, `sampler1DArray`→`sampler2D` emulation (via
  `tex1DArrayLookup()` in the compat layer), and compute dispatch guards
  are all complete. Remaining runtime concerns: texture readback stubs and
  edge cases in draw call ordering.
- **Warning fixes (in progress):**
  - Fixed `int → uint` sign-conversion in `GPU_compute_dispatch()` calls
    across `draw_view.cc`, `draw_command.cc`, `draw_manager.cc`,
    `draw_cache_impl_subdivision.cc`, `eevee_shadow.cc`, `workbench_shadow.cc`
    (literal `1` → `1u`, `int3` → `uint()` casts)
  - Fixed `float → uint` implicit conversion in subdivision dispatch
    size calculation (`draw_cache_impl_subdivision.cc`)
- **GLSL shader compatibility (done):**
  - `sampler1DArray`→`sampler2D` emulation in `gpu_shader_compat_glsl.glsl`
    with `tex1DArrayLookup()` helper for `texture()` calls; `texelFetch()`
    unchanged (integer coords map identically to sampler2D)
  - All 7 affected shaders updated: color ramp, curves, wavelength,
    blackbody, hue correct, plus the GLSL compat layer and texture backend
  - SSBO→UBO shader rewriting in `gl_shader.cc`: `buffer`→`uniform`,
    `std430`→`std140`, access qualifiers stripped
  - Geometry and compute shader stage creation skipped on Emscripten
  - GL capability queries guarded for unsupported constants in `gl_backend.cc`
- **WebGL2 compute fallback (done):**
  - Added `#ifdef __EMSCRIPTEN__` guards around ALL compute dispatch sites
    in the draw engine (6 locations across 6 files)
  - **Visibility culling:** Skipped on Emscripten; buffer initialized to
    all-visible (0xFFFFFFFF), so all objects render without frustum culling
  - **Resource finalization:** Skipped on Emscripten; bounds stay local-space
    (acceptable since culling is disabled), ORCO coords uninitialized
  - **Command generation:** Full CPU fallback implemented in
    `draw_command.cc` — generates DrawCommands and resource IDs on CPU,
    respecting front-facing/back-facing handedness sorting
  - **Subdivision compute:** Skipped on Emscripten; meshes render without
    GPU-based subdivision refinement
  - **Shadow visibility:** Skipped on Emscripten for both Eevee and
    Workbench engines; all shadow casters treated as visible

### Phase 3: Core libraries
- `source/blender/blenlib/` — math headers already have some pragmas
- `source/blender/blenkernel/` — large, but mostly safe conversions
- Focus: replace pragmas with proper casts where appropriate
- **Warning fixes (in progress):**
  - Fixed `(N << 2)` signed-to-unsigned assignment in `noise.cc` hash
    functions (4 instances, `uint32_t` targets) — use `Nu << 2`
  - Fixed `(1 << oct)` in `noise_c.cc` turbulence normalization — `1u`
  - Fixed `int flag |= (1 << i)` pattern in `math_matrix_c.cc`
    `orthogonalize_m3_zero_axes_impl()` — use `uint` flag type and `1u`
  - Fixed `(1 << int(domain)) & domain_mask` in `attribute.cc` (3
    instances) — use `1u` for unsigned bitmask comparison

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

## Known Pitfalls (read this first!)

> **This is engineering, not philosophy.**
>
> We are applying engineering principles to coding. In philosophy you can
> deal with ego and debate and whether something is "actually right" or
> "appears right," so that someone can have credibility. Here we are
> dealing with **functionality** — whether something works or doesn't work
> the way we intend. That's it. We are not dealing with ego. We are
> dealing with whether we can get a Blender fork into a browser. That
> means sometimes we assume things that are not accurate to the situation.
> And that's okay. It just means we adjust, correct ourselves, and move
> forward.
>
> Authentically good work comes from doing the work well — not from
> trying to convince people that it is good. We aren't trying to convince
> each other that we are doing good work; we are doing it and seeing the
> results. This inadvertently establishes us as excellent developers
> without needing ego badges. (This is a good life philosophy as well as
> an engineering one.)
>
> Every pitfall below was discovered the hard way — by chasing the wrong
> theory multiple times. Trust these docs. Don't re-derive from first
> principles. Don't add the flag "just to see what happens." Childlike trust
> in documented solutions beats scared-teenager re-investigation every time.
> See [PHILOSOPHY.md](PHILOSOPHY.md) §4 and §8.

### WASM data-segment corruption from -matomics / -mbulk-memory

**DO NOT** set `-matomics` or `-mbulk-memory` in global `CMAKE_C_FLAGS` /
`CMAKE_CXX_FLAGS`. These flags MUST NOT appear anywhere in
`platform_emscripten.cmake`'s global compiler flags.

**Why:** Build tools (makesrna, makesdna, datatoc, shader_tool) run under
Node.js at build time and link with `-sSHARED_MEMORY=0`. When ANY object
file linked into these tools was compiled with `-matomics`, the WASM
data-segment layout is corrupted. String constants become garbled — e.g.
`"studiolight_rotate_z"` becomes `"studi3<garbage>ght_r<garbage>ate_z"`,
causing the RNA code generator to fail. Per-target overrides
(`-mno-atomics`) do NOT help because library targets (bf_dna, bf_dna_blenlib,
etc.) are shared between build tools and the main blender executable — their
`.a` objects are already compiled with atomics by the time the build tool links.

**Correct approach:**
- `-matomics` / `-mbulk-memory` are applied ONLY to the `blender` target
  via its PRIVATE `-pthread` compile option (in `source/creator/CMakeLists.txt`)
- The blender link uses `-Wl,--no-check-features` to accept library objects
  compiled without atomics into its shared-memory binary
- Build tools compile and link without atomics — no special overrides needed

**This was broken and "fixed" multiple times.** If you're an AI assistant
working on this codebase: do NOT re-add global `-matomics` or
`-mbulk-memory` flags. The comment in `platform_emscripten.cmake` explains
the full rationale. Trust it.

### Global CMAKE_EXE_LINKER_FLAGS leak into build tools

**DO NOT** put browser-specific link flags in `CMAKE_EXE_LINKER_FLAGS`.
That variable applies to ALL executables — including build tools.

**What happened:** Browser flags (INITIAL_MEMORY=512MB, MODULARIZE=1,
USE_WEBGL2, FORCE_FILESYSTEM, etc.) were placed in global linker flags.
Build tools (makesrna, makesdna, datatoc, shader_tool) inherited them,
causing memory layout issues and data corruption — the exact same garbled
RNA string symptom as the atomics issue.

**Correct approach:** Browser link flags live in `EMSCRIPTEN_BROWSER_LINK_FLAGS`
(a CMake list variable) and are applied ONLY to the blender target via
`target_link_options()` in `source/creator/CMakeLists.txt`.
Global `CMAKE_EXE_LINKER_FLAGS` contains only `-sWASM=1 -sASSERTIONS=1`.

### Global -pthread corrupts build tools (same root cause)

**DO NOT** put `-pthread` in global `CMAKE_C_FLAGS` / `CMAKE_CXX_FLAGS`.

**Why:** `-pthread` at compile time enables shared-memory codegen (TLS,
atomics, bulk-memory). Build tools then get compiled with shared-memory
patterns but linked with `-sUSE_PTHREADS=0 -sSHARED_MEMORY=0`. This
compile/link mismatch corrupts the WASM data segment in exactly the same
way as the explicit `-matomics` flag.

**Correct approach:** `-pthread` is PRIVATE to the blender target only
(both compile and link options in `source/creator/CMakeLists.txt`).

### The build-tool / browser-target separation (META RULE)

Emscripten has TWO completely different execution contexts in this build:

1. **Build tools** (makesrna, makesdna, datatoc, shader_tool, msgfmt,
   zstd_compress) — run under **Node.js** at build time, single-threaded,
   non-shared-memory, NODERAWFS filesystem
2. **Browser target** (blender) — runs in **browser**, multi-threaded via
   Web Workers, shared memory, WebGL2, MODULARIZE, IDBFS filesystem

**Every global flag must be safe for BOTH contexts.** If a flag is only
appropriate for one context, it must be applied per-target. The full list
of build tools with Emscripten blocks:
- `source/blender/makesrna/intern/CMakeLists.txt`
- `source/blender/makesdna/intern/CMakeLists.txt`
- `source/blender/datatoc/CMakeLists.txt`
- `source/blender/gpu/shader_tool/CMakeLists.txt`
- `source/blender/blentranslation/msgfmt/CMakeLists.txt`
- `intern/cycles/kernel/CMakeLists.txt` (zstd_compress)

When adding a NEW build tool, call `emscripten_build_tool_flags(<target>)`
(defined in `platform_emscripten.cmake`). Use `STACK_SIZE <bytes>` for
tools that need a larger stack (code generators, shader tools):
```cmake
if(EMSCRIPTEN)
  emscripten_build_tool_flags(my_tool STACK_SIZE 8388608)
endif()
```

### Emscripten sets UNIX=true — watch for false matches

Emscripten defines `UNIX=true` in CMake. Any `elseif(UNIX)` guard will
match Emscripten, causing desktop-only targets to build. This happened
with `blender-thumbnailer` which had no purpose in the browser. Always
check Emscripten before UNIX:
```cmake
if(EMSCRIPTEN)
  # skip or handle differently
elseif(UNIX)
  # desktop UNIX only
endif()
```

### Build tools need CMAKE_CROSSCOMPILING_EMULATOR

When cross-compiling to Emscripten, build tools produce `.js` files that
need Node.js to execute. `add_custom_command` does NOT automatically use
`CMAKE_CROSSCOMPILING_EMULATOR` (unlike `add_test`). Every
`add_custom_command` that invokes a build tool must prepend
`${CMAKE_CROSSCOMPILING_EMULATOR}`. On native builds it expands to nothing.

### Build tools need SINGLE_FILE=1

Build tools run from `add_custom_command` which sets CWD to the output
directory, not `bin/`. Emscripten looks for `.wasm` relative to CWD,
causing "ENOENT: makesdna.wasm". `SINGLE_FILE=1` embeds the WASM binary
as base64 in the `.js` file, eliminating the lookup. Always set this for
build tools.

### OOM silent failures on CI

Emscripten's em++ uses ~2x more RAM than native compilers. GitHub Actions
runners have 7 GB RAM. `--parallel=$(nproc)` (4 cores) causes OOM kills.
OOM-killed processes produce NO stderr output — the build just says
"subcommand failed" with no explanation.

**Rule:** Use `--parallel 2` for Emscripten CI builds. If a build fails
with no error output at all, suspect OOM.

### wasm-opt OOM on large binaries

Binaryen's `wasm-opt` runs as the final post-link step when em++ links
with `-O2` or higher. For Blender's ~50 MB WASM binary, `wasm-opt -O2`
requires more RAM than GitHub Actions runners (7 GB) provide. The process
gets OOM-killed (`SIGKILL -9`), which prevents `em++` from emitting the
`.js` glue file — making the build appear to fail even though ALL 5000+
compilation targets succeed and the raw `.wasm` binary exists.

**Symptom:** Build output shows `[5446/5446] Linking CXX executable
bin/blender.js` then `em++: error: ... wasm-opt ... failed (received
SIGKILL (-9))`. The `.wasm` file exists on disk but `.js` does not.

**Fix:** Force the LINK step to `-O1` in the browser link flags (in
`platform_emscripten.cmake`). Emscripten only runs `wasm-opt` when
`OPT_LEVEL >= 2`, so `-O1` at link time skips it entirely. Object files
are still compiled at `-O2` (from `CMAKE_BUILD_TYPE=Release`), so codegen
quality is unaffected — only the post-link Binaryen passes are skipped.
For release builds with sufficient memory (16+ GB), run `wasm-opt`
separately: `wasm-opt -O2 bin/blender.wasm -o bin/blender.wasm`.

### WASM is 32-bit: size_t overflows and pointer assumptions

WASM uses 32-bit pointers and `size_t`. Code like `size_t(1) << 32` or
`sizeof(void*) == 8` assumptions will break. Blender was written for
64-bit systems. Watch for:
- `size_t` shifts that exceed 32 bits
- Pointer size assumptions in serialization
- Large allocation sizes that don't fit in 32-bit address space

### Epoxy shim: don't shadow real WebGL2/GLES3 functions

The epoxy shim (`emscripten_compat/`) provides stubs for desktop GL
functions that don't exist in WebGL2. But some functions that WERE stubbed
as no-ops are actually provided by Emscripten's GLES3 runtime (e.g.
`glFenceSync`, `glDeleteSync`, `glClientWaitSync`, `glWaitSync`).
Shadowing them breaks real functionality.

**Rule:** Before stubbing a GL function, check if Emscripten already
provides it via GLES3. Only stub functions that are truly desktop-only
(compute shaders, DSA, geometry shaders, etc.). The stubs in `epoxy_shim.c`
should log warnings, not silently swallow calls to functions that might
have real implementations.

### FreeType: Emscripten port may be older than expected

Emscripten's FreeType port version lags behind system FreeType. Functions
like `FT_Done_MM_Var` (added in 2.9) and `FT_Get_Var_Design_Coordinates`
(added in 2.8.1) may not be available. Guard with version checks:
```c
#if FREETYPE_MAJOR > 2 || (FREETYPE_MAJOR == 2 && FREETYPE_MINOR >= 9)
  FT_Done_MM_Var(library, amaster);
#else
  free(amaster);
#endif
```

### libjpeg boolean type: use TRUE/FALSE not true/false

Emscripten's libjpeg defines `boolean` as `int`, not C++ `bool`. Using
C++ `true`/`false` where libjpeg's `boolean` is expected causes type errors.
Use the `TRUE`/`FALSE` macros provided by libjpeg instead.

### Config contradictions between blended_wasm.cmake and platform_emscripten.cmake

`blended_wasm.cmake` (user config) and `platform_emscripten.cmake`
(platform enforcer) both set many of the same options. If they disagree,
the one that runs LAST wins (platform runs after config). This caused
`WITH_IMAGE_OPENJPEG` to be ON in config but force-OFF in platform.

**Rule:** `platform_emscripten.cmake` is the source of truth for what
Emscripten CAN support. `blended_wasm.cmake` is for what we WANT enabled.
If platform says OFF, config must also say OFF to avoid confusion.

### Trust user observations over your own theories

When a user tells you what they see on screen, **believe them**. Do not
reinterpret their observations to fit your existing theory.

**What happened:** The user reported a blank screen on mobile after the
WASM web editor loaded. The AI assistant:
1. Misidentified the Android navigation gesture bar as a "loading bar"
2. Concluded the page was "still loading" and declared progress
3. When corrected, assumed GitHub Pages wasn't deployed (403) and
   told the user to enable Pages in repo settings
4. The user corrected again: the page DOES load, shows the "Blended"
   title, shows all green feature checkmarks, THEN goes blank

The actual bug was that `setStatus("")` was hiding the loading overlay
before `onRuntimeInitialized` fired — a simple premature-hide bug. But
it took multiple rounds of the user pushing back against wrong theories
before the real investigation happened.

**Rule for AI assistants:** When the user says "you're wrong," stop
defending your theory and start listening. The user is looking at the
actual screen. You are not. Re-examine your assumptions from scratch
instead of finding new ways to explain why you were "actually right."
A screenshot is evidence — your theory is a guess.

### Don't guess at root causes — verify before "fixing"

The RNA string corruption bug was "fixed" 7+ times with different theories:
1. Stack overflow (wrong — increased to 32MB, didn't help)
2. Pthreads compile/link mismatch (partially right direction)
3. Added hex dump diagnostic (debugging, not a fix)
4. Moved -pthread from global linker to per-target (right direction)
5. Moved -pthread from global compiler to per-target (right direction)
6. Isolated browser link flags (fixed one cause)
7. Added global -matomics with per-target overrides (wrong — reintroduced)
8. Removed global -matomics entirely (correct)

**Rule for AI assistants:** When the same error recurs after a "fix", your
diagnosis was wrong. Do NOT keep trying variations of the same approach.
Step back and re-examine the fundamental assumptions. Read the EXISTING
comments and commit history before proposing a fix — the answer may already
be documented.

## Rules

> *"A single static_cast replacing an implicit narrowing conversion matters.
> Don't create complexity to avoid simplicity."*
> — [PHILOSOPHY.md](PHILOSOPHY.md) §3, §6

- **Fix, don't suppress.** Use explicit casts (`static_cast<>`, `int(x)`)
  where the conversion is intentional. A cast is the fix — not a template,
  not a macro, not a new header.
- **Don't add `(void)` casts** for unused parameters in Blended code —
  use `[[maybe_unused]]` or remove the parameter name.
- **Don't touch extern/.** Third-party code gets pragmas, not fixes.
- **Don't fix warnings in code paths disabled for WASM** (Cycles,
  Python, FFMPEG, etc.) — those are Blender's problem.
- **Each PR should remove warnings from one subsystem**, not scatter
  fixes across the whole tree. Each PR heals a facet. When the facet is
  stable, move to the next level. The healing cascades (§9).
- **When your fix doesn't work**, re-examine your assumption, not the
  code. The codebase is probably right. If the same error recurs after
  a "fix", your diagnosis is wrong (§4).

## Files

| File | Purpose |
|------|---------|
| `build_files/cmake/warnings_report.cmake` | CMake option to enable warning capture |
| `build_files/utils/parse_warnings.py` | Log parser and report generator |
| `build_files/cmake/platform/platform_emscripten.cmake` | Where blanket suppressions live |
| `build_files/cmake/config/blended_wasm.cmake` | WASM build configuration |
