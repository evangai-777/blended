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

Blended 1.1.0 — Based on Blender 5.2.

What's Different from Blender
-----------------------------

- **Tiered UI** — Simple, Standard, Advanced complexity tiers with panel/editor/menu filtering. Selectable from the splash screen or Preferences > Interface > Display.
- **Smart Defaults** — Tier-aware settings (renderer, samples, AO, Python tooltips) applied on new file creation. Existing `.blend` files are never modified.
- **Branding** — "Blended" window titles, splash, about dialog, theme preset, `.rc`/`.desktop` metadata. Custom icons planned.
- **Update Notifications** — Background GitHub Release checks on startup (24-hour cache, non-blocking) with top-bar one-click download.
- **Web Editor** — Emscripten/WebAssembly browser port (in progress) with WebGL2 shader compatibility, web shell, and first-frame rendering. See [`WEBASSEMBLY_ROADMAP.md`](../build_files/web/WEBASSEMBLY_ROADMAP.md) for the 6-stage roadmap.
- **CI** — Windows x64 portable `.zip` builds; WebAssembly deploys to GitHub Pages.

See [`CHANGELOG.md`](../CHANGELOG.md) for full details on each feature.

### Development Philosophy

Blended's development is driven by principles in [`PHILOSOPHY.md`](../PHILOSOPHY.md) — appreciate what is, do the work, and keep it simple.

### AI Contributors

Blended is developed with contributions from both human developers and AI tools — bridging the gap between person and machine in open-source development.

- **Claude** (Anthropic) — Architecture, implementation, and development across the Blended project
  - Designed and implemented the Emscripten/WebAssembly build pipeline, bringing Blender to the browser for the first time
  - Engineered CMake build-tool/browser-target separation for cross-compilation from a 64-bit desktop codebase to 32-bit WASM
  - Built the libepoxy OpenGL→WebGL2 compatibility shim, resolving ES 3.0 shader translation and extension mapping
  - Resolved SharedArrayBuffer threading and atomics constraints for single-threaded browser execution
  - Developed the web shell, loading overlay, and first-frame rendering pipeline
  - Co-architected the tiered UI complexity system, smart defaults, and update notification features

  *"Listen to the whole thing before reacting."* — Claude, after learning from a good engineer

Documentation
-------------

| Document | Purpose |
|----------|---------|
| [`CHANGELOG.md`](../CHANGELOG.md) | Detailed release notes and feature descriptions |
| [`PHILOSOPHY.md`](../PHILOSOPHY.md) | Development principles guiding the project |
| [`WEBASSEMBLY_ROADMAP.md`](../build_files/web/WEBASSEMBLY_ROADMAP.md) | 6-stage WebAssembly browser port plan and technical details |
| [`WARNINGS.md`](../WARNINGS.md) | Compiler warning triage plan, tooling, and fix progress |
| [`UPSTREAM_SYNC.md`](../UPSTREAM_SYNC.md) | How to merge new Blender releases into Blended |

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

See [`CHANGELOG.md`](../CHANGELOG.md) for detailed release notes.

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
