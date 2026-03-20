# Blended Changelog

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
- WebAssembly browser build via Emscripten (in progress)

### Web Editor (WebAssembly) — In Progress
- Emscripten cross-compilation infrastructure
- GitHub Pages deployment pipeline
- Web shell with loading screen and drag-and-drop .blend support
- WebGL2 shader compatibility: `sampler1DArray`→`sampler2D` emulation with `tex1DArrayLookup()` helper across 7 shader files
- Geometry and compute shader stage guards for Emscripten (WebGL2 has neither)
- SSBO→UBO shader rewriting: `buffer`/`std430` declarations emit `uniform`/`std140` on Emscripten
- GL capability query guards for unsupported constants (`GL_MAX_GEOMETRY_*`, `GL_MAX_COMPUTE_*`, `GL_MAX_SHADER_STORAGE_*`)
- `GPU_TEXTURE_1D_ARRAY`→`GL_TEXTURE_2D` mapping for WebGL2
- GL ES 3.0 context creation for Emscripten's SDL2
- Force initial window redraw to fix blank canvas on browsers (especially mobile)
- Runtime method exports (`FS`, `ENV`) and filesystem setup for web shell
- Fix WASM module loading: keep original Emscripten output filenames
- Skip `wasm-opt` at link time to avoid OOM on CI runners
