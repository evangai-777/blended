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
- Emscripten cross-compilation with WebGL2 shader compatibility, web shell, and first-frame rendering
- See [`build_files/web/WEBASSEMBLY_ROADMAP.md`](build_files/web/WEBASSEMBLY_ROADMAP.md) for the full 6-stage roadmap and technical details
