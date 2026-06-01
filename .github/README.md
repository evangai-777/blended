<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) being rebuilt from the foundation up around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

Currently at 1.0.0-dev — 0.9.0 CI-complete (Windows x64, build 101, commit `c8e87078`). Third first-try on the project. 0.9.0 delivered: `.blend` import ✓ — OB_MBALL versioning pass 502.31, `BKE_main_clear` Scar 2 drain (PA/TE/CU/LS), `BKE_screen_blend_read_data` deletion, dropped-data manifest Text block. 1.0.x: foundation complete — runtime audit + GitHub Pages launch in progress. Release tag when both clear.

What Blended Is
---------------

Blender carries three stacked, unreconciled visions from three different eras. Blended resolves them:

- **Studio-tool discipline** — animation is the shaping principle; design decisions serve it.
- **Access mission** — openable by a first-time user. Free means free.
- **Full creative scope** — 2D animation, 3D animation, game assets, compositing, audio. The breadth is real. The feature-parity pathology is not.

One animation engine — depsgraph, keyframes, F-curves, timeline — powers every content type. See [`BLENDED.md`](../BLENDED.md) for the full design document, architecture, and pipeline specs.

What's Different
----------------

- **Branding** — "Blended X.Y.Z" in window titles, splash, and about dialog. Tagline: *Blender, simplified.*
- **Simplified data model** — 39 → ~19 ID types. Fossils, UI-state types, and upstream deprecations removed through the datablock audit (0.2.x–0.5.x). Remaining types have a permanent home in the data model, not just tolerance.
- **`.blended` project format** — magic bytes `"BLENDED"`, `.blended` extension, `FILE_TYPE_BLENDED` flag, platform integration (Windows registry, Linux MIME, macOS UTI). Import from `.blend` is one-way with a dropped-data manifest.
- **Pre-5.0 rig compatibility** — `blended_rig_compat.py` restores `action.fcurves` for pre-Blender-5.0 Rigify rigs.
- **Update notifications** — Background GitHub Releases check at startup with top-bar notification when an update is available.
- **CI** — Windows x64 portable builds via GitHub Actions. Branch pushes: lite compile check. Tags: full release artifact.

Foundation Layers
-----------------

Six foundation layers — one minor version each, 1.0 when all six are honest.

| Version | Layer | Status |
|---------|-------|--------|
| 0.4.x | Datablock audit — 9 fossil removals (Bucket 5+6) | ✓ CI-complete (build 70) |
| 0.5.x | Datablock audit — Bucket 3 fold-downs complete; 39 → ~19 ID types | ✓ CI-complete (build 81) |
| 0.6.x | Evaluation model — depsgraph/draw/editor seam closure | ✓ CI-complete (build 82, commit `8f7dda22`) |
| 0.7.x | App lenses — launcher + all 28 mode lenses + full product identity | ✓ CI-complete (build 99, commit `2ddd1dd0`) |
| 0.8.x | File format — `.blended` magic, extension, platform integration | ✓ CI-complete (build 100, commit `99e20b96`) |
| 0.9.x | `.blend` import — seamless read, dropped-data manifest | ✓ CI-complete (build 101, commit `c8e87078`) |
| 1.0.0 | Foundation complete — Phase 1: runtime audit → Phase 2: GitHub Pages launch | In progress |

Full version history in [`CHANGELOG.md`](../CHANGELOG.md).

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

Blended is developed and published by **CHJ 3 Productions LLC** (Indiana).

Developed with [Claude](https://anthropic.com) (Anthropic) as primary implementation partner across the full rebuild — version identity, CI pipeline, rig compatibility, update checker, datablock audit (0.2.x–0.5.x), evaluation model seam closure (0.6.x), launcher and full product identity (0.7.x), file format (0.8.x), and `.blend` import (0.9.x). [Codex](https://openai.com) (OpenAI) contributes automated code review on pull requests, catching regressions and missed sites throughout 0.4.x–0.9.x. Per-release detail in [`CHANGELOG.md`](../CHANGELOG.md).

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
