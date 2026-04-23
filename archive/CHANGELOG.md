# Blended Changelog

## 1.1.0 — Based on Blender 5.2

WebAssembly browser port, build infrastructure improvements, and comprehensive tier filtering.

### Tiered UI Expansion
- Extended `blended_min_tier` panel filtering to 30+ property files across all data types
- Material, light, camera, mesh panels: granular tier assignments (Simple → core, Standard → intermediate, Advanced → custom props)
- Armature, bone, curve, texture: full data-type-level tier gating
- Specialized data types: grease pencil, lattice, metaball (Standard); lightprobe, point cloud, volume (Advanced)
- Collection panels: instancing/exporters at Standard, Line Art at Advanced
- 3D View sidebar/toolbar: animation panels at Standard, particle mode at Standard
- Image/UV Editor: scopes and mask panels at Standard, UDIM tiles at Standard
- Node Editor: texture mapping/backdrop/quality at Standard, custom properties at Advanced
- Dope Sheet: grease pencil layer panels at Standard, custom properties at Advanced
- Freestyle rendering: all panels gated to Advanced tier
- Workspace settings: panel gated to Standard, custom properties at Advanced
- Shader node add menu: simplified for Simple tier (hides Displacement, Utilities, Layout)
- Smart defaults: Simple tier hides N-panel sidebar, sets outliner to View Layer mode

### Web Editor (WebAssembly) — In Progress
- Emscripten cross-compilation: compiles, links, and renders first frame in-browser
- WebGL2 shader compatibility: `sampler1DArray` emulation, geometry/compute shader guards, SSBO→UBO rewriting, GL capability query fallbacks
- Web shell with loading screen, drag-and-drop `.blend` support, and `coi-serviceworker` for SharedArrayBuffer
- CPU command fallback for draw engine compute dispatch (6 sites across draw/eevee/workbench)
- Epoxy shim covering all desktop GL functions used by `gpu/opengl/` backend
- GitHub Pages deployment pipeline (`build-wasm.yml`)
- See [`build_files/web/WEBASSEMBLY_ROADMAP.md`](build_files/web/WEBASSEMBLY_ROADMAP.md) for the full 6-stage roadmap

### Build Infrastructure
- `emscripten_build_tool_flags()` CMake helper for consistent build-tool configuration
- Fixed WASM data-segment corruption from global `-matomics`/`-mbulk-memory` flags
- Isolated browser link flags from build tools (`CMAKE_EXE_LINKER_FLAGS` → per-target)
- Skip `wasm-opt` at link time to avoid OOM on CI runners
- Warning-report infrastructure (`parse_warnings.py`, `warnings_report.cmake`)

### Warning Fixes
- GPU backend: sign-conversion and narrowing fixes across `gl_state`, `gl_texture`, `gl_framebuffer`, `gl_storage_buffer`, `gl_uniform_buffer`, `gl_immediate`, `gl_index_buffer`, `gl_vertex_buffer`
- Draw engine: `int → uint` sign-conversion fixes in compute dispatch calls across 6 files
- Core libraries: unsigned bit-shift fixes in `noise.cc`, `noise_c.cc`, `math_matrix_c.cc`, `attribute.cc`

### Documentation
- Consolidated README, CHANGELOG, and WARNINGS to reduce duplication
- Added Document Map to README centralizing links to all project docs
- Cross-references between WARNINGS.md and WEBASSEMBLY_ROADMAP.md

---

## 1.0.0 — Based on Blender 5.2

Initial release of Blended.

### Tiered UI System
- Three-tier complexity system: Simple, Standard, Advanced
- Tier selector on splash screen and in Preferences > Interface > Display
- Panel, editor, and menu filtering per tier
- Workspace tab filtering by tier

### Smart Defaults
- Tier-aware defaults applied on new file creation
- Simple: Forces EEVEE, increased samples, Ambient Occlusion, hides Python tooltips
- Standard: Hides Python tooltips
- Advanced: Full Blender with Python tooltips enabled

### Branding
- Window title, splash screen, and about dialog show "Blended"
- Tagline: "Blender, simplified"
- Custom Blended.xml theme preset
- Windows .rc metadata branded as Blended
- Linux .desktop entry branded as Blended
- Custom app icon and splash screen planned for a future release

### Update Notifications
- Background check against GitHub Releases on startup
- 24-hour cache, non-blocking
- Top-bar notification with one-click download

### CI / Prebuilt Binaries
- Windows x64 portable .zip via GitHub Actions
