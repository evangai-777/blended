# Blended Changelog

Versioning policy: each minor version (0.x.0) corresponds to a completed
foundation layer from the build order in BLENDED.md §4. Patch releases
(0.x.y) are stable points within a layer — CI fixes, doc updates, build
repairs. 1.0.0 ships when all five foundation layers are honest and basic
pipeline navigation works.

In-flight work lives in the *Unreleased* section below. Design rationale
lives in BLENDED.md. Session instructions live in CLAUDE.md. The README
carries a one-liner status per active item.

**Build order → version map**

| Version | Foundation layer |
|---------|-----------------|
| 0.1.x | Identity — branding, CI, Python compat, design docs |
| 0.2.x | Datablock audit — UI-state removal: ID_WS |
| 0.3.x | Datablock audit — UI-state removals: ID_SCR, ID_WM |
| 0.4.x | Datablock audit — fossil removals (Buckets 5 + 6): 9 ID types |
| 0.5.x | Datablock audit — complete (Bucket 3 fold-downs; 39 → ~19 ID types) |
| 0.6.x | Evaluation model — depsgraph audit |
| 0.7.x | App lenses — launcher as canonical workspace system |
| 0.8.x | File format — `.blended` is the project, import/export is the boundary |
| 1.0.0 | Foundation complete; basic pipeline navigation working |

---

## 0.3.0 — 2026-04-29

`ID_SCR` and `ID_WM` removed from the ID type system (Bucket 4 completions). Both are now runtime-only window state — not project data, not stored in .blended files. CI green (Windows x64, build #49).

### ID_SCR / ID_WM Removal

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entries removed; `ID_SCR_LEGACY`/`ID_WM_LEGACY` `#define` added), `DNA_ID.h` (FILTER/INDEX macros), `DNA_screen_types.h`, `DNA_windowmanager_types.h` (id_type constexpr removed) | ✓ |
| `blenkernel` | `idtype.cc`, `BKE_idtype.hh` (extern/INIT_TYPE removed); `BKE_main.hh` (screens kept as non-indexed runtime listbase); `main.cc` (which_libbase routes ID_SCR_LEGACY/ID_WM_LEGACY; removed from BKE_main_lists_get); `screen.cc` (IDTypeInfo removed, lifecycle made public); `lib_id.cc`, `lib_override.cc`, `lib_query.cc`; `blendfile.cc` (screen swap removed, WM swap via direct std::swap); `grease_pencil_convert_legacy.cc`, `layer.cc`, `sound.cc`, `image.cc` (bmain->screens iteration fixed) | ✓ |
| `blenloader` | `readfile.cc` (skip ID_SCR_LEGACY/ID_SCRN blocks on read); `versioning_270/300/410/440.cc` (dead screen loops deleted — readfile skips these blocks) | ✓ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` (tag functions re-added for runtime lists), `rna_action.cc`, `rna_image.cc`, `rna_movieclip.cc`, `rna_space.cc`, `rna_wm.cc`, `rna_xr.cc` (GS() checks → _LEGACY defines) | ✓ |
| `editors` | `interface.cc`, `interface_handlers.cc`, `interface_icons.cc`, `interface_template_id.cc`, `interface_templates.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `ed_util_ops.cc`; runtime loops converted to window iteration: `ed_util.cc`, `render_update.cc`, `render_preview.cc`, `screen_edit.cc`, `screen_ops.cc`, `workspace_layout_edit.cc`, `armature_naming.cc`, `asset_shelf.cc`, `gizmo_library_utils.cc`, `outliner_sync.cc`, `view3d_view.cc` | ✓ |
| `depsgraph` | `deg_builder_nodes.cc`, `deg_builder_relations.cc` | ✓ |
| `python` | `bpy_rna.cc` (GS() check → _LEGACY), `bpy_rna_context.cc` | ✓ |
| `windowmanager` | `wm.cc` (IDTypeInfo removed; wm_add_default uses BKE_libblock_alloc(ID_WM_LEGACY)); `wm_operators.cc` (case → ID_SCR_LEGACY) | ✓ |
| `asset_system` | `asset_library.cc` (screen loop → window iteration) | ✓ |
| `tests` | `blendfile_loading_base_test.cc` (BKE_id_new template removed; uses BKE_libblock_alloc directly) | ✓ |

**Key design decisions:**
- `bmain->screens` and `bmain->wm` kept as non-indexed runtime listbases — same pattern for both. Removing them from Main caused ~200 compile errors (Scar note added to CLAUDE.md).
- `which_libbase(ID_SCR_LEGACY)` and `which_libbase(ID_WM_LEGACY)` still route to their listbases so `BKE_libblock_alloc` works; they are NOT in `BKE_main_lists_get` so the ID iteration system ignores them.
- `screen_add` uses `BKE_libblock_alloc(bmain, ID_SCR_LEGACY, ...)` since `bScreen::id_type` constexpr was removed.

**Deferred runtime debt:** Screens still have `BKE_screen_blend_read_data` defined (not dead, kept for future format work). Runtime behavior of window-based screen iteration vs full-list iteration may expose edge cases in multi-window scenarios.

---

## Unreleased — 0.4.0

Bucket 5 + 6 fossil removals. 9 ID types, 357 hits, same chisel pattern as 0.3.0.
Chisel order: ID_PC → ID_SPK → ID_PA → ID_GD_LEGACY → ID_LS → ID_MB → ID_TE → ID_CU_LEGACY → ID_CF (last, needs design decision — see CLAUDE.md Key note 8).

**Key notes:**
- `ID_CU_LEGACY` and `ID_GD_LEGACY` have active migration paths — only the type *registration* goes, not the converters.
- `ID_LS` is already guarded by `#ifdef WITH_FREESTYLE` (`WITH_FREESTYLE=OFF` in `blended_release.cmake`).
- `brush_test.cc` uses `ID_TE` and `ID_PC` in test fixtures — those tests get deleted with the types.
- `depsgraph.cc:160` has a `!= ID_PA` guard in `clear_id_nodes_conditional` — particle-specific cache invalidation; audit before removing.

### ID_CF — CacheFile

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entry), `DNA_cachefile_types.h` (id_type constexpr) | ☐ |
| `blenkernel` | `idtype.cc` (INIT_TYPE), `main.cc` (which_libbase case, BKE_main_lists_get entry) | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `io_cache.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `depsgraph` | `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ☐ |

### ID_PC — PaintCurve

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_brush_types.h` (id_type constexpr), `DNA_ID.h` (shared macro checks) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `brush_test.cc` (test fixtures deleted) | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `depsgraph` | `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ☐ |

### ID_SPK — Speaker

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_speaker_types.h` (id_type constexpr), `DNA_object_types.h` (macros) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `object.cc`, `sound.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `depsgraph` | `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ☐ |

### ID_PA — ParticleSettings

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_particle_types.h` (id_type constexpr) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `texture.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_texture.cc`, `rna_particle.cc`, `rna_boid.cc`, `rna_color.cc`, `rna_object_force.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `buttons_context.cc`, `interface_icons.cc`, `interface_template_id.cc`, `render_shading.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `anim_filter.cc`, `anim_channels_defines.cc` | ☐ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_builder_relations.cc`, `deg_builder_nodes.cc`, `depsgraph.cc` (clear_id_nodes_conditional) | ☐ |
| `animrig` | `animdata.cc` | ☐ |

### ID_GD_LEGACY — Old Grease Pencil

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_gpencil_legacy_types.h` (id_type constexpr), `DNA_object_types.h` (macros) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `material.cc`, `deform.cc`, `grease_pencil_convert_legacy.cc` (type-safety asserts only), `blendfile_link_append.cc` | ☐ |
| `blenloader` | `versioning_250.cc`, `versioning_common.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `object_data_transform.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `space_node.cc` | ☐ |
| `draw` | `draw_context.cc` | ☐ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_builder_nodes.cc`, `deg_builder_relations.cc` | ☐ |

### ID_LS — FreestyleLineStyle

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_linestyle_types.h` (id_type constexpr) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `node.cc`, `texture.cc`, `linestyle.cc` | ☐ |
| `blenloader` | `versioning_500.cc`, `versioning_450.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_texture.cc`, `rna_color.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `buttons_texture.cc`, `buttons_context.cc`, `interface_icons.cc`, `interface_template_id.cc`, `interface_template_preview.cc`, `render_shading.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `nodes` | `shader_nodes_inline.cc` | ☐ |
| `depsgraph` | `deg_eval_copy_on_write.cc`, `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ☐ |

### ID_MB — MetaBall

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_meta_types.h` (id_type constexpr), `DNA_object_types.h` (macros shared w/ ID_CU_LEGACY) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `material.cc`, `object.cc`, `object_dupli.cc`, `lib_remap.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `object_data_transform.cc`, `transform_convert_object_texspace.cc` (shared ELEM w/ CU_LEGACY), `render_opengl.cc`, `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `draw` | `overlay_bounds.hh`, `draw_resource.hh` | ☐ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_eval_copy_on_write.cc`, `deg_builder_relations.cc`, `deg_builder_nodes.cc`, `depsgraph_query_iter.cc` | ☐ |

### ID_TE — Texture

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_texture_types.h` (id_type constexpr), `DNA_ID.h` (shared macro) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `preview_image.cc`, `image.cc`, `compositor.cc`, `node.cc`, `brush_test.cc` (test fixtures deleted) | ☐ |
| `blenloader` | `versioning_500.cc`, `versioning_450.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_color.cc`, `rna_image.cc`, `rna_space.cc`, `rna_texture.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `buttons_texture.cc`, `interface_anim.cc`, `interface_icons.cc`, `interface_template_preview.cc`, `interface_template_id.cc`, `node_group_operator.cc`, `render_opengl.cc`, `render_update.cc`, `render_preview.cc`, `anim_filter.cc`, `anim_channels_defines.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_builder_relations.cc`, `deg_builder_nodes.cc`, `deg_eval_copy_on_write.cc` | ☐ |
| `windowmanager` | `wm_operators.cc` | ☐ |
| `modifiers` | `MOD_nodes.cc` | ☐ |

### ID_CU_LEGACY — Legacy Curve

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_curve_types.h` (id_type constexpr), `DNA_object_types.h` (macros shared w/ ID_MB) | ☐ |
| `blenkernel` | `idtype.cc`, `main.cc`, `curve.cc`, `key.cc`, `material.cc`, `object.cc`, `mesh_convert.cc`, `lib_remap.cc`, `object_update.cc` | ☐ |
| `makesrna` | `rna_ID.cc`, `rna_key.cc`, `rna_object.cc`, `rna_main_api.cc` | ☐ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `object_data_transform.cc`, `object_edit.cc`, `render_opengl.cc`, `transform_convert_object_texspace.cc`, `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc` | ☐ |
| `draw` | `overlay_bounds.hh`, `draw_resource.hh` | ☐ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_eval_copy_on_write.cc`, `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ☐ |

---

## 0.2.0 — 2026-04-28

`ID_WS` (WorkSpace) fully removed from every compilation unit. First concrete
structural delta from Blender's data model — WorkSpace goes from a first-class
ID type to nothing. Load-bearing for the launcher model (BLENDED.md §11)
becoming structurally true rather than just conceptually right.

### ID_WS Removal

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_ID.h`, `DNA_workspace_types.h` | ✓ |
| `blenkernel` | `workspace.cc` deleted; `BKE_main.hh`, `idtype.cc`, `main.cc`, `lib_id.cc`, `lib_override.cc`, `blendfile.cc` | ✓ |
| `makesrna` | `rna_ID.cc`, `rna_space.cc`, `rna_main.cc`, `rna_main_api.cc`, `rna_internal.hh` | ✓ |
| `editors` | `interface_template_id.cc`, `ed_util_ops.cc`, `interface_icons.cc`, `workspace_edit.cc`, `render_opengl.cc`, `outliner_edit.cc`, `outliner_draw.cc`, `outliner_tools.cc`, `tree_element_id.cc` | ✓ |
| `depsgraph` | `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ✓ |
| `python` | `bpy_rna.cc`, `bpy_library_load.cc` | ✓ |
| `windowmanager` | `wm.cc` | ✓ |

CI green (Windows x64, build #45). `grep -rn "ID_WS" source/` returns zero hits.

**Deferred runtime debt:** workspace cycle, reorder operators, and factory name
translation were left as compile-clean stubs. These won't surface in CI until
the architecture question — where does the workspace list live now that
`bmain->workspaces` is gone? — is answered. Documented in CLAUDE.md Scar 1.

---

## Roadmap

### 0.2.x — Datablock audit: UI-state (ID_WS)

**0.2.0 — ID_WS removed** *(see above)*

---

### 0.3.x — Datablock audit: UI-state (ID_SCR, ID_WM)

**0.3.0 — ID_SCR and ID_WM removed** *(see above)*

`bScreen` and `WindowManager` are Bucket 4 (BLENDED.md §10) — per-user,
per-machine state that currently leaks into `.blend` files. Removal scope was
larger than ID_WS (both are more deeply wired into the window system).

---

### 0.4.x — Datablock audit: fossils (Buckets 5 + 6)

Remove Bucket 5 (upstream deprecations Blender itself marked done) and Bucket 6
(fossils with no active users).

**Bucket 5** — finish what upstream started:
- `ID_CU_LEGACY` — legacy Curve, replaced by `ID_CV`
- `ID_GD_LEGACY` — legacy Grease Pencil, replaced by `ID_GP`

**Bucket 6** — cut:
- `ID_TE` — Blender Internal renderer fossil; residual folds into NodeTree
- `ID_PA` — ParticleSettings; replaced by Geometry Nodes
- `ID_MB` — MetaBall; sculpt/remesh covers it
- `ID_LS` — FreestyleLineStyle; NPR via shader nodes / Grease Pencil
- `ID_SPK` — Speaker; positional audio on scene objects. Audio flows through the timeline.
- `ID_PC` — PaintCurve; niche stroke guide
- `ID_CF` — CacheFile; external Alembic/USD cache reference — boundary concern, not project data

Each fossil follows the same chisel pattern as ID_WS / ID_SCR / ID_WM.
The breakage is the audit.

---

### 0.5.x — Datablock audit: complete (Bucket 3 fold-downs)

Fold-downs from Bucket 3 — property bags pretending to be first-class IDs:

- `ID_BR` → user state + shareable brush packs
- `ID_PAL` → brush property or inline
- `ID_LT` → modifier, not a datablock
- `ID_LP` → merge into `ID_LA` with a type flag
- `ID_MSK` → hang off compositor NodeTree
- `ID_VF` → system font reference; FreeType handles the rest

Open questions also resolved here: `ID_WO` (keep as reusable environment asset or fold
into Scene properties?), `ID_KE` (survey real projects before collapsing into geometry).

**Milestone:** 39 → ~19 ID types. No legitimate scope lost.

---

### 0.6.x — Evaluation model

Depsgraph audit under Blended's scope. Current depsgraph has had three rewrites
and carries assumptions from all three eras. Questions:

- What evaluation paths actually exist in Blended's scope?
- Which depsgraph node types survive the datablock cuts above?
- What's kept because it's right vs kept because removing it is hard?

The ID type cuts in 0.2–0.5 will have already removed dead branches from the
depsgraph. This milestone cleans up what remains.

---

### 0.7.x — App lenses (launcher)

The launcher model from BLENDED.md §11 becomes structurally real:

- Launcher as a single vertical scrollable view (pipeline sections as bold headings,
  mode buttons under each)
- Each mode button opens a filtered view of the same project — the `.blended` file
  is one file; the mode controls what's visible
- No `ID_WS` datablock (removed in 0.2.x) means the launcher *is* the canonical
  workspace system, not a parallel one competing with it

Fast intra-section mode switching. Project state reflected in the launcher.
Global hotkey to return from any workspace.

---

### 0.8.x — File format

`.blended` is the project, period. Import/export is an explicit boundary, not a
default workflow.

- Collapse to one `.blended` — drop userpref-as-blend and startup-as-blend
- Format audit (§5 Groups 2–6): cut dead interchange formats, collapse sim caches
  into project data, resolve the external-`.py` / text-datablock split
- Asset library design: library = directory of `.blended` files with a real asset
  primitive inside (or true external linking — TBD per §9 open questions)

---

### 1.0.0 — Foundation complete

All five foundation layers honest. Basic pipeline navigation working. A user can
walk up, pick a section from the launcher, do real creative work, and save a
`.blended` file that travels cleanly to another machine.

Not "feature-complete." Scope is wide; 1.0 is the point where the shape of the
rebuild is true. Post-1.0 work fills in pipeline sections, adds modes, and
follows the community where Blended's scope takes it.

---

## 0.1.0

First independent Blended release. Based on Blender 5.2 alpha.

### Branding

- CMake project renamed to `Blended`
- Window titles: `Blended 0.1.0`
- Splash screen: name `Blended`, tagline *"Blender, simplified."*
- About dialog updated to Blended
- Windows file metadata (ProductName, FileDescription, CompanyName) updated
- Linux desktop entry (Name, Comment, StartupWMClass) updated

### Version Identity

- `BLENDED_VERSION_MAJOR`, `BLENDED_VERSION_MINOR`, `BLENDED_VERSION_PATCH` defines
  in `BKE_blender_version.h`, independent of Blender's `BLENDER_VERSION` integer
  (which stays at 502 for `.blend` file format compatibility)
- `BKE_blended_version_string()` — Blended-specific version string, used in window
  titles and splash screen
- CI artifact named `Blended-0.1.0-windows-x64.zip`

### Pre-5.0 Rig Compatibility

- `scripts/startup/blended_rig_compat.py` — restores `action.fcurves` as a Python
  property on `bpy.types.Action`
- `_FCurvesCompat` proxy flattens F-Curves from all channelbags across all
  layers/strips in Blender 5.x's layered action system
- Pre-Blender-5.0 Rigify rigs (including CGCookie Vonnbots rigs) that access
  `action.fcurves` directly work again
- IK/FK bake operators no longer fail silently

### Update Notifications

- `scripts/startup/blended_update_check.py` — background GitHub Releases check
  at startup (non-blocking, 24-hour cache)
- Top-bar notification with version string when an update is available
- One-click download via browser (`BLENDED_OT_open_update_page`)
- `BLENDED_PT_update_prefs` panel in System Preferences → System

### CI

- Windows x64 portable `.zip` builds via GitHub Actions (`build-windows.yml`)
- Manual dispatch (`workflow_dispatch`) for development builds
- Tag push (`v*`) for release builds → artifact + GitHub Release
- `blended_release.cmake` — inherits `blender_release.cmake`, disables GPU kernel
  pre-compilation (CUDA/HIP/OneAPI) and Freestyle for CI runners
- LFS handled explicitly: source via `projects.blender.org/blender/blender.git`,
  libraries via `projects.blender.org/blender/lib-windows_x64.git`

### Documentation

- `BLENDED.md` — full design document: identity (§1–§3), methodology (§4), file
  format principles (§5), pipeline as UX (§11), detailed specs for all eight
  pipeline sections (§12.1–§12.8: Storyboarding, 2D Animation, 3D Animation,
  Game, Design, Finalizing, Compositing, Audio), datablock audit (§10), guardrails (§8)
- `CLAUDE.md` — operational context for Claude sessions
- `UPSTREAM_SYNC.md` — upstream merge workflow and conflict-prone files
- `PHILOSOPHY.md` — project philosophy
- `wtf.md` — who the developer is and how to work with them
