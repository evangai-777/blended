<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) being rebuilt from the foundation up around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

The project is at 0.3.0 (tagged, CI green) — 0.4.0 in progress. Early, honest, and moving forward with intention.

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

See [`BLENDED.md`](../BLENDED.md) for the full design document: identity, architecture, datablock audit, full pipeline specs for all eight pipeline sections, and guardrails.

What's Different Right Now
--------------------------

- **Branding** — "Blended 0.1.0" in window titles, splash screen, and about dialog. Tagline: *"Blender, simplified."* CMake project renamed to Blended.
- **Pre-5.0 rig compatibility** — `blended_rig_compat.py` restores `action.fcurves` as a compatibility property on `bpy.types.Action`. Pre-Blender-5.0 Rigify rigs (including CGCookie Vonnbots rigs) that access `action.fcurves` directly work again. IK/FK bake operators no longer fail silently.
- **Update notifications** — Background GitHub Releases check at startup (24-hour cache, non-blocking). Top-bar notification with version string when an update is available. One-click download via browser. "Blended Updates" panel in System Preferences.
- **CI** — Windows x64 portable `.zip` builds via GitHub Actions. Branch pushes run a fast lite build for compile-error checking. Tags produce a full release artifact. `blended_release.cmake` disables GPU kernel pre-compilation (CUDA/HIP/OneAPI) to keep CI under an hour — runtime compilation covers the same hardware.
- **Datablock audit — 0.4.x in progress.** Target: 39 → ~19 ID types. Removed so far: `ID_WS` ✓ (0.2.0), `ID_SCR` + `ID_WM` ✓ (0.3.0), `ID_PC` + `ID_SPK` + `ID_PA` + `ID_GD_LEGACY` + `ID_LS` ✓ (0.4.0). Next: `ID_MB`. See [`CHANGELOG.md`](../CHANGELOG.md) for per-layer file detail.

On the Horizon
--------------

Five foundation layers to go — one minor version each, 1.0 when all five are honest. Full roadmap with per-milestone scope in [`CHANGELOG.md`](../CHANGELOG.md).

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
  documentation architecture (CLAUDE.md, UPSTREAM_SYNC.md, this README, archive consolidation);
  `ID_WS` (WorkSpace) removal — full chisel across all layers; `ID_SCR` and `ID_WM` removal
  — full chisel across all layers, introducing ID_SCR_LEGACY/ID_WM_LEGACY routing pattern;
  `ID_PC` (PaintCurve) removal — 35+ files, three entire files deleted, paint curve undo
  subsystem gutted, versioning pass added;
  `ID_SPK` (Speaker) removal — 45+ files, five entire files deleted (DNA_speaker_types.h,
  speaker.cc, BKE_speaker.hh, rna_speaker.cc, overlay_speaker.hh), speaker 3D audio loop
  and NLA sound strip function removed, ANIMTYPE_DSSPK animation channel removed,
  SPEAKER_EVAL depsgraph opcode removed, versioning pass 502.23 added;
  `ID_PA` (ParticleSettings) removal — 40+ files, IDTypeInfo and all static callbacks
  removed from particle.cc, rna_main.cc listbase unregistered, depsgraph two-pass
  teardown guard updated, rna_space.cc asset browser filter corrected (literal grep miss);
  Scar 2 pattern applied post-chisel: bmain->particles and which_libbase routing restored
  as non-indexed listbase — blenloader versioning_250 through versioning_400 iterate it to
  upgrade particle data in legacy files; INIT_TYPE and BKE_main_lists_get entry remain removed;
  `ID_GD_LEGACY` (legacy Grease Pencil) removal — 30+ files, IDTypeInfo removed,
  RNA registration (BlendDataAnnotations, rna_Main_annotations_new, gpencils listbase)
  removed, editor dispatch tables cleared; bmain->gpencils and which_libbase routing
  preserved (Scar 2 pattern — OB_GPENCIL_LEGACY objects and annotations still use
  bGPdata at runtime); depsgraph geometry node building kept for same reason;
  rna_space.cc asset browser filter corrected (same grep-miss pattern as ID_PA);
  `ID_LS` (FreestyleLineStyle) removal — ~50 files (vs 28 literal hits), ANIMTYPE_DSLINESTYLE
  chain across 7 files (9 fallthrough cases), ACF_DSLINESTYLE animation channel type removed,
  animdata_filter_ds_linestyle function removed, ADS_FILTER_NOLINESTYLE bitmask removed,
  tree_element_id_linestyle.cc/.hh deleted, NC_LINESTYLE cases in space_node.cc (unguarded),
  4 NESTED_ID_NASTY_WORKAROUND SPECIAL_CASE entries removed from depsgraph COW; Scar 2
  pattern: bmain->linestyles and which_libbase routing preserved for anim_sys iteration;
  Post-chisel allocation crash (Scar 10): INIT_TYPE removal breaks BKE_libblock_alloc for
  that type in all build types — BKE_libblock_alloc_notest returns nullptr (size=0, no
  IDTypeInfo), BKE_libblock_runtime_ensure then dereferences it. Three allocation functions
  patched to use MEM_new<T> + manual ID init bypassing the registry: BKE_particlesettings_add
  (particle.cc), BKE_gpencil_data_addnew (gpencil_legacy.cc), BKE_linestyle_new (linestyle.cc).
  MEM_callocN is wrong for non-trivial types (skips constructors); MEM_new is correct.
  ongoing PR review and integration: 10+ PRs assessed, applied selectively.
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
