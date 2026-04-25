<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) being rebuilt from the foundation up around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

The project is at 0.1.0 — early, honest, and moving forward with intention.

What Blended Is
---------------

Blender carries three stacked, unreconciled visions from three different eras. Blended resolves them:

- **From Blender's studio-tool era** — opinionated discipline. Animation is the shaping principle; design decisions serve it.
- **From Blender's access mission** — the tool is openable by a first-time user. Free means free.
- **From Blender's industry-suite era** — the full breadth of creative work: 2D animation, 3D animation, game assets, compositing, audio. The scope is real. The pathology (feature-parity arms race, implicit priorities, formats deforming internals) is not.

The user-facing structure is the production pipeline itself:

> **Creative:** Storyboarding → 2D Animation → 3D Animation → Game → Design
>
> **Post:** Finalizing → Compositing → Audio

One animation engine — depsgraph, keyframes, F-curves, timeline — powers every content type. Every property in every section is keyframeable. Static work is animation with one frame.

See [`BLENDED.md`](../BLENDED.md) for the full design document: identity, architecture, datablock audit, pipeline specs, and guardrails.

What's Different Right Now
--------------------------

- **Branding** — "Blended 0.1.0" in window titles, splash screen, and about dialog. Tagline: *"Blender, simplified."* CMake project renamed to Blended.
- **Pre-5.0 rig compatibility** — `blended_rig_compat.py` restores `action.fcurves` as a compatibility property on `bpy.types.Action`. Pre-Blender-5.0 Rigify rigs (including CGCookie Vonnbots rigs) that access `action.fcurves` directly work again. IK/FK bake operators no longer fail silently.
- **Update notifications** — Background GitHub Releases check at startup (24-hour cache, non-blocking). Top-bar notification with version string when an update is available. One-click download via browser. "Blended Updates" panel in System Preferences.
- **CI** — Windows x64 portable `.zip` builds via GitHub Actions. Branch pushes run a fast lite build for compile-error checking. Tags produce a full release artifact. `blended_release.cmake` disables GPU kernel pre-compilation (CUDA/HIP/OneAPI) to keep CI under an hour — runtime compilation covers the same hardware.

On the Horizon
--------------

The foundation-first rebuild is underway. Build order per the design doc:

1. **File format** — `.blended` is the project, period. Import/export is an explicit boundary.
2. **Datablocks** — 39 → ~19 ID types. Fossils and UI-state datablocks removed.
3. **Evaluation model** — depsgraph audit under Blended's scope.
4. **App lenses** — the launcher as the canonical workspace system.
5. **UI** — only after 1–4 are honest.

`ID_WS` (WorkSpace) removal is the next code milestone — load-bearing for the launcher model becoming structurally true rather than just conceptually right.

Changelog
---------

See [`CHANGELOG.md`](../CHANGELOG.md) for a full list of changes per release.

Building from Source
--------------------

Blended builds the same way as Blender on all platforms:

- [Build Instructions](https://developer.blender.org/docs/handbook/building_blender/)

For CI / release builds:

```sh
cmake -S . -B build \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -C build_files/cmake/config/blended_release.cmake
cmake --build build --target install
```

`blended_release.cmake` inherits the full Blender release config and turns off GPU binary pre-compilation. Everything else — EEVEE, Cycles, Cycles GPU via runtime compilation, USD, Alembic, OSL — ships.

Contributors
------------

Blended is developed with contributions from both human developers and AI tools.

- **Claude** (Anthropic) — Implementation partner across the full rebuild, not just planning:
  version identity (`BLENDED_VERSION_*` defines independent of Blender's version integer,
  `BKE_blended_version_string()`, window titles, splash screen, about dialog, tagline);
  pre-5.0 Rigify rig compatibility (`blended_rig_compat.py` — `_FCurvesCompat` proxy
  restoring `action.fcurves` across Blender 5.x's layered action system);
  background update checker (`blended_update_check.py` — GitHub Releases API, 24-hour
  cache, top-bar notification, System Preferences panel);
  Windows x64 CI/CD pipeline (`build-windows.yml` — LFS handling, submodule management,
  library caching, artifact packaging, GitHub Release automation);
  `blended_release.cmake` build configuration;
  documentation architecture (CLAUDE.md, UPSTREAM_SYNC.md, this README, archive
  consolidation — assessing and pruning eight archive files against current scope);
  ongoing PR review and integration: 10+ PRs assessed, applied selectively, with
  explicit reasoning about what belongs in the rebuild and what doesn't.
  *"Listen to the whole thing before reacting."*

Upstream Blender Resources
--------------------------

- [Blender Website](https://www.blender.org)
- [Reference Manual](https://docs.blender.org/manual/en/latest/index.html)
- [Developer Documentation](https://developer.blender.org/docs/)

License
-------

Blended, like Blender, is licensed under the GNU General Public License, Version 3.
Individual files may have a different but compatible license.

See [blender.org/about/license](https://www.blender.org/about/license) for details.
