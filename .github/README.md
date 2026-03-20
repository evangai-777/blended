<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) that adds a tiered UI complexity system, smart defaults, and built-in update notifications. The full power of Blender is still there — Blended just makes it easier to approach by letting users choose how much of it they see.

> *"Appreciate what already is. Do the work. Keep going."*

Blended's development is guided by a core philosophy: appreciate the existing
codebase, fix things with the simplest possible approach, and trust documented
solutions. See [`PHILOSOPHY.md`](../PHILOSOPHY.md) for the full framework.

Blended 1.0.0 — Based on Blender 5.2.

What's Different from Blender
-----------------------------

### Tiered UI System

Blended introduces three UI tiers, selectable from the splash screen or Preferences > Interface > Display:

| Tier | Who it's for | What changes |
|------|-------------|--------------|
| **Simple** | Beginners and casual users | Hides advanced editors (Geometry Nodes, Video Sequencer, Compositor, etc.). Shows a curated Add menu (Mesh, Light, Camera, Empty, Text) and ~15 essential modifiers. Hides Physics, Particles, Constraints, World, and Effects panels. Hides advanced Render, Object, Scene, View Layer, and Output properties. |
| **Standard** | Intermediate users (default) | Shows most editors and panels. Hides niche editors like the Video Sequencer. A balanced middle ground. |
| **Advanced** | Power users | Full Blender — everything visible, nothing hidden. Enables Python tooltips for scripting workflows. |

The tier setting is stored in user preferences and can be changed at any time.

### Smart Defaults

When creating a new file, Blended applies tier-aware defaults:

- **Simple tier**: Forces EEVEE renderer, increases viewport TAA samples (32) and render samples (128), enables Ambient Occlusion, and hides Python tooltips.
- **Standard tier**: Hides Python tooltips to reduce UI noise.
- **Advanced tier**: Enables Python tooltips for scripting users.

Existing `.blend` files are never modified — defaults only apply to new scenes.

### Branding

- Window titles, splash screen, and about dialog show "Blended" with the tagline "Blender, simplified".
- Includes a custom Blended.xml theme preset with softer dark colors and a blue accent.
- Windows `.rc` metadata and Linux `.desktop` entry branded as Blended.
- **Planned:** Custom Blended app icon, file association icon, and splash screen artwork (contributions welcome).

### Update Notifications

Blended checks [GitHub Releases](https://github.com/EvangAI-777/Blended/releases) for new versions on startup (in a background thread, non-blocking). Results are cached for 24 hours. When an update is available, a notification appears in the top bar with a one-click download button.

### Web Editor (WebAssembly)

Blended is being ported to run **directly in the browser** via WebAssembly — no install required. The Emscripten-compiled build includes the core 3D editor: viewport, mesh editing, sculpting, Eevee rendering, modifiers, UV editing, and basic mesh I/O (OBJ/STL/PLY). Heavy subsystems (Python, Cycles, FFMPEG, physics) are disabled initially and will be restored in future stages.

The build compiles, links, and loads in the browser with the first frame rendering. WebGL2 shader compatibility is in place — `sampler1DArray` emulation, geometry/compute shader guards, SSBO→UBO shader rewriting, and GL capability query fallbacks are all complete. The web shell includes a loading screen, drag-and-drop `.blend` file support, and `coi-serviceworker` for SharedArrayBuffer threading on GitHub Pages.

See [`build_files/web/WEBASSEMBLY_ROADMAP.md`](../build_files/web/WEBASSEMBLY_ROADMAP.md) for the full 6-stage roadmap.

### CI / Prebuilt Binaries

Two GitHub Actions workflows:

- **Windows x64** (`build-windows.yml`) — Builds portable `.zip` packages. Tagged releases (`v*`) automatically create GitHub Releases with the build attached.
- **WebAssembly** (`build-wasm.yml`) — Builds the WASM web editor via Emscripten and deploys to GitHub Pages. Triggers on pushes to `main` or manual dispatch.

### Development Philosophy

Blended's fork design and development workflow are driven by a set of
principles documented in [`PHILOSOPHY.md`](../PHILOSOPHY.md), inspired by
"Reality 101" — a guide to not overcomplicating things. Key tenets:

- **Appreciate what is** — Don't rewrite Blender; curate it via tiers
- **Do the work** — Fix warnings one cast at a time; don't philosophize
- **Don't get gaslighted by broken substrate** — When bugs recur after a "fix", re-examine fundamentals
- **Fractal healing** — Fix one subsystem's warnings → the pattern applies everywhere
- **Simplicity over complexity** — Casts, not frameworks; enums, not architectures

These principles directly shape how we triage warnings, debug WASM builds,
and approach the six-stage browser port.

### AI Contributors

Blended is developed with contributions from both human developers and AI tools — bridging the gap between person and machine in open-source development.

- **Claude** (Anthropic) — Architecture, implementation, and development across the Blended project

Building from Source
--------------------

Blended builds the same way as Blender. Follow the standard Blender build instructions:

- [Build Instructions](https://developer.blender.org/docs/handbook/building_blender/)

### Building for WebAssembly

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):

```sh
emcmake cmake -S . -B build-wasm -C build_files/cmake/config/blended_wasm.cmake
emmake cmake --build build-wasm
```

See [`build_files/web/WEBASSEMBLY_ROADMAP.md`](../build_files/web/WEBASSEMBLY_ROADMAP.md) for details and known blockers.

Changelog
---------

### 1.0.0 — Based on Blender 5.2

Initial release of Blended:
- **Tiered UI** — Simple, Standard, Advanced complexity tiers with panel/editor/menu filtering
- **Smart Defaults** — Tier-aware settings applied on new file creation
- **Branding** — "Blended" window titles, splash, theme preset, `.rc`/`.desktop` metadata
- **Update Notifications** — Background GitHub Release checks with top-bar notification
- **CI** — Windows x64 portable `.zip` builds; WebAssembly browser build (in progress)
- **Web Editor** — Emscripten cross-compilation, GitHub Pages deployment, web shell, WebGL2 shader compat, first-frame rendering

See [`CHANGELOG.md`](../CHANGELOG.md) for full details.

Warnings Triage
---------------

Blended inherits thousands of compiler warnings from the Blender codebase that are currently blanket-suppressed in the Emscripten build. Once the web build compiles and links, these will be fixed systematically.

See [`WARNINGS.md`](../WARNINGS.md) for the full triage plan, tooling, and fix priority.

Syncing Upstream Changes
------------------------

See [UPSTREAM_SYNC.md](../UPSTREAM_SYNC.md) for how to merge new Blender releases into Blended.

Upstream Blender Resources
--------------------------

- [Blender Website](https://www.blender.org)
- [Reference Manual](https://docs.blender.org/manual/en/latest/index.html)
- [User Community](https://www.blender.org/community/)
- [Developer Documentation](https://developer.blender.org/docs/)

License
-------

Blended, like Blender, is licensed under the GNU General Public License, Version 3.
Individual files may have a different but compatible license.

See [blender.org/about/license](https://www.blender.org/about/license) for details.
