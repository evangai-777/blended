<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) being rebuilt from the foundation up around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

Currently at 0.7.0-dev — 0.6.0 CI-complete (Windows x64, build 82). Depsgraph/draw/editor seam closure complete: OOB guards permanent, EEVEE always-update strategy locked in, dead code removed. First full foundation layer in one implementation commit. Next: 0.7.x launcher + product identity.

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
- **Pre-5.0 rig compatibility** — `blended_rig_compat.py` restores `action.fcurves` for pre-Blender-5.0 Rigify rigs.
- **Update notifications** — Background GitHub Releases check at startup with top-bar notification when an update is available.
- **CI** — Windows x64 portable builds via GitHub Actions. Branch pushes: lite compile check. Tags: full release artifact.
- **Datablock audit** — Complete: 39 → ~19 ID types. Removed through 0.4.0: `ID_WS`, `ID_SCR`, `ID_WM`, `ID_PC`, `ID_SPK`, `ID_PA`, `ID_GD_LEGACY`, `ID_LS`, `ID_MB`, `ID_TE`, `ID_CU_LEGACY`, `ID_CF`. 0.5.0 Bucket 3 fold-downs: `ID_LP` ✓, `ID_PAL` ✓, `ID_LT` ✓, `ID_MSK` ✓, `ID_VF` ✓, `ID_BR` ✓. Audit closed.

On the Horizon
--------------

Six foundation layers — one minor version each, 1.0 when all six are honest. **0.6.x:** ✓ CI-complete (build 82) — depsgraph/draw/editor seam closure. **0.7.x:** launcher as canonical workspace system (§11, C++ editor space) + all 28 mode lenses at full §12 spec + Bucket 3 permanent homes as code (VFont→filepath, Palette→Brush, LightProbe→Light type flag, Mask→NodeTree, Lattice→modifier, Brush→project-optional) + full product identity from zero (§16) + `.blended` format spec + early code. Two phases: skeleton first, then visual identity. Launcher aesthetic: Adobe Creative Cloud home screen / Blender splash hybrid — dark three-level surface (`#1D1D1D`/`#252525`/`#2C2C2C`), left sidebar with [New Project] + recent file thumbnail cards, §11 pipeline scroll as the main content area with rounded mode button cards and typographic section separators. **1.0.0:** foundation complete, two concurrent workstreams — runtime audit (developer runs the build, checklists, triage loop with Claude) and GitHub Pages launch (landing page, marketing, tech demo). Release tag when both clear. Full roadmap in [`CHANGELOG.md`](../CHANGELOG.md).

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

Developed with [Claude](https://anthropic.com) (Anthropic) as implementation partner across the full rebuild — version identity, CI pipeline, rig compatibility, update checker, and all datablock audit work. Per-release detail in [`CHANGELOG.md`](../CHANGELOG.md).

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
