<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) being rebuilt from the foundation up around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

The project is at 0.3.0 (tagged) — 0.4.0 pending CI (all 9 Bucket 5+6 fossil removals complete). Early, honest, and moving forward with intention.

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

- **Branding** — "Blended 0.3.0" in window titles, splash screen, and about dialog (rendered dynamically from `BLENDED_VERSION_*` defines). Tagline: *"Blender, simplified."* CMake project renamed to Blended.
- **Pre-5.0 rig compatibility** — `blended_rig_compat.py` restores `action.fcurves` as a compatibility property on `bpy.types.Action`. Pre-Blender-5.0 Rigify rigs (including CGCookie Vonnbots rigs) that access `action.fcurves` directly work again. IK/FK bake operators no longer fail silently.
- **Update notifications** — Background GitHub Releases check at startup (24-hour cache, non-blocking). Top-bar notification with version string when an update is available. One-click download via browser. "Blended Updates" panel in System Preferences.
- **CI** — Windows x64 portable `.zip` builds via GitHub Actions. Branch pushes run a fast lite build for compile-error checking. Tags produce a full release artifact. `blended_release.cmake` disables GPU kernel pre-compilation (CUDA/HIP/OneAPI) to keep CI under an hour — runtime compilation covers the same hardware.
- **Datablock audit — 0.4.x complete (pending CI).** Target: 39 → ~19 ID types. Removed: `ID_WS` ✓ (0.2.0), `ID_SCR` + `ID_WM` ✓ (0.3.0), `ID_PC` + `ID_SPK` + `ID_PA` + `ID_GD_LEGACY` + `ID_LS` + `ID_MB` + `ID_TE` + `ID_CU_LEGACY` + `ID_CF` ✓ (0.4.0). `ID_CF` removal: inline per-instance — `CacheFile *` replaced with inlined filepath/settings fields directly in `MeshSeqCacheModifierData` and `bTransformCacheConstraint`; `bmain->cachefiles` removed entirely (true fossil, no Scar 2). See [`CHANGELOG.md`](../CHANGELOG.md) for per-layer file detail.

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
  Version-defines sync: BLENDED_VERSION_* bumped 0.2.0 → 0.3.0 to match the latest tag —
  the defines had been stuck at 0.2.0 even after 0.3.0 shipped, so without the sync the
  wired update checker (below) would have flashed "0.3.0 available" to users already on 0.3.0.
  bpy.app.blended_version_major/minor/patch wired into bpy_app.cc PyStructSequence
  (app_info_fields[] + make_app_info() SetIntItem calls) — not RNA, as a previous CLAUDE.md
  note had wrongly suggested. blended_update_check.py simplified: getattr(..., default)
  fallback dropped (resolves deferred-debt item 6).
  Particle-add operator removal: OBJECT_OT_particle_system_add/_remove and
  PARTICLE_OT_particle_system_remove_all (live in UI but silently broken since the ID_PA
  chisel — the alloc path worked via Scar 10, but the broader particle machinery was gone)
  + the QuickExplode operator that chained them + the four Python UI sites + the CI test
  + the manual-ref URL all deleted. -359 lines across 8 files. BKE_particlesettings_add
  itself stays for fluid.cc Mantaflow particle output (resolves deferred-debt item 1).
  Session Discipline rule + Codex Standard operationalization added to CLAUDE.md: todo
  lists are mandatory at three or more maneuvers; the Codex verification pass is a
  todo-list item before commit/push, not a post-hoc cleanup. The rule cites a real
  same-session incident as its concrete reference.
  `ID_MB` (MetaBall) removal — ~130+ files (vs 60 literal hits), 16 layers:
  editors/metaball subsystem deleted (mball_edit/ops/undo), overlay_metaball.hh deleted,
  transform_convert_mball.cc deleted, abc_writer_mball and usd_writer_metaball deleted
  (WITH_ALEMBIC + WITH_USD both ON in CI), ANIMTYPE_DSMBALL chain across 7 files,
  MetaBall basis machinery in depsgraph builder, MetaBall single-thread evaluation
  workaround removed (is_metaball_object_operation), rna_meta.cc/rna_meta_api.cc deleted,
  tree_element_id_metaball.cc/.hh deleted, Python startup cleaned (properties_data_metaball
  deleted, 5 space_view3d classes removed, space_dopesheet/outliner/userpref/wm patched,
  rigify metaball.new() removed); OB_MBALL=5 enum value kept for .blend compat;
  bmain->metaballs fully removed (true fossil — no versioning pass iterates it).
  `ID_TE` (Texture / Blender Internal) removal — ~76 hits, 45 files, 9 source layers:
  IDTypeInfo removed (texture.cc), INIT_TYPE removed, INDEX_ID_TE and FILTER_ID_TE removed,
  SPECIAL_CASE entries removed from depsgraph COW (deg_eval_copy_on_write.cc — 4 blocks),
  build_texture calls removed from deg_builder_nodes/relations, render_preview.cc Blender
  Internal preview path removed (5 sites), PREVIEW_FILTER_TEXTURE removed from wm_operators,
  tree_element_id_texture.cc/.hh deleted, anim_filter.cc texture animation filter removed,
  rna_main.cc/rna_main_api.cc/rna_internal.hh texture RNA unregistered; Scar 2 pattern:
  bmain->textures and which_libbase routing restored as non-indexed listbase — versioning_250,
  versioning_260, versioning_280, versioning_legacy all iterate it to upgrade Blender Internal
  texture data in legacy files; INIT_TYPE and BKE_main_lists_get entry remain removed;
  anim_sys.cc field-name grep-miss caught post-chisel (EVAL_ANIM_NODETREE_IDS uses field name
  not ID_TE string); post-chisel Codex fix: two direct add_relation() calls in
  deg_builder_relations.cc bypassing the build_texture() no-op (PFIELD_TEXTURE effector
  and RigidBody effector loops) removed — without fix these logged "Failed to add relation"
  errors on every legacy file with a texture force field (Scar 12, commit c320633b).
  `ID_CU_LEGACY` (Legacy Curve) removal — ~86 hits, 36 files, 6 layers (makesdna,
  blenkernel, makesrna, depsgraph active; editors/draw audited — zero compile errors
  via #define preservation; pre-chisel literal count was 74/33):
  IDTypeInfo removed (curve.cc), INIT_TYPE + both CASE_IDINDEX(CU_LEGACY) removed (Scar 4),
  INDEX_ID_CU_LEGACY and FILTER_ID_CU_LEGACY removed, OB_DATA_SUPPORT_EDITMODE/ID/ID_CASE
  macros cleaned; Scar 2: bmain->curves and which_libbase routing kept — 23+ versioning
  iterations across versioning_250 through versioning_520 + anim_data_bmain_utils.cc +
  anim_sys.cc; Scar 8: DNA_DEFINE_CXX_METHODS kept in Curve's #ifdef __cplusplus block,
  only id_type line removed; Scar 10: BKE_curve_add rewritten with MEM_new<Curve> +
  manual bmain insert (live callers: object.cc 3 types, Alembic NURBS reader, OBJ NURBS
  importer, mesh_convert.cc, rna_main_api.cc); two depsgraph OOB crash paths fixed
  (add_id_node id_type_exist + DEG_graph_id_type_tag id_type_updated — both guarded with
  BKE_idtype_idcode_to_index < 0 checks); FILTER_ID_CU_LEGACY grep-miss in key.cc:173 and
  rna_space.cc:3951 caught during layer edits.
  `ID_CF` (CacheFile) removal — ~76 files, 8 layers, inline per-instance design:
  MeshSeqCacheModifierData and bTransformCacheConstraint carry filepath/settings directly;
  bmain->cachefiles removed entirely (true fossil, no Scar 2); full Alembic/USD reader
  hierarchy migrated (all CacheFile* parameters replaced with inline path+settings);
  ANIMTYPE_DSCACHEFILE chain removed (anim_channels_defines.cc ACF_DSCACHEFILE block,
  animdata_filter_ds_cachefile function, fallthrough cases across 7 anim/NLA/transform files);
  files deleted: io_cache.cc, io_cache.hh, interface_template_cache_file.cc, rna_cachefile.cc,
  cachefile.cc, BKE_cachefile.hh; versioning_290.cc velocity_unit loop removed;
  CTX_data_edit_cachefile and BLT_I18NCONTEXT_ID_CACHEFILE removed; RNA migrated from
  PROP_POINTER to PROP_STRING/PROP_FILEPATH in rna_constraint.cc and rna_modifier.cc.
  Post-merge CI fixes: MeshSeqCacheModifierData DNA alignment (5 chars → two 4-char groups;
  PR #156); BLT_I18NCONTEXT_ID_CACHEFILE borrowed by NodesModifier bake_target properties —
  Scar 13, invisible to ID_CF grep; final fix remaps to BLT_I18NCONTEXT_ID_NODETREE (PR #157).
  i18n cleanup: BLT_I18NCONTEXT_ID_METABALL latent reference in interface_template_id.cc removed;
  12 "Abusing id_curve" apology comments cleaned up — these are correct uses of the curve-concept
  context for interpolation/falloff properties; BLT_I18NCONTEXT_ID_CURVE_LEGACY retained as
  intentional hook for community localizers.
  Community i18n architectural decision: Blended core ships no .po catalogs; i18n infrastructure
  is a community-extension hook (BLENDED.md §13). Context constants remap to semantically correct
  msgctxt on type removal, not dropped, so community translations key correctly.
  Build 69 fallout (branch `claude/quality-integrity-standards-ETYzo`): `UI_UL_cache_file_layers`
  linker orphan from the ID_CF chisel — same root cause as Scar 4, four C++ files (declaration,
  definition, sibling declaration, caller) all needed cleanup; the broader sweep then surfaced
  Python orphans from the same incomplete chisel: Transform Cache constraint UI rebuilt around
  the inline-per-instance RNA fields the chisel exposed (`con.filepath`, `con.object_path` —
  velocity/time/layer settings were dropped when the shared CacheFile ID was inlined and not
  restored, so those sub-panels and their 6 panel classes are deleted rather than rebuilt);
  `space_dopesheet.py` had four orphan `bpy.data.*` filter blocks (particles, linestyles,
  textures, cache_files) — collections unregistered from `rna_main.cc` by their respective
  chisels; block removed entirely.
  Scar 14 added to CLAUDE.md (and rewritten once in the same session after the developer caught
  the model producing the failure mode it was trying to prevent): "Common Sense Is Upstream of
  the Rules." Names the actual failure mode AI coding agents produce — treating situations as
  items, generating menus of options about plainly wrong things, applying rules without the
  substrate of common sense that gives rules their meaning. Includes operational expressions of
  common sense (not substitutes for it) and an architectural acknowledgment that the scar does
  not fix the problem because the model that reads the scar is the same generator that produced
  the failure. Triggered initially by a previous session announcing it would hide breakage from
  CI; reframed after this session produced "want me to fix the wrong build number?" as a menu,
  same shape, different surface.
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
