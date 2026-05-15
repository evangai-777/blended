# Blended Changelog

Versioning policy: each minor version (0.x.0) corresponds to a completed
foundation layer from the build order in BLENDED.md §4. Patch releases
(0.x.y) are stable points within a layer — CI fixes, doc updates, build
repairs. 1.0.0 ships when all six foundation layers are honest and basic
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
| 0.7.x | App lenses — launcher as canonical workspace system + full product identity |
| 0.8.x | File format — `.blended` is the project, import/export is the boundary |
| 0.9.x | `.blend` import — seamless read with dropped-data manifest output |
| 1.0.0 | Foundation complete; basic pipeline navigation working |

---

## 0.3.0 — 2026-04-29

`ID_SCR` and `ID_WM` removed from the ID type system (Bucket 4 completions). Both are now runtime-only window state — not project data, not stored in .blended files. CI green (Windows x64, build 49).

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

## Unreleased — 0.6.0

**Design vision:** 0.5.0 declared the ~19-type data model. 0.6.x makes the evaluation and draw layers honest about it.

The 0.5.0 datablock audit stripped the ID registration machinery for 12 removed and 6 folded-down types, but the consequences of that surgery haven't fully propagated. The depsgraph builders, viewport draw layer, and editor dispatch still carry the old 39-type topology: case statements for types that are no longer first-class, OOB guards added as temporary scaffolding during fold-downs, `→ true` workarounds (e.g. EEVEE probe callers after ID_LP), and missed Scar 19 enum-demotion side effects. The skeleton of the old world is still visible under the skin of the new one.

0.6.x closes that seam. This is not new removals — the removals are done. It is the intentional folding-in of what 0.5.0 already decided, propagated through the depsgraph and viewport subsystems. Once the internals match the declared data model, the evaluation layer is honest, and 0.7.x additive work (launcher, product identity) can be built on a clean foundation.

**Scope note:** Not a redesign of the depsgraph architecture. The depsgraph is the core animation engine (BLENDED.md §2 — non-negotiable). The audit makes it honest for the ~19-type world, not rebuilt.

---

**Pre-implementation blast radius audit (grepped 2026-05-15). ~95 total hits across depsgraph, draw, and editor layers.**

Depsgraph layer — 34 hits across 5 files:

| File | Hits | Classification |
|------|------|----------------|
| `deg_builder_nodes.cc` | 10 | 8× fold-down case dispatch (ID_LP, ID_MSK, ID_CU_LEGACY, ID_LT, ID_GD_LEGACY, ID_VF, ID_BR+PAL+assert); 2× ID_TE OOB scaffold comments |
| `deg_builder_relations.cc` | 16 | 12× fold-down case dispatch; 3× ID_TE OOB scaffold comments; 1× geometry comment |
| `depsgraph_query.cc` | 2 | OOB guards in `DEG_id_type_any_exists` + `DEG_id_type_updated` — `if (id_type_index < 0) return false` |
| `depsgraph_tag.cc` | 5 | 4× fold-down dispatch (ID_CU_LEGACY, ID_LT, ID_LP, ID_GD_LEGACY, ID_PAL, ID_MSK); 1× OOB guard |
| `depsgraph.cc` | 1 | OOB guard on `id_type_exist` write |

Draw/EEVEE layer — 4 hits across 4 files:

| File | Line | Hit | Classification |
|------|------|-----|----------------|
| `eevee_lightprobe_planar.cc` | 54 | `→ true` (was `DEG_id_type_any_exists(depsgraph, ID_LP)`) | (c) → true workaround |
| `eevee_lightprobe_sphere.cc` | 24 | `→ true` (was `DEG_id_type_any_exists(depsgraph, ID_LP)`) | (c) → true workaround |
| `overlay_bounds.hh` | 180 | `case ID_CU_LEGACY:` texspace | (a) fold-down dispatch — live |
| `draw_resource.hh` | 149 | `case ID_CU_LEGACY:` orco data | (a) fold-down dispatch — live |

Editors layer — ~57 hits across 14 files (all fold-down case dispatch unless noted):

| File(s) | Hits | Notes |
|---------|------|-------|
| `interface_template_id.cc` | 9 | Browse dialogs for ID_CU_LEGACY, ID_LT, ID_BR, ID_MSK, ID_PAL, ID_LP, ID_VF — live UI |
| `interface_icons.cc` | 8 | Icon dispatch + 1× `GS != ID_SCR_LEGACY` check (e) |
| `outliner_draw/intern/tools/tree_element_id.cc` | ~30 | All fold-down dispatch; 1× ID_SCR_LEGACY comment (e) |
| `render_opengl.cc` | 7 | Whitelist/blacklist for fold-down types — live |
| `interface.cc` | 1 | `GS == ID_SCR_LEGACY` (e) — verify vs vestigial |
| `interface_templates.cc` | 1 | `GS != ID_SCR_LEGACY` (e) — verify vs vestigial |
| `space_image.cc:1214`, `space_clip.cc:1186` | 2 | `FILTER_ID_MSK` (d) — intentionally kept, Scar 19 |
| `render_preview.cc:1289` | 1 | `#if 0` block `id_type == ID_TE` (e) — dead code, remove |

Classification totals:

| Class | Count | Action |
|-------|-------|--------|
| (a) Fold-down dispatch — runtime, intentional | ~71 | **Stays.** Live runtime code per fold-down protocol. Same rule as blenkernel. |
| (b) OOB scaffolding guards | 5 | **Confirm as permanent.** Unregistered types correctly return false/no-op. Update comments from "scaffolding" to permanent design. |
| (c) → true workarounds | 2 | **Evaluate or document.** EEVEE probe callers: replace with direct check, or confirm as permanent conservative-update strategy. |
| (d) Scar 19 FILTER_ID refs | 2 | **Stays.** `FILTER_ID_MSK` in space_image/space_clip intentionally kept per Scar 19. |
| (e) Lingering chiseled-type refs | 5 | **Small cleanup.** `ID_SCR_LEGACY` in 3 editor files (verify correct use vs vestigial); `ID_TE` `#if 0` dead code in render_preview.cc (remove). |

**Key finding:** The fold-down dispatch in the depsgraph builders (`case ID_LP:` → `build_lightprobe()`, `case ID_MSK:` → `build_mask()`, `case ID_LT:`, `case ID_VF:`, etc.) is **live runtime code** — EEVEE probes render, compositor masks work, text objects display, lattice deformers deform. These are the Scar 2 equivalent for the depsgraph layer: kept because the functionality is alive, not because the removal is incomplete. 0.6.x does not touch them.

The actual 0.6.x seam closure is the (b), (c), (e) items: ~10 lines of dead code, 5 guard comments to re-document as permanent, and 2 EEVEE workarounds to resolve or lock in.

**0.6.x action items (from audit):**
1. `depsgraph_query.cc` — confirm OOB guards as permanent architecture; update comments from "scaffolding" to "Blended permanent: unregistered types return false here."
2. `depsgraph.cc`, `depsgraph_tag.cc` — same OOB guard documentation pass.
3. `eevee_lightprobe_planar.cc:54`, `eevee_lightprobe_sphere.cc:24` — evaluate `→ true` workaround: replace with direct probe-exists check, or document as permanent conservative-update.
4. `render_preview.cc:1289` — remove `#if 0` ID_TE dead code block.
5. `interface.cc:1543`, `interface_icons.cc:1220`, `interface_templates.cc:85` — verify ID_SCR_LEGACY uses are correct allocation-path references, not vestigial.
6. `deg_builder_nodes.cc:642-643`, `deg_builder_relations.cc:583-584` — document ID_BR/ID_PAL BLI_assert dispatch as intentional: brushes/palettes must not build depsgraph nodes.

---

## 0.5.0 — 2026-05-14

**CI-complete: Windows x64, build 81, commit `d6ee8478`.**

Bucket 3 fold-downs — the last datablock-audit version. Six IDs that are property bags pretending to be first-class entities, each folded into the structure they actually belong to: `ID_BR` (Brush) → user state + shareable brush packs, `ID_PAL` (Palette) → brush property or inline, `ID_LT` (Lattice) → modifier, not a datablock, `ID_LP` (LightProbe) → merge into `ID_LA` with a type flag, `ID_MSK` (Mask) → hang off compositor NodeTree, `ID_VF` (VFont) → system font reference. Closes the datablock audit (39 → ~19 ID types).

Fold-down order: **ID_LP ✓** → **ID_PAL ✓** → **ID_LT ✓** → **ID_MSK ✓** → **ID_VF ✓** → **ID_BR ✓**. All Bucket 3 fold-downs complete. Datablock audit closed: 39 → ~19 ID types.

### ID_LP — LightProbe ✓ complete

**Pre-fold-down blast radius audit (35 literal / ~40 true hits):**

makesdna (5 files): `DNA_ID_enums.h:154` (enum entry → deprecated #define); `DNA_lightprobe_types.h:113` (entire `#ifdef __cplusplus` block — Scar 8); `DNA_ID.h:1179,1195,1271` (FILTER_ID_LP, FILTER_ID_ALL, INDEX_ID_LP); `DNA_action_types.h:447` (ADS_FILTER_NOLIGHTPROBE); `DNA_object_types.h:740,752` (NO CHANGE — compiles via deprecated #define).

blenkernel (3 files): `BKE_idtype.hh:326` (extern IDTypeInfo IDType_ID_LP); `idtype.cc:163` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4); `main.cc:152,1090` (CASE_ID_INDEX + lb[] removed; which_libbase `case ID_LP:` KEPT — Scar 2); `lightprobe.cc:51–55` (IDTypeInfo block removed; BKE_lightprobe_add → Scar 10 MEM_new).

Scar 2 — kept (mandatory): `BKE_main.hh` (lightprobes field); `main.cc` which_libbase routing; `versioning_280.cc` (×2), `versioning_400.cc` (×3), `versioning_410.cc` (×1), `versioning_420.cc` (×1), `versioning_500.cc` (×1) — all kept as Scar 2 bridge.

makesrna (8 files): `rna_ID.cc:46,131,407,479` (×4); `rna_main_api.cc:765` (RNA_def_main_lightprobes + collection accessor); `rna_main.cc` (listbase funcs + table entry); `rna_internal.hh` (RNA_def_main_lightprobes removed; RNA_def_lightprobe KEPT — Scar 15); `rna_action.cc:1558–1559` (show_lightprobes + ADS_FILTER_NOLIGHTPROBE); `rna_space.cc:3958` (FILTER_ID_LP); `BLT_translation.hh:124,198` (BLT_I18NCONTEXT_ID_LIGHTPROBE — Scar 13); `interface_template_id.cc:968` (BLT_I18N_MSGID_MULTI_CTXT entry).

editors — standard sweep (KEPT per fold-down protocol): `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_select.cc`, `outliner_tools.cc`, `tree_element_id.cc`, `buttons_context.cc`.

editors — anim chain (KEPT per fold-down protocol): `anim_channels_defines.cc` (ACF_DSLIGHTPROBE), `anim_channels_edit.cc` (9 cases), `anim_filter.cc` (function + dispatch; only ADS_FILTER_NOLIGHTPROBE block removed — forced by DNA removal), `anim_deps.cc`, `ED_anim_api.hh`, `nla_buttons.cc`, `nla_draw.cc`, `nla_tracks.cc`, `transform_convert_action.cc`.

depsgraph (3 literal + 3 true blast radius): `deg_builder_nodes.cc:597` (case ID_LP: KEPT); `deg_builder_relations.cc:538` (KEPT); `depsgraph_tag.cc:622` (removed); `depsgraph_query.cc:134` (OOB guard added — true blast radius); `eevee_lightprobe_planar.cc:54` (→ `true` — true blast radius); `eevee_lightprobe_sphere.cc:24` (→ `true` — true blast radius).

python (4 entries): `space_dopesheet.py:125–126`; `space_outliner.py:528`; `wm.py:2946`; `_bl_i18n_utils/settings.py:457`.

Runtime code — all KEPT (fold-down, not chisel): all `eevee_lightprobe_*.cc/.hh`, `overlay_lightprobe.hh`, `object_add.cc` (OBJECT_OT_lightprobe_add), `render_shading.cc`, `properties_data_lightprobe.py`, `properties_world.py`, `rna_lightprobe.cc`, `DNA_lightprobe_types.h` (struct body), `BKE_lightprobe.h`.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entry removed, deprecated `#define` added), `DNA_lightprobe_types.h` (Scar 8: entire `#ifdef __cplusplus` block removed), `DNA_ID.h` (`FILTER_ID_LP`, `INDEX_ID_LP`, `FILTER_ID_ALL` inclusion removed), `DNA_action_types.h` (`ADS_FILTER_NOLIGHTPROBE` removed from `eDopeSheet_FilterFlag2`) | ✓ |
| `blenkernel` | `BKE_idtype.hh` (extern decl removed), `idtype.cc` (`INIT_TYPE(ID_LP)` + both `CASE_IDINDEX(LP)` entries removed — Scar 4), `main.cc` (`CASE_ID_INDEX(INDEX_ID_LP)` + `lb[INDEX_ID_LP]` removed; `case ID_LP:` which_libbase routing KEPT — Scar 2), `lightprobe.cc` (`IDTypeInfo IDType_ID_LP` + static callbacks removed; `BKE_lightprobe_add` rewritten with `MEM_new<LightProbe>` + manual listbase insert — Scar 10; `BLO_read_write.hh` added explicitly for cache blend write/read functions — Scar 17) | ✓ |
| `makesrna` | `rna_ID.cc` (RNA enum item, FILTER_ID_LP filter item, base_type check, `case ID_LP:` switch case removed), `rna_main_api.cc` (`rna_Main_lightprobe_new`, `RNA_MAIN_ID_TAG_FUNCS_DEF`, `RNA_def_main_lightprobes`, DNA/BKE includes removed), `rna_main.cc` (`RNA_MAIN_LISTBASE_FUNCS_DEF(lightprobes)` + table entry removed), `rna_internal.hh` (`RNA_def_main_lightprobes` declaration removed; `RNA_def_lightprobe` struct RNA declaration KEPT — Scar 15), `rna_action.cc` (`show_lightprobes` + `ADS_FILTER_NOLIGHTPROBE` RNA prop removed), `rna_space.cc` (`FILTER_ID_LP` removed from environment category filter), `BLT_translation.hh` (define + ITEM removed), `interface_template_id.cc` (`BLT_I18NCONTEXT_ID_LIGHTPROBE` from `BLT_I18N_MSGID_MULTI_CTXT` — Scar 13) | ✓ |
| `editors/anim` | `anim_filter.cc`: removed `if (ads_filterflag2 & ADS_FILTER_NOLIGHTPROBE) { return 0; }` (forced by DNA removal). All other anim chain code KEPT: `ANIMTYPE_DSLIGHTPROBE`, `ACF_DSLIGHTPROBE`, all switch cases in anim_channels_edit/deps/nla_*/transform_convert_action — fold-down keeps runtime anim. | ✓ |
| `depsgraph` | `depsgraph_query.cc`: OOB guard added to `DEG_id_type_any_exists` and `DEG_id_type_updated` — `BKE_idtype_idcode_to_index(ID_LP)` returns -1 after INIT_TYPE removal; guard returns `false` safely. All `case ID_LP:` dispatch in deg_builder_nodes/relations KEPT — LightProbe depsgraph evaluation still runs. | ✓ |
| `draw/eevee` | `eevee_lightprobe_planar.cc:54`, `eevee_lightprobe_sphere.cc:24`: `DEG_id_type_any_exists(depsgraph, ID_LP)` → `true` — OOB guard would return false (LP not in index system), killing probe updates. Conservative always-update is correct. | ✓ |
| `python` | `space_dopesheet.py` (bpy.data.lightprobes check + show_lightprobes prop removed), `space_outliner.py` (bpy.data.lightprobes from outliner filter), `wm.py` (LIGHT_PROBE enum + collection mapping from copy-to-selected operator), `_bl_i18n_utils/settings.py` ("lightprobes" data path) | ✓ |

**What stays (fold-down keeps all runtime code):** `rna_lightprobe.cc` (LightProbe RNA struct props), `BKE_lightprobe.hh` (allocation + cache API), `lightprobe.cc` (all non-IDTypeInfo functions), all editor dispatch for icons/outliner/template_id/buttons_context/render_opengl, full anim channel chain (ANIMTYPE_DSLIGHTPROBE, ACF_DSLIGHTPROBE, all case statements), full depsgraph builder dispatch (build_lightprobe, build_object_data_lightprobe, relations), all blenloader versioning loops over bmain->lightprobes (5 files, Scar 2 bridge). `use_duplicate_lightprobe` in space_userpref.py KEPT (property lives in rna_userdef.cc, not tied to bpy.data collection).

**Claude AI contributor (2026-05-13):** ID_LP fold-down across all 7 layers in one session on branch `claude/review-claude-md-7cDGm`. 5 commits pushed. Key distinction from prior chisel sessions: fold-down mindset confirmed — the standard editor sweep and anim chain removal that would apply to a chisel were correctly NOT applied here. Only compile-forced changes (ADS_FILTER_NOLIGHTPROBE, OOB crash, bpy.data collection gone) were made. All runtime workflows intact.

**ID_LP CI fixes (2026-05-13):** Four post-fold-down CI failures resolved across two PRs (#168, #169) on branches `claude/fix-rna-lightprobe-NoI0x` and `claude/0.5.0-idlp-folddown-ci-fixes`. Failures: (1) `RNA_def_lightprobe` declaration stripped from `rna_internal.hh` alongside `RNA_def_main_lightprobes` — only the collection accessor should go (Scar 15). (2) `BKE_lightprobe_add` Scar 10 allocator stored `which_libbase` result as `ListBase*` instead of `ListBaseT<ID>*` (Scar 16). (3) Removing IDTypeInfo dropped implicit `BLO_read_write.hh` include chain — cache blend write/read functions needed explicit include (Scar 17). (4) `MEM_new_zeroed<LightProbe>` on a non-trivial type — `LightProbe` has in-class initializers throughout; corrected to `MEM_new<LightProbe>` (Scar 18). CI green: Windows x64, build 74, commit `80002ae`.

**Version bump (2026-05-13):** `BLENDED_VERSION_MINOR` updated from 3 to 4 in `BKE_blender_version.h`. Was stuck at 0.3.0 since the 0.4.0 CI-complete milestone (build 70). CI workflow reads this dynamically — packaged artifacts now correctly labelled `Blended-0.4.0-windows-x64`.

**Version bump (2026-05-13):** `BLENDED_VERSION_MINOR` updated from 4 to 5. Per the new Version Management rule (CLAUDE.md — Version Management section): bump MINOR on the first commit of a new dev cycle, not at CI green. The 0.5.0 dev cycle started at commit `46af9695` (ID_LP makesdna layer); the header should have read 0.5.0 from that point. Immediately corrected after the rule was codified. Packaged artifacts now correctly labelled `Blended-0.5.0-windows-x64`.

### ID_PAL — Palette ✓ complete (pending CI)

**Pre-fold-down blast radius audit (38 literal / ~46 true hits):**

makesdna (3 files): `DNA_ID_enums.h:152` (enum entry → deprecated `#define`); `DNA_brush_types.h:466-474` (entire `#ifdef __cplusplus` block — Scar 8); `DNA_ID.h` (`FILTER_ID_PAL`, `INDEX_ID_PAL`, removed from `FILTER_ID_ALL`). `DNA_ID.h:695` `!ELEM` macro keeps `ID_PAL` — still compiles via deprecated `#define`, semantically correct.

blenkernel (5 files): `BKE_idtype.hh:325` (extern removed); `idtype.cc:162` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4); `main.cc:154,1061,1087` (CASE_ID_INDEX + lb[] ×2 removed; `case ID_PAL:` which_libbase routing KEPT — Scar 2); `paint.cc` (IDTypeInfo IDType_ID_PAL removed; `BKE_palette_add` → Scar 10 MEM_new<Palette>); `scene.cc:1612` (FILTER_ID_PAL from Scene IDTypeInfo dependencies_id_types).

Scar 2 — kept (mandatory): `BKE_main.hh` (palettes field); `main.cc` which_libbase routing; `anim_data_bmain_utils.cc:110`; `anim_sys.cc:4155`; `gpencil_legacy.cc:1170,1174`; `versioning_500.cc:3950`; `versioning_290.cc:843` — all kept as Scar 2 bridge.

makesrna (7 files): `rna_ID.cc:52,144,393,464` (×4 — enum item, filter item, base_type check, switch case); `rna_main_api.cc:602,740,1648` (rna_Main_palettes_new, RNA_MAIN_ID_TAG_FUNCS_DEF, RNA_def_main_palettes all removed); `rna_main.cc:170,433` (RNA_MAIN_LISTBASE_FUNCS_DEF + table entry); `rna_internal.hh:544` (RNA_def_main_palettes removed; RNA_def_palette KEPT — Scar 15); `rna_space.cc:3963` (FILTER_ID_PAL from category_misc); `BLT_translation.hh:130,203` (define + ITEM — Scar 13, no external borrowers); `interface_template_id.cc:974` (BLT_I18N_MSGID_MULTI_CTXT entry — Scar 13 sweep).

python (1 file): `_bl_i18n_utils/settings.py:468` ("palettes" data path). No space_dopesheet.py / space_outliner.py / wm.py changes — ID_PAL had no ADS_FILTER_NOPALETTE, no copy-to-selected entry, no outliner filter entry.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum removed, deprecated `#define` added), `DNA_brush_types.h` (Scar 8: `#ifdef __cplusplus` block removed), `DNA_ID.h` (`FILTER_ID_PAL`, `INDEX_ID_PAL`, FILTER_ID_ALL entry removed) | ✓ |
| `blenkernel` | `BKE_idtype.hh` (extern removed), `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4), `main.cc` (CASE_ID_INDEX + lb[] ×2 removed; which_libbase `case ID_PAL:` KEPT — Scar 2), `paint.cc` (IDTypeInfo block removed; BKE_palette_add → MEM_new<Palette> + manual insert — Scar 10; id_fake_user_set replicated explicitly), `scene.cc` (FILTER_ID_PAL from dependencies_id_types) | ✓ |
| `makesrna` | `rna_ID.cc` (4 entries), `rna_main_api.cc` (new + tag funcs + collection accessor), `rna_main.cc` (listbase funcs + table entry), `rna_internal.hh` (RNA_def_main_palettes removed; RNA_def_palette KEPT — Scar 15), `rna_space.cc` (FILTER_ID_PAL from category_misc), `BLT_translation.hh` (define + ITEM — Scar 13), `interface_template_id.cc` (BLT_I18N_MSGID_MULTI_CTXT — Scar 13 sweep) | ✓ |
| `python` | `_bl_i18n_utils/settings.py` ("palettes" data path removed) | ✓ |

**What stays (fold-down keeps all runtime code):** `rna_palette.cc`, `palette.cc` (editors — 646 lines of paint/palette UI), `BKE_paint.hh` (BKE_palette_add, BKE_paint_palette, BKE_paint_palette_set), all editor dispatch (icons, outliner, template_id browse string, render_opengl), `ANIMTYPE_PALETTE` enum stub, depsgraph `case ID_PAL:` dispatch in both builders + depsgraph_tag.cc, all blenloader versioning paths over bmain->palettes (Scar 2 bridge). `makesrna.cc:4046` `{"rna_palette.cc", nullptr, RNA_def_palette}` entry kept.

**Claude AI contributor (2026-05-13):** ID_PAL fold-down across 4 layers on branch `claude/review-docs-history-l0OGD`. 4 code commits + 1 docs commit. Cleaner than ID_LP: no forced anim_filter.cc change (no ADS_FILTER_NOPALETTE), no depsgraph OOB fixes needed (guards already generic from ID_LP fold-down), no eevee callers. Scar 10 id_fake_user_set explicit replication is the one detail specific to Palette. Pending CI.

---

### ID_LT — Lattice ✓ complete (pending CI)

**Pre-fold-down blast radius audit (70 literal / ~90 true hits):**

makesdna (3 files): `DNA_ID_enums.h:136` (enum entry → deprecated `#define`); `DNA_lattice_types.h:53-57` (Scar 8 partial: remove only `id_type = ID_LT` line — keep `#ifdef __cplusplus` guard + `DNA_DEFINE_CXX_METHODS(Lattice)`); `DNA_ID.h` (`FILTER_ID_LT`, `INDEX_ID_LT`, removed from `FILTER_ID_ALL`). `DNA_object_types.h` ELEM macros keep `ID_LT` — compile via deprecated `#define`, semantically correct (valid OB_LATTICE runtime check).

blenkernel (5 files): `BKE_idtype.hh:308` (extern removed — `lattice_deform_test.cc` uses IDType_ID_LT but inside `#if DO_PERF_TESTS 0`, dead code); `idtype.cc:145` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4); `main.cc:149,1076` (CASE_ID_INDEX + lb[] removed; `case ID_LT:` which_libbase routing KEPT — Scar 2; only one `BKE_main_lists_get` copy confirmed); `lattice.cc` (IDTypeInfo IDType_ID_LT removed; `BKE_lattice_add` → Scar 10 `MEM_new<Lattice>` + manual insert; static blend I/O callbacks kept for 0.9.x, `BLO_read_write.hh` retained — Scar 17 pattern); `key.cc:173` (`FILTER_ID_LT` removed from Key IDTypeInfo `dependencies_id_types` — compile-error site).

Scar 2 — kept (mandatory): `BKE_main.hh` (lattices field); `main.cc` which_libbase routing; `anim_data_bmain_utils.cc` (bmain->lattices.first); `anim_sys.cc` (main->lattices.first); `versioning_250.cc:960`; `versioning_legacy.cc:1352,2385` — all kept as Scar 2 bridge.

makesrna (7 files): `rna_ID.cc` (×4 — enum item, filter item, base_type check, switch case); `rna_main_api.cc` (`rna_Main_lattices_new`, `RNA_MAIN_ID_TAG_FUNCS_DEF`, `RNA_def_main_lattices` all removed); `rna_main.cc` (`RNA_MAIN_LISTBASE_FUNCS_DEF(lattices)` + table entry); `rna_internal.hh` (`RNA_def_main_lattices` removed; `RNA_def_lattice` + `RNA_api_lattice` KEPT — Scar 15); `rna_space.cc:3942` (`FILTER_ID_LT` from `category_geometry`); `BLT_translation.hh:121,193` (define + ITEM — Scar 13, only one borrower: `interface_template_id.cc:966`); `interface_template_id.cc:966` (BLT_I18N_MSGID_MULTI_CTXT entry — Scar 13 sweep).

python (4 files): `_bl_i18n_utils/settings.py:455` ("lattices" data path); `space_dopesheet.py:117` (`if bpy.data.lattices:` guard removed, `show_lattices` prop kept unconditional — `ADS_FILTER_NOLAT` stays, prop is valid); `space_outliner.py:528` (`bpy.data.lattices or` removed); `_bpy_types.py:141` ("lattices" from attr_links). `bl_ui/__init__.py`, `space_userpref.py`, `space_view3d.py`, `properties_data_lattice.py` all kept.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum removed, deprecated `#define` added), `DNA_lattice_types.h` (Scar 8 partial: `id_type` line only removed, guard + CXX methods kept), `DNA_ID.h` (`FILTER_ID_LT`, `INDEX_ID_LT`, FILTER_ID_ALL entry removed) | ✓ |
| `blenkernel` | `BKE_idtype.hh` (extern removed), `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4), `main.cc` (CASE_ID_INDEX + lb[] removed; which_libbase `case ID_LT:` KEPT — Scar 2), `lattice.cc` (IDTypeInfo block removed; BKE_lattice_add → MEM_new<Lattice> + manual insert — Scar 10; no id_fake_user_set; BLO_read_write.hh retained — Scar 17), `key.cc` (FILTER_ID_LT from dependencies_id_types) | ✓ |
| `makesrna` | `rna_ID.cc` (4 entries), `rna_main_api.cc` (new + tag funcs + collection accessor), `rna_main.cc` (listbase funcs + table entry), `rna_internal.hh` (RNA_def_main_lattices removed; RNA_def_lattice + RNA_api_lattice KEPT — Scar 15), `rna_space.cc` (FILTER_ID_LT from category_geometry), `BLT_translation.hh` (define + ITEM — Scar 13), `interface_template_id.cc` (BLT_I18N_MSGID_MULTI_CTXT — Scar 13 sweep) | ✓ |
| `python` | `_bl_i18n_utils/settings.py`, `space_dopesheet.py` (guard removed, prop kept), `space_outliner.py` (bpy.data.lattices condition removed), `_bpy_types.py` (attr_links) | ✓ |

**What stays (fold-down keeps all runtime code):** `lattice.cc`, `editlattice_select.cc`, `editlattice_tools.cc`, `editlattice_undo.cc`, `lattice_ops.cc` (entire editors/lattice/ subsystem), `rna_lattice.cc`, `rna_lattice_api.cc`, `draw_cache_impl_lattice.cc`, `overlay_lattice.hh`, `transform_convert_lattice.cc`, `MOD_lattice.cc`, `MOD_grease_pencil_lattice.cc`, `BKE_lattice.hh`, depsgraph `case ID_LT:` dispatch in both builders + `depsgraph_tag.cc`, all blenloader versioning paths over `bmain->lattices` (Scar 2 bridge), full anim chain (`ANIMTYPE_DSLAT`, `ACF_DSLAT`, `animdata_filter_ds_lat`, `ADS_FILTER_NOLAT`, `show_lattices` RNA prop). `makesrna.cc:4026` `{"rna_lattice.cc",..., RNA_def_lattice}` entry kept.

**Claude AI contributor (2026-05-13):** ID_LT fold-down across 4 layers on branch `claude/review-docs-history-l0OGD`. 4 code commits + 1 docs commit. Key distinction from previous fold-downs: anim chain fully kept (ANIMTYPE_DSLAT, ACF_DSLAT, ADS_FILTER_NOLAT — all live runtime code); Scar 8 partial (DNA_DEFINE_CXX_METHODS stays in the guard, unlike PAL/LP where the entire block went); lattice_deform_test.cc IDType_ID_LT usage dead code (#if DO_PERF_TESTS 0); key.cc FILTER_ID_LT compile-error dependency identified and fixed. Pending CI.

---

### ID_MSK — Mask ✓ complete (pending CI)

**Pre-fold-down blast radius audit (41 literal / ~50 true hits):**

makesdna (3 files): `DNA_ID_enums.h` (enum entry → deprecated `#define`); `DNA_mask_types.h:125-128` (Scar 8: entire `#ifdef __cplusplus` block — id_type only, no DNA_DEFINE_CXX_METHODS in this block; second `#ifdef __cplusplus` at line 229 for MaskLayerShape::vertices() methods untouched); `DNA_ID.h` (`FILTER_ID_MSK`, `INDEX_ID_MSK`, `FILTER_ID_ALL` entry removed).

blenkernel (4 files): `BKE_idtype.hh` (extern decl removed); `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4); `main.cc` (CASE_ID_INDEX + lb[] removed; `case ID_MSK:` which_libbase routing KEPT — Scar 2); `mask.cc` (IDTypeInfo IDType_ID_MSK removed; `mask_alloc()` helper → Scar 10 `MEM_new<Mask>` + manual listbase insert; `id_fake_user_set` replicated from original `mask_init_data`).

`scene.cc` (FILTER_ID_MSK from Scene IDTypeInfo `dependencies_id_types`).

Scar 2 — kept (mandatory): `BKE_main.hh` (masks field); `main.cc` which_libbase routing. No versioning files iterate `bmain->masks` by field name (clean Scar 2 — no versioning bridge hits beyond the `which_libbase` routing).

makesrna (7 files): `rna_ID.cc` (×4 — enum item, filter item, base_type check, switch case); `rna_main_api.cc` (`rna_Main_masks_new`, `RNA_MAIN_ID_TAG_FUNCS_DEF`, `RNA_def_main_masks` all removed); `rna_main.cc` (`RNA_MAIN_LISTBASE_FUNCS_DEF(masks)` + table entry); `rna_internal.hh` (`RNA_def_main_masks` removed; `RNA_def_mask` KEPT — Scar 15); `rna_space.cc` (`FILTER_ID_MSK` removed from `category_image` asset browser filter); `BLT_I18NCONTEXT_ID_MASK` KEPT in `BLT_translation.hh` (legitimate borrowers: `rna_mask.cc` line properties, `rna_brush.cc` mask_tool property); `interface_template_id.cc` (`BLT_I18N_MSGID_MULTI_CTXT` entry removed — Scar 13 sweep; constant itself KEPT).

`editmesh_bisect.cc` (Scar 13 remap: `RNA_def_property_translation_context` for `use_fill` prop changed from `BLT_I18NCONTEXT_ID_MASK` → `BLT_I18NCONTEXT_DEFAULT` — unrelated borrower).

`sequencer_edit.cc` (`BKE_idtype_idcode_to_name[_plural](ID_MSK)` → hardcoded `"Mask"`/`"Masks"` — INIT_TYPE removed so those functions return nullptr).

python (1 file): `space_sequencer.py` (bpy.data.masks length conditional → always INVOKE_DEFAULT path; `bpy.data.masks` collection is gone).

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum removed, deprecated `#define` added), `DNA_mask_types.h` (Scar 8: `#ifdef __cplusplus` block removed — id_type only; second block for MaskLayerShape::vertices() untouched), `DNA_ID.h` (`FILTER_ID_MSK`, `INDEX_ID_MSK`, FILTER_ID_ALL entry removed) | ✓ |
| `blenkernel` | `BKE_idtype.hh` (extern removed), `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4), `main.cc` (CASE_ID_INDEX + lb[] removed; which_libbase `case ID_MSK:` KEPT — Scar 2), `mask.cc` (IDTypeInfo block removed; `mask_alloc()` → MEM_new<Mask> + manual insert — Scar 10; id_fake_user_set replicated), `scene.cc` (FILTER_ID_MSK from dependencies_id_types) | ✓ |
| `makesrna` | `rna_ID.cc` (4 entries), `rna_main_api.cc` (new + tag funcs + collection accessor), `rna_main.cc` (listbase funcs + table entry), `rna_internal.hh` (RNA_def_main_masks removed; RNA_def_mask KEPT — Scar 15), `rna_space.cc` (FILTER_ID_MSK from category_image), `interface_template_id.cc` (BLT_I18N_MSGID_MULTI_CTXT — Scar 13 sweep; BLT_I18NCONTEXT_ID_MASK constant KEPT) | ✓ |
| `editors+python` | `editmesh_bisect.cc` (Scar 13 remap: use_fill prop context → BLT_I18NCONTEXT_DEFAULT), `sequencer_edit.cc` (hardcoded "Mask"/"Masks" strings), `space_sequencer.py` (bpy.data.masks → INVOKE_DEFAULT path) | ✓ |

**What stays (fold-down keeps all runtime code):** `rna_mask.cc`, `BKE_mask.hh`, `mask.cc` (all non-IDTypeInfo functions), `mask_query.cc`, `mask_evaluate.cc`, `mask_ops.cc`, `mask_draw.cc`, all editor dispatch (icons, outliner, template_id browse string, buttons_context), full `ANIMTYPE_MASKLAYER` anim channel chain (mask uses per-layer channels, not a DS-wrapper — no ANIMTYPE_DSMASK exists), full depsgraph builder dispatch, all `BKE_mask_*` API. `bpy.context.edit_mask` and `bpy.context.active_object.mask` still valid — the runtime mask object is accessed via context, not `bpy.data.masks`.

**Claude AI contributor (2026-05-13):** ID_MSK fold-down across 4 layers on branch `claude/update-ci-status-5uAPa`. 4 code commits pushed. Key distinctions: no ANIMTYPE_DSMASK exists (mask anim uses ANIMTYPE_MASKLAYER — per-layer channels, not a type-level DS-wrapper), so no anim chain cleanup needed; `BLT_I18NCONTEXT_ID_MASK` kept unlike LP/PAL (legitimate non-ID borrowers in rna_mask.cc and rna_brush.cc); `sequencer_edit.cc` needed hardcoded string fix because `BKE_idtype_idcode_to_name` returns nullptr after INIT_TYPE removal; `editmesh_bisect.cc` Scar 13 remap (unrelated `use_fill` prop borrowed the mask i18n context). Pending CI.

---

### ID_VF — VFont ✓ complete (pending CI)

Literal grep: 45 hits. True blast radius: ~55 hits across ~30 files.

VFont is a fold-down, not a chisel. All runtime functionality kept. Text objects, geometry nodes string-to-curves, SOCK_FONT node sockets, depsgraph evaluation — all preserved. Only the ID registration machinery removed.

**Key notes:**
- **No anim chain removal** — VFont has no `ANIMTYPE_DSVFONT`, no `ACF_DSVFONT`, no `ADS_FILTER_NOVFONT`. VFont is not animatable directly; text object properties are animated via Object ID, not VFont. Nothing to touch in the anim system.
- **Scar 10 inside BKE_vfont_load()** — VFont's allocation happens inside `BKE_vfont_load()` (which also parses the font file), unlike other types that have a dedicated `BKE_X_add()`. Scar 10 applied surgically: only the `BKE_libblock_alloc` call replaced with `MEM_new<VFont>` + manual listbase insert; remaining `BKE_vfont_load` logic unchanged.
- **BLT_I18NCONTEXT_ID_VFONT fully removed** — no external borrowers (unlike ID_MSK where rna_mask.cc/rna_brush.cc legitimately used the constant). Not present in `interface_template_id.cc` MULTI_CTXT list (VFont was never added there). Fully removed from `BLT_translation.hh`.
- **SOCK_FONT kept** — active geometry nodes socket type; `node_geo_input_font.cc`, `node_geo_string_to_curves.cc`, all SOCK_FONT handling kept entirely.
- **`rna_space.cc` category_misc** — `FILTER_ID_VF |` removed from asset browser `category_misc` filter group.
- **`sequencer_clipboard.cc` VSE_COPYBUFFER_IDTYPES** — includes `ID_VF`; stays valid since deprecated define has same numeric value.

| Layer | Files changed | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entry → deprecated `#define`), `DNA_vfont_types.h` (Scar 8: entire `#ifdef __cplusplus` block removed — id_type only, no DNA_DEFINE_CXX_METHODS), `DNA_ID.h` (`FILTER_ID_VF`, `INDEX_ID_VF`, FILTER_ID_ALL entry removed) | ✓ |
| `blenkernel` | `BKE_idtype.hh` (extern removed), `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4), `main.cc` (CASE_ID_INDEX + lb[] removed; which_libbase `case ID_VF:` KEPT — Scar 2), `vfont.cc` (IDTypeInfo block removed; `BKE_vfont_load()` → MEM_new<VFont> + manual insert — Scar 10; static blend I/O callbacks kept — Scar 17) | ✓ |
| `makesrna` | `rna_ID.cc` (4 entries — enum item, filter item, base_type check, switch case; both `case ID_VF:` runtime mappings KEPT), `rna_main_api.cc` (load func + tag funcs + collection accessor removed), `rna_main.cc` (listbase funcs + table entry), `rna_internal.hh` (RNA_def_main_fonts removed; RNA_def_vfont KEPT — Scar 15), `rna_space.cc` (FILTER_ID_VF from category_misc), `BLT_translation.hh` (BLT_I18NCONTEXT_ID_VFONT removed — Scar 13; not in MULTI_CTXT list) | ✓ |
| `python` | `settings.py` ("fonts" data path), `_bpy_types.py` ("fonts" attr_links), `space_outliner.py` (bpy.data.fonts orphan condition) | ✓ |

**What stays (fold-down keeps all runtime code):** `vfont.cc` (all non-IDTypeInfo functions including `BKE_vfont_load`, `BKE_vfont_copy`, `vfont_blend_write`, `vfont_blend_read_data`), `BKE_vfont.hh`, all `curve_deform.cc` / `text_layout.cc` text rendering, all `OB_FONT` handling throughout editors, full depsgraph `build_vfont` dispatch, all SOCK_FONT geometry node / compositor handling. `bpy.context.object.data.font` still valid — font assignment via text object data, not `bpy.data.fonts`.

**Claude AI contributor (2026-05-13):** ID_VF fold-down across 4 layers on branch `claude/update-ci-status-5uAPa`. 4 code commits pushed. Key distinction from other fold-downs: no dedicated `BKE_vfont_add()` — allocation lives inside `BKE_vfont_load()` which parses the font simultaneously; Scar 10 applied surgically inside that function. SOCK_FONT socket handling confirmed active runtime code and fully kept. BLT_I18NCONTEXT_ID_VFONT fully removed (no external borrowers unlike ID_MSK). Pending CI.

---

### ID_BR — Brush ✓ complete (pending CI)

True blast radius: 119 literal / ~135 true hits — fold-down, not chisel; all runtime code kept.

**Key notes:**
- **No anim chain** — Brush has no ANIMTYPE_DSBRUSH, no ACF_DSBRUSH, no ADS_FILTER_NOBRUSH. Nothing to remove.
- **Scar 8 partial** — `DNA_brush_types.h` block has `DNA_DEFINE_CXX_METHODS(Brush)` + `id_type = ID_BR`. Removed only `id_type` line; guard + CXX methods stay (same as ID_LT).
- **Scar 10** — `BKE_brush_add` → `MEM_new<Brush>` + manual insert. `brush_init_data` (static in same file) called after insert. `lib_id_test.cc` secondary crash site patched: `BKE_id_new(ctx.bmain, ID_BR, ...)` → `&BKE_brush_add(...)→id`.
- **Scar 13 partial** — `BLT_I18NCONTEXT_ID_BRUSH` KEPT (20+ borrowers in rna_brush.cc + dynamicpaint.cc). Only `interface_template_id.cc` MULTI_CTXT entry removed.
- **Compile-error sites caught in audit** — `scene.cc:1611` FILTER_ID_BR in dependencies_id_types, `ED_asset_type.hh:21` FILTER_ID_BR in non-experimental flags. Both fixed by removing `FILTER_ID_BR |`.
- **Python copy-to-selected** — `bpy.data.brushes` fallback in wm.py replaced with `[]`; outliner path still works.
- **Final Bucket 3 fold-down** — datablock audit closed at 39 → ~19 ID types.

| Layer | Files | Status |
|-------|-------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum → deprecated `#define`), `DNA_brush_types.h` (Scar 8 partial: `id_type` line only; `DNA_DEFINE_CXX_METHODS` + guard kept), `DNA_ID.h` (`FILTER_ID_BR`, `INDEX_ID_BR`, FILTER_ID_ALL entry removed) | ✓ |
| `blenkernel` | `BKE_idtype.hh` (extern removed), `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2 — Scar 4), `main.cc` (CASE_ID_INDEX + lb[] removed; which_libbase `case ID_BR:` KEPT — Scar 2), `brush.cc` (IDTypeInfo block removed; `BKE_brush_add` → MEM_new<Brush> + manual insert + brush_init_data call — Scar 10), `lib_id_test.cc` (BKE_id_new crash site patched) | ✓ |
| `makesrna` | `rna_ID.cc` (enum items + filter items removed; both `case ID_BR:` runtime mappings KEPT), `rna_main_api.cc` (new func + tag funcs + collection accessor removed), `rna_main.cc` (listbase funcs + table entry removed), `rna_internal.hh` (`RNA_def_main_brushes` removed; `RNA_def_brush` KEPT — Scar 15), `rna_space.cc` (FILTER_ID_BR from category_misc), `interface_template_id.cc` (`BLT_I18NCONTEXT_ID_BRUSH` from MULTI_CTXT — Scar 13), `scene.cc` (FILTER_ID_BR from dependencies_id_types), `ED_asset_type.hh` (FILTER_ID_BR from non-experimental flags) | ✓ |
| `python` | `settings.py` ("brushes" data path removed), `_bpy_types.py` ("brushes" from attr_links), `wm.py` (bpy.data.brushes fallback → `[]`) | ✓ |

**What stays (fold-down keeps all runtime code):** `brush.cc` (all non-IDTypeInfo functions: `BKE_brush_add`, `BKE_brush_copy`, `BKE_brush_free`, `BKE_brush_size_get/set`, all paint/sculpt brush functions), `BKE_brush.hh`, all `rna_brush.cc` RNA definitions, full depsgraph brush dispatch, all paint mode brush panel UI, all sculpt mode brush functionality, all `BKE_paintmode_*` / `BKE_paint_*` brush accessors. `bpy.context.tool_settings.*.brush` still valid — brushes accessed via tool settings, not `bpy.data.brushes`.

**Claude AI contributor (2026-05-13):** ID_BR fold-down across 4 layers on branch `claude/update-ci-status-5uAPa`. 4 code commits pushed. Final Bucket 3 fold-down; datablock audit closed at 39 → ~19 ID types. Key distinction: Brush is the highest blast radius type in Bucket 3 (119 literal hits); every paint and sculpt mode touches bmain->brushes every frame. All runtime paint/sculpt code confirmed kept. BLT_I18NCONTEXT_ID_BRUSH retained (20+ borrowers). Two compile-error sites (scene.cc, ED_asset_type.hh) caught during audit and fixed in makesrna layer.

**0.5.0 CI fixes (2026-05-14):** Six post-fold-down CI failures resolved across five PRs (#175–#179) on branch `ci-consideration`. All failures traced to Bucket 3 fold-down types demoted from `ID_Type` enum to plain `#define` int constants — MSVC rejects implicit int→enum conversions. Failures: (1) `FILTER_ID_MSK` undefined in space_image.cc/space_clip.cc remap guards (Scar 19 Mode A: restored `#define FILTER_ID_MSK` in DNA_ID.h, absent from `FILTER_ID_ALL`). (2) `socket_type_to_id_type()` returning `ID_VF`/`ID_MSK` — C2440; added `SOCK_TEXTURE` nullopt case + `static_cast<ID_Type>` on returns (Scar 19 Mode B). (3) Six `asset_edit_id_from_weak_reference(bmain, ID_BR, ...)` calls in brush_asset_ops.cc, paint.cc, erase.cc — C2664; `static_cast<ID_Type>(ID_BR)`. (4) `BKE_id_new_nomain<Brush>` in grease_pencil/paint.cc — C2039 (`Brush::id_type` removed by fold-down); added `BKE_brush_new_nomain()` to brush.cc/BKE_brush.hh using Scar 10 manual allocation. (5) `WM_operator_properties_id_lookup_from_name_or_session_uid(bmain, op->ptr, ID_MSK)` in node_add.cc — C2664; `static_cast<ID_Type>(ID_MSK)`. (6) `BLI_strncpy` undefined in brush.cc — C3861; missing `#include "BLI_string.h"` (surfaced only in full release build, not lite build). **Scar 19 documented** with both failure modes, detection greps, and known `ID_Type` vs `short/int` callee lists. CI green: Windows x64, build 81, commit `d6ee8478`.

**Version bump (2026-05-14):** `BLENDED_VERSION_MINOR` updated from 5 to 6 in `BKE_blender_version.h`. 0.5.0 datablock audit layer CI-complete; 0.6.x depsgraph audit dev cycle begins. Packaged artifacts now correctly labelled `Blended-0.6.0-windows-x64`.

---

## 0.4.0 — 2026-05-08

Bucket 5 + 6 fossil removals — 9 ID types removed across the same chisel pattern as 0.3.0. CI green (Windows x64, build 70, commit `7bd69df`) — 9 of 9 types complete.
Chisel order: **ID_PC ✓** → **ID_SPK ✓** → **ID_PA ✓** → **ID_GD_LEGACY ✓** → **ID_LS ✓** → **ID_MB ✓** → **ID_TE ✓** → **ID_CU_LEGACY ✓** → **ID_CF ✓**.
*(Order corrected in PR #126 fix — initial commit had ID_CF first, contradicting CLAUDE.md Key note 8. Scar 7.)*

**Key notes:**
- **Scar 2 mandatory for:** `ID_GD_LEGACY` (`bmain->gpencils` — OB_GPENCIL_LEGACY objects still active), `ID_LS` (`bmain->linestyles` — legacy file loads populate it), `ID_PA` (`bmain->particles` — versioning_250–400 + versioning_legacy iterate it), `ID_TE` (`bmain->textures` — versioning_250/260/280/legacy iterate it), `ID_CU_LEGACY` (`bmain->curves` — 23+ iterations across versioning_250–520, anim_data_bmain_utils.cc, anim_sys.cc). `ID_PC`, `ID_SPK`, `ID_MB`, `ID_CF` — no Scar 2 rescue (true fossils; no versioning iteration).
- `ID_CU_LEGACY` and `ID_GD_LEGACY` have active migration paths — only the type *registration* goes, not the converters.
- ~~`ID_LS` is already guarded by `#ifdef WITH_FREESTYLE`~~ — verified false for core registration files; `linestyle.cc` and render_shading.cc ID_LS cases are outside the guard. Full removal required.
- `ID_LS` known latent leak: opening a legacy `.blend` with Freestyle data in a `WITH_FREESTYLE=OFF` build populates `bmain->linestyles` via the kept `which_libbase` routing, but that listbase is not in `BKE_main_lists_get`, so `BKE_main_free` does not free those IDs. Accepted artifact — no Freestyle fixtures in CI, does not affect release builds. Fix if needed: blenloader post-read pass draining `bmain->linestyles` when `WITH_FREESTYLE=OFF`.
- **Shared switch cases:** batch removals per file rather than per type — depsgraph (`deg_builder_nodes.cc`, `deg_builder_relations.cc`), outliner (`outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`), and RNA (`rna_ID.cc`, `rna_main_api.cc`) contain cases for many types side by side.
- ~~`brush_test.cc` uses `ID_TE` in test fixtures~~ — test fixtures deleted in makesdna/blenkernel layers.
- ~~`depsgraph.cc:160` has a `!= ID_PA` guard~~ — resolved in ID_PA chisel; guard changed to `!= ID_SCE`.

### ID_PC — PaintCurve ✓ complete

**Pre-chisel blast radius audit (21 literal / ~35 true hits):**

makesdna (3 files): `DNA_ID_enums.h:158` (enum → deprecated #define); `DNA_brush_types.h:192` (Brush::paint_curve field) + `:482–500` (PaintCurvePoint/PaintCurve struct definitions); `DNA_ID.h:655,695` (shared macro checks); `DNA_undo_system_types.h` (UNDO_REF_ID_TYPE).

blenkernel (5 files): `BKE_idtype.hh:331`; `idtype.cc:168` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:1046` (CASE_ID_INDEX + lb[] — no Scar 2); `BKE_main.hh:394` (paintcurves field); `paint.cc:185–247` (IDTypeInfo + copy/free/blend-write/read callbacks); `brush.cc:229,550` (FOREACHID + FILTER_ID_PC in deps); `brush_test.cc:64,95` (test fixtures rewritten); `BKE_paint.hh:149,262`; `BKE_undo_system.hh:50`.

editors (10 files): DELETED: `paint_curve.cc`, `paint_curve_undo.cc`, `transform_convert_paintcurve.cc`; `paint_cursor.cc:877–896`; `paint_stroke.cc:1276–1293`; `interface_icons.cc:2085`; `interface_template_id.cc:901`; `render_opengl.cc:643`; `outliner_draw.cc:2583`; `outliner_intern.hh:173`; `outliner_tools.cc:162`; `tree_element_id.cc:89`.

depsgraph (2 files): `deg_builder_relations.cc:610`; `deg_builder_nodes.cc:663`.

makesrna (4 files): `rna_ID.cc:57,448,538`; `rna_main_api.cc:866`; `rna_brush.cc` (paint_curve RNA prop); `rna_sculpt_paint.cc` (PaintCurve RNA defs, 7 sites).

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_brush_types.h` (id_type constexpr, PaintCurvePoint/PaintCurve structs, Brush::paint_curve field), `DNA_ID.h` (shared macro checks) | ✓ |
| `blenkernel` | `idtype.cc`, `main.cc`, `paint.cc` (IDTypeInfo + callbacks), `brush.cc` (FOREACHID, deps), `brush_test.cc` (test fixtures rewritten); headers: `BKE_idtype.hh`, `BKE_main.hh`, `BKE_paint.hh`, `BKE_undo_system.hh` | ✓ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc`, `rna_main.cc`, `rna_brush.cc`, `rna_sculpt_paint.cc`, `rna_space.cc`, `rna_internal.hh` | ✓ |
| `editors` | `paint_curve.cc` (deleted), `paint_curve_undo.cc` (deleted), `transform_convert_paintcurve.cc` (deleted); `paint_cursor.cc/hh`, `paint_stroke.cc`, `paint_ops.cc`, `paint_intern.hh`, `transform_convert.cc/hh`, `transform_generics.cc`, `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `BLT_translation.hh` | ✓ |
| `depsgraph` | `deg_builder_relations.cc`, `deg_builder_nodes.cc` | ✓ |

### ID_SPK — Speaker ✓ complete

**Pre-chisel blast radius audit (23 literal / ~45 true hits):**

makesdna (5 files): `DNA_ID_enums.h` (enum → deprecated #define); `DNA_ID.h` (FILTER_ID_SPK, INDEX_ID_SPK, FILTER_ID_ALL); `DNA_object_types.h` (OB_SPEAKER enum + macros); `DNA_userdef_types.h` (dupflag default); DELETED: `DNA_speaker_types.h` (entire struct).

blenkernel (10+ files): `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc` (CASE_ID_INDEX + lb[] — no Scar 2); `object.cc`; `sound.cc` (speaker loop + speaker_handles removed); `nla.cc` (BKE_nla_add_soundstrip removed); `anim_data_bmain_utils.cc`; `anim_sys.cc`; `geometry_set_instances.cc`; DELETED: `speaker.cc`, `BKE_speaker.hh`; headers: `BKE_idtype.hh`, `BKE_main.hh`, `BKE_nla.hh`, `BKE_scene_runtime.hh`.

makesrna (9 files): `rna_ID.cc`; `rna_main_api.cc`; `rna_main.cc`; `rna_object.cc`; `rna_userdef.cc`; `rna_action.cc`; `rna_space.cc`; `rna_space_api.cc`; `rna_internal.hh`; DELETED: `rna_speaker.cc`.

editors (26+ files): `interface_icons.cc`; `interface_template_id.cc`; `render_opengl.cc`; `outliner_draw.cc`; `outliner_select.cc`; `outliner_intern.hh`; `outliner_tools.cc`; `tree_element_id.cc`; `space_view3d.cc`; `buttons_context.cc`; `object_add.cc` (OBJECT_OT_speaker_add deleted); `object_intern.hh`; `object_ops.cc`; `object_relations.cc`; `nla_edit.cc` (NLA_OT_soundclip_add deleted); `nla_intern.hh`; `nla_ops.cc`; `nla_draw.cc`; `nla_tracks.cc`; `nla_buttons.cc`; `anim_filter.cc`; `anim_channels_defines.cc` (ACF_DSSPK + 3 callbacks + table entry); `anim_channels_edit.cc` (ANIMTYPE_DSSPK ×9 fallthroughs); `anim_deps.cc`; `transform_convert_action.cc`; `BLT_translation.hh` (BLT_I18NCONTEXT_ID_SPEAKER); `ED_anim_api.hh` (ANIMTYPE_DSSPK enum).

draw (5 files): `overlay_instance.hh/.cc`; `overlay_private.hh`; `overlay_bounds.hh`; DELETED: `overlay_speaker.hh`.

depsgraph (8 files): `deg_builder_nodes.cc/.h`; `deg_builder_nodes_scene.cc`; `deg_builder_relations.cc/.h`; `deg_builder_relations_scene.cc`; `deg_node_operation.hh/.cc` (SPEAKER_EVAL opcode removed).

python (10 files): `bl_ui/__init__.py`; `space_dopesheet.py`; `space_outliner.py`; `space_userpref.py`; `space_view3d.py`; `bl_operators/wm.py`; `_bpy_types.py`; `_bl_i18n_utils/settings.py`; `_rna_manual_reference.py`; DELETED: `properties_data_speaker.py`.

blenloader (1 file): `versioning_520.cc` (502.23 pass — OB_SPEAKER → OB_EMPTY).

io (2 files): `usd_hierarchy_iterator.cc`; `abc_hierarchy_iterator.cc`.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_speaker_types.h` (deleted), `DNA_ID.h` (FILTER/INDEX macros), `DNA_object_types.h` (OB_SPEAKER enum + macros), `DNA_userdef_types.h` (dupflag default) | ✓ |
| `blenkernel` | `idtype.cc`, `main.cc`, `object.cc`, `sound.cc` (speaker loop + speaker_handles), `nla.cc` (BKE_nla_add_soundstrip), `anim_data_bmain_utils.cc`, `anim_sys.cc`, `geometry_set_instances.cc`; `speaker.cc` (deleted), headers: `BKE_idtype.hh`, `BKE_main.hh`, `BKE_nla.hh`, `BKE_scene_runtime.hh`, `BKE_speaker.hh` (deleted) | ✓ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc`, `rna_main.cc`, `rna_object.cc`, `rna_userdef.cc`, `rna_action.cc`, `rna_space.cc`, `rna_space_api.cc`, `rna_internal.hh`; `rna_speaker.cc` (deleted) | ✓ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `space_view3d.cc`, `buttons_context.cc`, `object_add.cc` (OBJECT_OT_speaker_add deleted), `object_intern.hh`, `object_ops.cc`, `object_relations.cc`, `space_nla/nla_edit.cc` (NLA_OT_soundclip_add deleted), `nla_intern.hh`, `nla_ops.cc`, `nla_draw.cc`, `nla_tracks.cc`, `nla_buttons.cc`, `anim_filter.cc`, `anim_channels_defines.cc` (ACF_DSSPK deleted), `anim_channels_edit.cc`, `anim_deps.cc`, `transform/transform_convert_action.cc`, `BLT_translation.hh` (BLT_I18NCONTEXT_ID_SPEAKER), `ED_anim_api.hh` (ANIMTYPE_DSSPK) | ✓ |
| `draw` | `overlay_speaker.hh` (deleted), `overlay_instance.hh`, `overlay_instance.cc`, `overlay_private.hh`, `overlay_bounds.hh` | ✓ |
| `depsgraph` | `deg_builder_nodes.cc`, `deg_builder_nodes.h`, `deg_builder_nodes_scene.cc`, `deg_builder_relations.cc`, `deg_builder_relations.h`, `deg_builder_relations_scene.cc`, `deg_node_operation.hh`, `deg_node_operation.cc` (SPEAKER_EVAL removed) | ✓ |
| `python` | `properties_data_speaker.py` (deleted), `bl_ui/__init__.py`, `space_dopesheet.py`, `space_outliner.py`, `space_userpref.py`, `space_view3d.py`, `bl_operators/wm.py`, `_bpy_types.py`, `_bl_i18n_utils/settings.py`, `_rna_manual_reference.py` | ✓ |
| `blenloader` | `versioning_520.cc` (502.23 pass — OB_SPEAKER → OB_EMPTY), `BKE_blender_version.h` (subversion bump) | ✓ |
| `io` | `io/usd/intern/usd_hierarchy_iterator.cc`, `io/alembic/exporter/abc_hierarchy_iterator.cc` | ✓ |

### ID_PA — ParticleSettings ✓ complete

**Pre-chisel blast radius audit (35 hits):**

makesdna (4 files): `DNA_ID_enums.h:152` (enum entry → deprecated #define); `DNA_particle_types.h:533` (id_type constexpr); `DNA_ID.h` (FILTER_ID_PA, INDEX_ID_PA, FILTER_ID_ALL).

blenkernel (2 files): `idtype.cc:162` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:1032` (which_libbase case — KEEP particles field + Scar 2 routing); `texture.cc:486,516` (×2).

editors (9+ files): `buttons_context.cc:517`; `interface_icons.cc:2081`; `interface_template_id.cc:636,891`; `render_shading.cc:3045,3075`; `render_opengl.cc:624`; `outliner_draw.cc:2585`; `outliner_intern.hh:157`; `outliner_tools.cc:157`; `tree_element_id.cc:77`; `anim_filter.cc:2676`; `anim_channels_defines.cc:329` (ELEM shared with MA).

depsgraph (4 files): `depsgraph_tag.cc:157,635`; `deg_builder_relations.cc:600`; `deg_builder_nodes.cc:653`; `depsgraph.cc:160` (teardown guard → `!= ID_SCE`).

animrig (1 file): `animdata.cc:125`.

makesrna (7 files): `rna_ID.cc:59,442,534,1050` (×4); `rna_texture.cc:272`; `rna_particle.cc:1014`; `rna_boid.cc:232`; `rna_color.cc:386`; `rna_object_force.cc:671`; `rna_main_api.cc:858`.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entry → deprecated `#define`), `DNA_particle_types.h` (id_type constexpr), `DNA_ID.h` (FILTER_ID_PA, INDEX_ID_PA, FILTER_ID_ALL) | ✓ |
| `blenkernel` | `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2), `main.cc` (CASE_ID_INDEX, lb[] — KEEP which_libbase routing + particles field as Scar 2), `texture.cc` (×2), `particle.cc` (IDTypeInfo + all callbacks + fluid_free_settings), `BKE_idtype.hh`, `BKE_main.hh` (particles field restored as non-indexed Scar 2 listbase — see correction note), `anim_data_bmain_utils.cc` (missed in original chisel — ANIMDATA_IDS_CB removed) | ✓ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc` (RNA_def_main_particles, rna_Main_particles_new), `rna_main.cc` (listbase macro + table entry), `rna_internal.hh`, `rna_space.cc` (FILTER_ID_PA in asset browser misc category — literal grep miss) | ✓ |
| `editors` | `buttons_context.cc`, `interface_icons.cc`, `interface_template_id.cc`, `render_shading.cc` (×2), `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `anim_filter.cc`, `anim_channels_defines.cc` | ✓ |
| `depsgraph` | `depsgraph_tag.cc` (×2), `deg_builder_relations.cc`, `deg_builder_nodes.cc`, `depsgraph.cc` (teardown guard: `!= ID_PA` → `!= ID_SCE`) | ✓ |
| `animrig` | `animdata.cc` | ✓ |

### ID_GD_LEGACY — Old Grease Pencil ✓ complete

*Session note (2026-04-30): Three key true-blast-radius findings: (1) bmain->gpencils kept as Scar 2 listbase — OB_GPENCIL_LEGACY and annotation creation still need it. (2) All depsgraph sites (depsgraph_tag.cc:72,626; deg_builder_nodes.cc:630; deg_builder_relations.cc:580,2758) left untouched — bGPdata evaluation must survive. (3) material.cc mat/totcol pointer cases initially removed then restored — OB_GPENCIL_LEGACY objects have material slots. Active migration: grease_pencil_convert_legacy.cc and blendfile_link_append.cc converter code kept; only the type registration went.*

**Pre-chisel blast radius audit (56 hits, 32 files):**

makesdna (4 files): `DNA_ID_enums.h:151` (enum entry → deprecated #define); `DNA_gpencil_legacy_types.h:711` (id_type constexpr); `DNA_object_types.h:747,762` (object type macros ×2); `DNA_ID.h:1162,1195,1244` (FILTER_ID_GD, FILTER_ID_ALL, INDEX_ID_GD).

blenkernel (9 files): `BKE_idtype.hh:324` (extern decl); `gpencil_legacy.cc:267,269,271,654` (IDTypeInfo callbacks + alloc call); `idtype.cc:161` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:131,1027,1071` (CASE_ID_INDEX + which_libbase case + lb[] — KEEP which_libbase); `material.cc:427,455,850` (×3); `deform.cc:460,481` (×2); `grease_pencil_convert_legacy.cc:3057,3151` KEEP; `blendfile_link_append.cc:555` KEEP; `scene.cc:1611` (FILTER_ID_GD from deps); `movieclip.cc:298` (IDTypeInfo dep check).

blenloader (2 files — KEEP): `versioning_250.cc:444` (GS write); `versioning_common.cc:61,62` (GD_LEGACY → GP v3 marker).

editors (10 files): `interface_icons.cc:2055`; `interface_template_id.cc:588,885`; `object_data_transform.cc:816`; `render_opengl.cc:649`; `outliner_select.cc:1295`; `outliner_draw.cc:2556`; `outliner_intern.hh:156`; `outliner_tools.cc:156`; `tree_element_id.cc:56`; `space_node.cc:1537`; `space_image.cc:1214` (FILTER_ID_GD).

draw (1 file): `draw_context.cc:1166` (DEG_id_type_any_exists → gpencil-only). depsgraph (3 files): `depsgraph_tag.cc:72,626`; `deg_builder_nodes.cc:630`; `deg_builder_relations.cc:580,2758`. makesrna (4 files): `rna_ID.cc:41,124,377,477`; `rna_main_api.cc:831`; `rna_main.cc`; `rna_space.cc:3975` (FILTER_ID_GD in misc filter — literal grep miss).

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entry → deprecated `#define`), `DNA_gpencil_legacy_types.h` (id_type constexpr), `DNA_object_types.h` (OB_DATA_SUPPORT_ID macros ×2), `DNA_ID.h` (FILTER_ID_GD_LEGACY, INDEX_ID_GD_LEGACY, FILTER_ID_ALL) | ✓ |
| `blenkernel` | `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2), `main.cc` (CASE_ID_INDEX, lb[] — KEEP which_libbase routing), `gpencil_legacy.cc` (IDTypeInfo block removed — KEEP alloc call), `material.cc` (render case removed, mat/totcol pointer cases kept), `scene.cc` (FILTER_ID_GD_LEGACY from deps), `movieclip.cc` (FILTER_ID_GD_LEGACY from deps); `BKE_idtype.hh` (extern decl); `BKE_main.hh` KEPT, `deform.cc` KEPT | ✓ |
| `blenloader` | `versioning_250.cc` KEPT, `versioning_common.cc` KEPT (converter logic) | ✓ |
| `makesrna` | `rna_ID.cc` (enum item, filter item, base_type check, case return), `rna_main_api.cc` (RNA_def_main_annotations, rna_Main_annotations_new, RNA_MAIN_ID_TAG_FUNCS_DEF(gpencils)), `rna_main.cc` (listbase macro + table entry), `rna_internal.hh` (decl), `rna_space.cc` (FILTER_ID_GD_LEGACY in asset browser misc — literal grep miss) | ✓ |
| `editors` | `interface_icons.cc`, `interface_template_id.cc` (×2), `object_data_transform.cc`, `render_opengl.cc`, `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `space_node.cc`, `space_image.cc` | ✓ |
| `draw` | `draw_context.cc` (gpencil_any_exists simplified to ID_GP only) | ✓ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_builder_nodes.cc`, `deg_builder_relations.cc` — **ALL KEPT** (OB_GPENCIL_LEGACY objects still exist at runtime; geometry node building and relations for bGPdata must survive) | ✓ |

### ID_LS — FreestyleLineStyle ✓ complete

**Pre-chisel blast radius audit (28 literal / ~50 true hits):**

makesdna (4 files): `DNA_ID_enums.h:156` (enum → deprecated #define); `DNA_linestyle_types.h:649` (id_type constexpr); `DNA_ID.h` (FILTER_ID_LS, INDEX_ID_LS, FILTER_ID_ALL); `DNA_action_types.h` (ADS_FILTER_NOLINESTYLE).

blenkernel (4+ files): `BKE_idtype.hh` (extern decl); `idtype.cc:166` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:1042` (CASE_ID_INDEX + lb[] — KEEP which_libbase routing + linestyles Scar 2); `linestyle.cc` (IDTypeInfo block removed — KEEP BKE_linestyle_new); `node.cc:5153`; `texture.cc:480,513` (×2); `scene.cc` (FILTER_ID_LS from deps).

blenloader (2 files): `versioning_500.cc:4494`; `versioning_450.cc:5891`.

editors (26 files): `buttons_texture.cc`; `buttons_context.cc` (linestyle path fn + pinnable fn + dispatch + "line_style" member + FS texture slot); `interface_icons.cc`; `interface_template_id.cc`; `interface_template_preview.cc` (×3); `render_shading.cc` (×3 incl. FreestyleLineStyle paste context); `render_opengl.cc`; `outliner_draw.cc`; `outliner_intern.hh`; `outliner_tools.cc`; `tree_element_id.cc`; DELETED: `tree_element_id_linestyle.cc/.hh`; `space_node.cc` (NC_LINESTYLE ×2); `anim_channels_defines.cc` (ACF_DSLINESTYLE 3 fns + struct + table entry); `anim_channels_edit.cc` (ANIMTYPE_DSLINESTYLE ×9); `anim_deps.cc`; `anim_filter.cc` (fn + call site + case); `ED_anim_api.hh` (ANIMTYPE_DSLINESTYLE + FILTER_LS_SCED); `nla_buttons.cc`; `nla_draw.cc`; `nla_tracks.cc`; `transform_convert_action.cc`; `rna_action.cc` (show_linestyles prop).

depsgraph (5 files): `deg_eval_copy_on_write.cc` (SPECIAL_CASE ×4 + sizeof + include removed); `deg_builder_relations.cc/.h` (case + fn + decl); `deg_builder_nodes.cc/.h` (case + fn + decl); `deg_builder_relations_view_layer.cc`; `deg_builder_nodes_view_layer.cc`.

nodes (2 files): `shader_nodes_inline.cc` (ShaderNodeOutputLineStyle case); `node_texture_tree.cc` (unguarded SNODE_TEX_LINESTYLE branch + include).

makesrna (7 files): `rna_ID.cc`; `rna_texture.cc` (NC_LINESTYLE case); `rna_color.cc` (×3); `rna_space.cc` (FILTER_ID_LS in shading filter); `rna_main_api.cc`; `rna_main.cc`; `rna_internal.hh`.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum → deprecated `#define`), `DNA_linestyle_types.h` (id_type constexpr), `DNA_ID.h` (FILTER_ID_LS, INDEX_ID_LS, FILTER_ID_ALL), `DNA_action_types.h` (ADS_FILTER_NOLINESTYLE) | ✓ |
| `blenkernel` | `idtype.cc` (INIT_TYPE + CASE_IDINDEX ×2), `main.cc` (CASE_ID_INDEX, lb[] — KEEP which_libbase routing), `linestyle.cc` (IDTypeInfo block removed — KEEP alloc call), `node.cc` (node tree case), `texture.cc` (×2), `scene.cc` (FILTER_ID_LS from deps); `BKE_idtype.hh` (extern decl) | ✓ |
| `blenloader` | `versioning_500.cc` (`, ID_LS` from ELEM), `versioning_450.cc` (`, ID_LS` from ELEM) | ✓ |
| `makesrna` | `rna_ID.cc` (enum item, filter item, base_type check, case return), `rna_texture.cc` (NC_LINESTYLE case), `rna_color.cc` (×3), `rna_space.cc` (FILTER_ID_LS from shading filter), `rna_main_api.cc`, `rna_main.cc` (listbase macro + table entry), `rna_internal.hh`, `rna_action.cc` (show_linestyles prop) | ✓ |
| `editors` | `buttons_texture.cc`, `buttons_context.cc` (linestyle path fn + pinnable fn + dispatch + "line_style" member + FS texture slot), `interface_icons.cc`, `interface_template_id.cc` (×2), `interface_template_preview.cc` (×3), `render_shading.cc` (×3 incl. FreestyleLineStyle paste context), `render_opengl.cc`, `outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc` (×2 + simplified unlink_texture_fn), `tree_element_id.cc`, `tree_element_id_linestyle.cc/.hh` (DELETED), `space_node.cc` (NC_LINESTYLE ×2), `anim_channels_defines.cc` (ACF_DSLINESTYLE 3 fns + struct + table entry), `anim_channels_edit.cc` (ANIMTYPE_DSLINESTYLE ×9), `anim_deps.cc` (×1), `anim_filter.cc` (animdata_filter_ds_linestyle fn + ANIMTYPE case + ANIMCHANNEL_NEW_CHANNEL call), `ED_anim_api.hh` (ANIMTYPE_DSLINESTYLE enum + FILTER_LS_SCED macro), `nla_buttons.cc`, `nla_draw.cc`, `nla_tracks.cc`, `transform_convert_action.cc` | ✓ |
| `nodes` | `shader_nodes_inline.cc` (ShaderNodeOutputLineStyle case), `node_texture_tree.cc` (SNODE_TEX_LINESTYLE unguarded branch) | ✓ |
| `depsgraph` | `deg_eval_copy_on_write.cc` (SPECIAL_CASE ×4 + sizeof(FreestyleLineStyle) + DNA include), `deg_builder_relations.cc/.h` (case + fn + forward decl), `deg_builder_nodes.cc/.h` (case + fn + forward decl), `deg_builder_relations_view_layer.cc` (call site), `deg_builder_nodes_view_layer.cc` (call site) | ✓ |

### 0.4.0 Cleanup fixes (2026-05-01)

Three post-chisel bugs found during first full build of the 0.4.0 removal set:

| Fix | File | Root cause |
|-----|------|-----------|
| Restore `#endif` balance in `DNA_particle_types.h` — remove dangling `#ifdef __cplusplus` | `makesdna/DNA_particle_types.h` | ID_PA chisel removed `id_type` + comment + `#endif` but left opening `#ifdef __cplusplus`; initial fix of placing `#endif` at end of struct was wrong (dna_parse.cc `strip_ignored_tokens()` would have voided all struct members from SDNA) |
| Remove blank continuation line in `TREESTORE_ID_TYPE` macro | `editors/space_outliner/outliner_intern.hh` | ID_SPK + ID_PA + ID_GD_LEGACY + ID_LS removed consecutively from middle of `ELEM()` call, leaving blank line with no `\` that silently terminated the macro |
| Remove `ANIMDATA_IDS_CB(bmain->particles.first)` | `blenkernel/intern/anim_data_bmain_utils.cc` | Missed site — not in literal or true blast radius audit for ID_PA |
| Apply Scar 2 to ID_PA — restore `bmain->particles` + `which_libbase` routing | `BKE_main.hh`, `blenkernel/intern/main.cc` | 15+ blenloader versioning sites (`versioning_250` through `versioning_400`, `versioning_legacy`) iterate `bmain->particles`; full field removal crashed legacy file loading |
| Fix `BKE_particlesettings_add` allocation crash — bypass IDType registry, use `MEM_new<ParticleSettings>` + manual ID init | `blenkernel/intern/particle.cc` | INIT_TYPE(ID_PA) removed → `BKE_libblock_alloc_notest` returns nullptr → `BKE_libblock_runtime_ensure(*id)` null dereference crash in all build types. Callers include `versioning_legacy.cc` and `fluid.cc` (live paths). See Scar 10. |
| Fix `BKE_gpencil_data_addnew` allocation crash — bypass IDType registry, use `MEM_new<bGPdata>` + manual ID init | `blenkernel/intern/gpencil_legacy.cc` | Same pattern: INIT_TYPE(ID_GD_LEGACY) removed. Called from annotation painting, gpencil operators, ruler gizmo — all live. |
| Fix `BKE_linestyle_new` allocation crash — bypass IDType registry, use `MEM_new<FreestyleLineStyle>` + manual ID init | `blenkernel/intern/linestyle.cc` | Same pattern: INIT_TYPE(ID_LS) removed. `freestyle.cc` (caller) is unconditionally compiled — not guarded by `WITH_FREESTYLE`. |
| Fix spurious `}` premature namespace close | `nodes/texture/node_texture_tree.cc` | ID_LS chisel removed `SNODE_TEX_LINESTYLE` else-if block but left a stray `}` that closed `namespace blender` at line 72, stranding 260 lines outside the namespace. All `bke::` references failed. Fix: delete the stray brace. |

### 0.4.0 Deferred-debt resolution + doc protocols (2026-05-01 cont.)

Resolves two deferred-debt items, syncs a stale version define, and adds two operational rules to CLAUDE.md.

| Change | Files | Notes |
|--------|-------|-------|
| Sync `BLENDED_VERSION_*` to 0.3.0 | `BKE_blender_version.h` | Defines were stuck at 0.2.0 even after the 0.3.0 tag shipped. Without this bump, the now-wired update checker would have flashed "0.3.0 available" to users already on 0.3.0. Commit `084414e6`. |
| Wire `bpy.app.blended_version_major/minor/patch` *(resolves deferred-debt item 6)* | `python/intern/bpy_app.cc`, `scripts/startup/blended_update_check.py` | Added three int fields to `app_info_fields[]` + matching `SetIntItem` calls in `make_app_info()`. Python-side `getattr(..., default)` fallback dropped. **Note:** real target was `bpy_app.cc` (PyStructSequence), not `rna_wm.cc` as CLAUDE.md previously claimed — `bpy.app` is a Python `PyStructSequence`, not RNA. CLAUDE.md note corrected. Commit `270760c9`. |
| Delete particle-add operators + Quick Explode *(resolves deferred-debt item 1)* | `editors/physics/particle_object.cc`, `physics_intern.hh`, `physics_ops.cc`, `bl_ui/properties_particle.py`, `bl_operators/object_quick_effects.py`, `bl_ui/space_view3d.py`, `modules/_rna_manual_reference.py` | `OBJECT_OT_particle_system_add/_remove` + `PARTICLE_OT_particle_system_remove_all` were live in the UI but silently broken since the ID_PA chisel — the operator path allocated ParticleSettings via Scar 10 but the broader machinery (depsgraph relations, modifier eval, simulation) was gone. `QuickExplode` chained `particle_system_add` + Explode modifier — useless without particles, deleted entire. Surgical delete (B option per the audit), -359 lines. `BKE_particlesettings_add` itself stays (still called by `fluid.cc:4433` Mantaflow output). Commit `b4f8e3e1`. |
| Codex-pass cleanup on the particle removal | `tests/python/ui_simulate/test_quick_effects.py`, `tests/python/CMakeLists.txt`, `bl_operators/object_quick_effects.py` | Post-commit verification turned up: `add_quick_explode()` UI test (would have crashed CI on the deleted operator), CMake test runner registration, two orphan imports (`IntProperty`, `pgettext_rpt as rpt_` only used by QuickExplode). Caught before CI but should have been pre-commit, not post-hoc — see Codex Standard operationalization below. Commit `e39bcd58`. |
| Doc: Session Discipline rule | `CLAUDE.md` | "Always run a todo list each session. Three+ maneuvers = list required." Hard threshold, not vibes. Commits `bdc8a5e7`, `336b8a60`. |
| Doc: Codex Standard operationalization | `CLAUDE.md` | The Codex verification pass is a todo-list item before commit/push, never after. Cites the `b4f8e3e1` / `e39bcd58` incident as the concrete reference for future sessions. Commit `3a6d1a8f`. |
| Consolidate `PHILOSOPHY.md` into `CLAUDE.md` | `CLAUDE.md`, `PHILOSOPHY.md` (deleted), `UPSTREAM_SYNC.md` | Spliced the 12 principles + mapping table + "For AI Assistants" checklist + Charlie attribution into a new `## Development Philosophy` section in `CLAUDE.md`, before `## Key Documentation`. So a session loads operational principles alongside operational rules. `PHILOSOPHY.md` deleted. `UPSTREAM_SYNC.md` epigraph link updated to point at the new section. The 0.1.0 historical release entry below still references `PHILOSOPHY.md` — kept intentionally as accurate-as-of-then. |

### ID_MB — MetaBall ✓ (0.4.0)

**Pre-chisel blast radius audit (60 literal / 130+ true hits):**

makesdna (4 files): `DNA_ID_enums.h:134` (enum → deprecated #define); `DNA_meta_types.h:91` (id_type constexpr); `DNA_object_types.h:735,742,756` (OB_MBALL macros shared w/ CU_LEGACY); `DNA_ID.h:1169,1196,1274` (FILTER_ID_MB, FILTER_ID_ALL, INDEX_ID_MB).

blenkernel (12+ files): `BKE_idtype.hh:307`; `idtype.cc:144` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:149,991,1088` (CASE_ID_INDEX + which_libbase + lb[] — all removed, no Scar 2); `BKE_main.hh:369` (metaballs field removed — true fossil); `material.cc:425,453,486,519,542,847` (×6); `object.cc:1933,1977,2225,4279` (×4); `object_update.cc:157,291`; `object_dupli.cc:312`; `lib_remap.cc:627`; `mesh_convert.cc:921,989`; `context.cc:1415,1498`; `lib_id.cc:2412`; `anim_sys.cc:4135`; `anim_data_bmain_utils.cc:77`; DELETED: `mball_tessellate.cc`, `BKE_mball.hh`, `BKE_mball_tessellate.hh`.

blenloader (1 file — KEEP): `versioning_legacy.cc:2382` (KEEP — bmain->metaballs iteration for legacy ID property fix; only versioning pass site).

editors (40+ files): `interface_icons.cc`; `interface_template_id.cc:583,854`; `object_data_transform.cc` (×4); `transform_convert_object_texspace.cc:52` (ELEM w/ CU_LEGACY); `render_opengl.cc:610`; `outliner_select.cc:1289`; `outliner_draw.cc:2485`; `outliner_intern.hh:141` (Scar 9); `outliner_tools.cc:136,287`; `tree_element_id.cc:49`; DELETED: `tree_element_id_metaball.cc/.hh`, `metaball/mball_edit.cc`, `metaball/mball_ops.cc`, `metaball/editmball_undo.cc`, `metaball/mball_intern.hh`, `ED_mball.hh`; `anim_channels_defines.cc` (ACF_DSMBALL + 3 callbacks); `anim_filter.cc` (animdata_filter_ds_metaball); `ED_anim_api.hh` (ANIMTYPE_DSMBALL + FILTER_MBALL_OBJD); `anim_channels_edit.cc` (9 fallthrough sites) + nla/transform fallthrough; `object_add.cc`, `object_bake_api.cc`, `object_edit.cc`, `object_hook.cc`, `object_modes.cc`, `object_modifier.cc`, `object_relations.cc`, `object_transform.cc`, `object_utils.cc`, `screen_ops.cc`, `spacetypes.cc`, `buttons_context.cc`, `info_stats.cc`, `view3d_buttons.cc`, `view3d_iterators.cc`, `view3d_select.cc`, `view3d_snap.cc`, `transform.cc`, `transform_convert.cc`, `transform_convert.hh`; DELETED: `transform_convert_mball.cc`; `transform_gizmo_3d.cc`, `transform_mode.cc`, `transform_orientations.cc`, `transform_snap.cc`, `undo_system_types.cc`, `ed_transverts.cc`.

draw (9 files): `overlay_bounds.hh:188`; `draw_resource.hh:157`; `overlay_instance.cc/.hh`; `overlay_private.hh`; `overlay_shape.cc`; `draw_context.cc`; `draw_handle.hh`; DELETED: `overlay_metaball.hh`.

depsgraph (5 files): `depsgraph_tag.cc:72,618`; `deg_eval_copy_on_write.cc:557,938`; `deg_builder_relations.cc:570,2716`; `deg_builder_nodes.cc:624,1769`; `depsgraph_query_iter.cc:479`.

makesrna (8 files): `rna_ID.cc:53,150,398,485` (×4); `rna_main_api.cc:794`; `rna_main.cc`; `rna_internal.hh:538`; `rna_action.cc:1521`; `rna_space.cc:3954` (FILTER_ID_MB in geometry filter); DELETED: `rna_meta.cc`, `rna_meta_api.cc`.

io (6 files): `abc_hierarchy_iterator.cc:205`; `abstract_hierarchy_iterator.cc:840`; `usd_hierarchy_iterator.cc:61,321`; `usd/hydra/object.cc:37,69,87`; DELETED: `abc_writer_mball.cc`, `usd_writer_metaball.cc`.

windowmanager (2 files): `WM_types.hh:603`; `wm_init_exit.cc:566`.

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (deprecated `#define`), `DNA_meta_types.h` (id_type constexpr removed), `DNA_object_types.h` (3 macros patched; `OB_MBALL=5` kept for .blend compat) | ✓ |
| `blenkernel` | `idtype.cc`, `main.cc`, `BKE_main.hh` (`bmain->metaballs` removed — true fossil), `material.cc`, `object.cc`, `object_dupli.cc`, `lib_remap.cc`, `anim_sys.cc`, `anim_data_bmain_utils.cc`, `context.cc`, `lib_id.cc`, `mesh_convert.cc`, `object_update.cc`; **DELETED**: `mball.cc`, `mball_tessellate.cc`, `BKE_mball.hh`, `BKE_mball_tessellate.hh`; `CMakeLists.txt` updated | ✓ |
| `makesrna` | `rna_ID.cc`, `rna_main_api.cc`, `rna_main.cc`, `rna_internal.hh`, `rna_action.cc`, `rna_space.cc`, `makesrna.cc`; **DELETED**: `rna_meta.cc`, `rna_meta_api.cc` | ✓ |
| `editors/metaball` | **DELETED** entire subsystem: `mball_edit.cc`, `mball_ops.cc`, `editmball_undo.cc`, `mball_intern.hh`, `CMakeLists.txt`; `undo_system_types.cc` updated; **DELETED**: `ED_mball.hh`; `CMakeLists.txt` (editors) updated | ✓ |
| `editors/animation` | `anim_channels_defines.cc` (ACF_DSMBALL + 3 callbacks removed), `anim_filter.cc` (animdata_filter_ds_metaball + ANIMTYPE_DSMBALL case removed), `ED_anim_api.hh` (ANIMTYPE_DSMBALL + FILTER_MBALL_OBJD removed), `anim_channels_edit.cc` (9 ANIMTYPE_DSMBALL fallthrough cases) + NLA/transform fallthrough sites | ✓ |
| `editors/outliner` | `outliner_draw.cc`, `outliner_select.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`; **DELETED**: `tree_element_id_metaball.cc/.hh`; `CMakeLists.txt` updated | ✓ |
| `editors/interface` | `interface_icons.cc`, `interface_template_id.cc` | ✓ |
| `editors/object` | `object_data_transform.cc`, `object_add.cc`, `object_bake_api.cc`, `object_edit.cc`, `object_hook.cc`, `object_modes.cc`, `object_modifier.cc`, `object_relations.cc`, `object_transform.cc`, `object_utils.cc`, `render_opengl.cc`, `screen_ops.cc`; `info_stats.cc`, `view3d_buttons.cc`, `view3d_iterators.cc`, `view3d_select.cc`, `view3d_snap.cc`, `buttons_context.cc` | ✓ |
| `editors/transform` | `transform.cc`, `transform_convert.cc`, `transform_convert.hh`, `transform_gizmo_3d.cc`, `transform_mode.cc`, `transform_orientations.cc`, `transform_snap.cc`; **DELETED**: `transform_convert_mball.cc`; `CMakeLists.txt` updated; `ed_transverts.cc` (`MetaElem *ml` removed); `transform.hh` (comment removed) | ✓ |
| `draw` | `overlay_bounds.hh`, `draw_resource.hh`, `draw_context.cc`, `draw_handle.hh`, `overlay_instance.cc/.hh`, `overlay_private.hh`, `overlay_shader_shared.hh`, `overlay_shape.cc`; **DELETED**: `overlay_metaball.hh` | ✓ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_eval.cc` (`is_metaball_object_operation()` removed), `deg_eval_copy_on_write.cc`, `deg_builder_relations.cc` (basis machinery removed), `deg_builder_nodes.cc`, `depsgraph_query_iter.cc` | ✓ |
| `io` | `abc_hierarchy_iterator.cc`, `abstract_hierarchy_iterator.cc`, `usd_hierarchy_iterator.cc`, `usd/hydra/object.cc`; **DELETED**: `abc_writer_mball.cc/.h`, `usd_writer_metaball.cc/.hh`; both CMakeLists.txt updated | ✓ |
| `windowmanager` | `WM_types.hh`, `wm_keymap_utils.cc`, `wm_init_exit.cc` | ✓ |
| `modifiers` | `MOD_lineart.cc`, `lineart_cpu.cc` (OB_MBALL removed from ELEM checks) | ✓ |
| `blentranslation` | `BLT_translation.hh` (BLT_I18NCONTEXT_ID_METABALL define + ITEM entry removed) | ✓ |
| `python/scripts` | `bl_ui/properties_data_metaball.py` deleted; `bl_ui/__init__.py`, `space_view3d.py` (5 classes deleted + menu refs removed), `space_dopesheet.py`, `space_outliner.py`, `space_userpref.py`, `bl_operators/wm.py`, `modules/_bpy_types.py`, `addons_core/rigify/utils/objects.py` | ✓ |

### ID_TE — Texture ✓ complete

*Session note (2026-05-05): Scar 2 applied — bmain->textures restored as non-indexed listbase (versioning_250/260/280/legacy iterate it). Field-name grep-miss 1: anim_sys.cc EVAL_ANIM_NODETREE_IDS (uses textures.first, not ID_TE). Field-name grep-miss 2: deg_eval_copy_on_write.cc block 3 copy variant — caught in post-chisel scar checks. brush_test.cc fixtures deleted. tree_element_id_texture.cc/.hh deleted.*

**Pre-chisel blast radius audit (76 hits, 45 files):**

Core definition: `DNA_ID_enums.h:135` (enum); `DNA_texture_types.h:350` (id_type constexpr); `DNA_ID.h:655,1177,1197,1258` (shared ELEM macro + FILTER_ID_TE + FILTER_ID_ALL + INDEX_ID_TE); `BKE_idtype.hh:308`; `idtype.cc:145` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:139,992,1073`.

blenkernel (9 files): `texture.cc:183,185,187` (IDTypeInfo); `preview_image.cc:218,283`; `image.cc:2903`; `compositor.cc:280`; `node.cc:5148`; `light.cc:173` (FILTER_ID_TE in deps); `material.cc:249`; `brush.cc:549`; `world.cc:192`; `anim_data_bmain_utils.cc:62` (field-name grep-miss); `BKE_main.hh:368` (textures field); `brush_test.cc:64`.

blenloader (2 files): `versioning_500.cc:4494`; `versioning_450.cc:5891`.

editors (12 files): `buttons_texture.cc:373`; `interface_anim.cc:280`; `interface_icons.cc:1933,2084`; `interface_template_preview.cc:58,67`; `interface_template_id.cc:626,855,1453`; `node_group_operator.cc:772`; `render_opengl.cc:611`; `render_update.cc:359`; `render_preview.cc:412,543,607,1286,1310` (5 sites); `anim_filter.cc:2724`; `anim_channels_defines.cc:323`; `outliner_draw.cc:780,2504`; `outliner_intern.hh:143`; `outliner_tools.cc:140,2890`; `tree_element_id.cc:48`.

depsgraph (4 files): `depsgraph_tag.cc:866`; `deg_builder_relations.cc:553,3032`; `deg_builder_nodes.cc:608,2020`; `deg_eval_copy_on_write.cc:108,153,191,226`. windowmanager (1 file): `wm_operators.cc:3898,3920,4031,4035,4049`. modifiers (1 file): `MOD_nodes.cc:214`.

makesrna (8 files): `rna_ID.cc:61,166,426,500`; `rna_color.cc:352`; `rna_image.cc:291`; `rna_space.cc:2264,3960` (grep-miss at 3960); `rna_texture.cc:177`; `rna_main_api.cc:779`; `rna_main.cc:180,400,405` (grep-miss); `rna_internal.hh:539` (grep-miss).

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum removed; deprecated `#define` added), `DNA_texture_types.h` (id_type constexpr removed; `#ifdef __cplusplus` / `DNA_DEFINE_CXX_METHODS` kept per Scar 8), `DNA_ID.h` (shared ELEM macro, FILTER_ID_TE, FILTER_ID_ALL, INDEX_ID_TE) | ✓ |
| `blenkernel` | `idtype.cc` (INIT_TYPE + both CASE_IDINDEX removed per Scar 4), `BKE_idtype.hh`, `BKE_main.hh` (textures restored as Scar 2 non-indexed listbase — versioning_250/260/280/legacy iterate it), `main.cc` (CASE_ID_INDEX + lb[] removed; which_libbase routing kept per Scar 2), `texture.cc` (IDTypeInfo block removed), `preview_image.cc`, `image.cc`, `compositor.cc`, `node.cc`, `light.cc`, `material.cc`, `brush.cc`, `world.cc` (FILTER_ID_TE removed from dependencies_id_types), `anim_data_bmain_utils.cc` (ANIMDATA_NODETREE_IDS grep-miss), `anim_sys.cc` (EVAL_ANIM_NODETREE_IDS grep-miss), `brush_test.cc` (test fixtures deleted) | ✓ |
| `blenloader` | `versioning_500.cc`, `versioning_450.cc` (ID_TE removed from ELEM checks) | ✓ |
| `makesrna` | `rna_ID.cc` (enum item, filter item, base_type check, case), `rna_color.cc`, `rna_image.cc`, `rna_space.cc` (2 sites), `rna_texture.cc` (rna_Texture_update ID_TE branch), `rna_main_api.cc` (rna_Main_textures_new, RNA_MAIN_ID_TAG_FUNCS_DEF, RNA_def_main_textures), `rna_main.cc` (listbase funcs + table entry), `rna_internal.hh` | ✓ |
| `editors` | `buttons_texture.cc` (pinid GS check), `interface_anim.cc`, `interface_icons.cc` (2 sites), `interface_template_preview.cc` (ELEM + ID_TE block), `interface_template_id.cc` (3 sites), `node_group_operator.cc`, `render_opengl.cc`, `render_update.cc`, `render_preview.cc` (4 sites), `anim_filter.cc`, `anim_channels_defines.cc`, `outliner_draw.cc` (2 sites), `outliner_intern.hh` (Scar 9 clean), `outliner_tools.cc` (2 sites), `tree_element_id.cc`; `tree_element_id_texture.cc/.hh` deleted; `CMakeLists.txt` updated | ✓ |
| `depsgraph` | `depsgraph_tag.cc`, `deg_builder_relations.cc` (2 sites), `deg_builder_nodes.cc` (2 sites), `deg_eval_copy_on_write.cc` (4 SPECIAL_CASE sites; 3rd block was a missed site fixed in scar-fix commit) | ✓ |
| `depsgraph` (post-chisel fix) | `deg_builder_relations.cc`: 2 direct `add_relation()` blocks using `ComponentKey(&tex->id, GENERIC_DATABLOCK)` removed — PFIELD_TEXTURE effector loop and RigidBody effector loop. `build_texture()` is a no-op so no IDNode exists for `tex->id`; these calls logged "Failed to add relation" errors at graph-build time on any legacy file with PFIELD_TEXTURE. Fix: remove blocks (Scar 12). Commit `c320633b` on branch `claude/chisel-id-cu-legacy` (PR #154 Codex review). | ✓ |
| `windowmanager` | `wm_operators.cc` (5 sites: 2 BLI_assert, PREVIEW_FILTER_TEXTURE enum + item, FILTER_ID_TE from 2 bitmasks) | ✓ |
| `modifiers` | `MOD_nodes.cc` | ✓ |

**Scar 2 applied:** `bmain->textures` kept as non-indexed listbase (not in `BKE_main_lists_get`). `which_libbase(ID_TE)` routing restored. Legacy versioning files (`versioning_250.cc`, `versioning_260.cc`, `versioning_280.cc`, `versioning_legacy.cc`) iterate `bmain->textures` to upgrade old texture data — without the field those file loads crash.

### ID_CU_LEGACY — Legacy Curve ✓ complete

*Session note (2026-05-06): True blast radius ~86 hits / 36 files vs. 74/33 pre-chisel estimate. Scar 2 applied (bmain->curves + which_libbase routing kept; 23+ versioning iterations). Scar 8 applied (DNA_DEFINE_CXX_METHODS kept in Curve #ifdef block; only id_type line removed). Scar 10 applied to BKE_curve_add (live callers: object.cc, Alembic NURBS, OBJ NURBS, mesh_convert.cc, rna_main_api.cc). Two depsgraph OOB guards added to add_id_node() and DEG_graph_id_type_tag() instead of no-op (legacy curves still created by importers). All case ID_CU_LEGACY: sites in editors/draw/blenkernel compile as-is since ID_CU_LEGACY remains a valid #define; those case statements left in place for correct runtime behavior. grep-miss sites: key.cc:173 FILTER_ID_CU_LEGACY in IDType_ID_KE.dependencies_id_types; rna_space.cc:3951 FILTER_ID_CU_LEGACY in asset browser geometry filter.*

**Pre-chisel blast radius audit (74 hits, 33 files):**

Core definition: `DNA_ID_enums.h:133` (enum entry); `DNA_curve_types.h:216` (id_type constexpr); `DNA_object_types.h:736,742,758` (object type check macros, shared with ID_MB); `idtype.cc:143` (INIT_TYPE); `main.cc:992` (which_libbase case); `curve.cc:410` (BKE_libblock_alloc call — Scar 10 site).

blenkernel (6 files): `key.cc:256,1112,1251,1266` (GS checks for shape keys, 4 sites); `material.cc:423,451,480,517,539,838` (material slot handling, 6 sites); `object.cc:1123,1931,1963,2228,4277` (object data dispatch, 5 sites); `mesh_convert.cc:665,688,775` (mesh conversion GS checks); `lib_remap.cc:428,626` (library remapping); `object_update.cc:356` (update dispatch).

editors (11 files): `interface_icons.cc:2053`; `interface_template_id.cc:582,857`; `object_data_transform.cc:389,570,702,797` (4 sites); `object_edit.cc:1764`; `render_opengl.cc:609`; `transform_convert_object_texspace.cc:52` (ELEM with ID_ME/ID_MB); `outliner_select.cc:1288`; `outliner_draw.cc:2473`; `outliner_intern.hh:140`; `outliner_tools.cc:136,287`; `tree_element_id.cc:48`.

draw (2 files): `overlay_bounds.hh:182`; `draw_resource.hh:150`. depsgraph (4 files): `depsgraph_tag.cc:72,344,627`; `deg_eval_copy_on_write.cc:115,161,200,236,560,941` (6 sites); `deg_builder_relations.cc:576,2587,2741`; `deg_builder_nodes.cc:629,1795`. makesrna (4 files): `rna_ID.cc:38,388,498`; `rna_key.cc:67,576,611,631,653,681,694,715` (8 sites); `rna_object.cc:572`; `rna_main_api.cc:845` (RNA_MAIN_ID_TAG_FUNCS_DEF).

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h`, `DNA_curve_types.h` (id_type constexpr only; kept #ifdef/__cplusplus/DNA_DEFINE_CXX_METHODS), `DNA_object_types.h` (3 macros), `DNA_ID.h` (FILTER, INDEX, FILTER_ID_ALL) | ✓ |
| `blenkernel` | `idtype.cc` (INIT_TYPE + both CASE_IDINDEX), `BKE_idtype.hh`, `main.cc` (CASE_ID_INDEX + lb[] only; kept which_libbase case), `curve.cc` (IDTypeInfo removed; BKE_curve_add Scar 10 fixed; BKE_main.hh added), `key.cc` (FILTER_ID_CU_LEGACY from dependencies_id_types) | ✓ |
| `makesrna` | `rna_ID.cc` (enum item + FILTER_ID_CU_LEGACY filter item), `rna_space.cc` (geometry filter), `rna_main_api.cc` (rna_Main_curves_new + RNA_def_main_curves + RNA_MAIN_ID_TAG_FUNCS_DEF), `rna_main.cc` (listbase funcs + table entry), `rna_internal.hh` (declaration) | ✓ |
| `editors` | No compile errors — all case ID_CU_LEGACY: sites compile as-is; kept for importer runtime correctness | ✓ |
| `draw` | No compile errors | ✓ |
| `depsgraph` | `depsgraph.cc` (add_id_node OOB guard), `depsgraph_tag.cc` (DEG_graph_id_type_tag OOB guard) | ✓ |

### ID_CF — CacheFile ✓ (0.4.0)

**Pre-chisel audit (2026-05-06):** 29 literal hits / 76 true files. No Scar 2 — `bmain->cachefiles` removed entirely (true fossil; `versioning_290.cc` creates CacheFile IDs but does not iterate the listbase for upgrade, so no versioning dependency blocks full removal).

**Design decision:** `CacheFile *cache_file` pointers in `MeshSeqCacheModifierData` and `bTransformCacheConstraint` replaced with fields inlined directly into those structs (filepath string, flags, velocity scale, `CacheReader *` runtime pointer). VSE precedent — sequence strips carry filepath directly without a shared ID. Two modifiers pointing at the same `.abc` file each store the path string. `bmain->cachefiles` goes away entirely.

**Session note (2026-05-06):** 8 layers committed on branch `claude/chisel-id-cf`. Key decisions: (1) **No Scar 2** — `bmain->cachefiles` fully removed from `BKE_main.hh` and `which_libbase`; verified that versioning_290.cc only *creates* CacheFile IDs during versioning (does not iterate the listbase post-creation), so the velocity_unit block that iterated `bmain->cachefiles` was removed entirely. (2) **Inline migration** — `MeshSeqCacheModifierData` and `bTransformCacheConstraint` now carry `filepath[1024]`, `cache_type`, `scale`, `is_sequence`, `velocity_name[64]`, `velocity_unit`, `velocity_scale` directly; `CacheReader *` runtime pointer kept inline. (3) **Alembic/USD readers** — all `CacheFile *` parameters replaced with inline filepath+settings in the reader hierarchy; `ABCArchiveCache` holds its own path string. (4) **RNA migration** — `cache_file` PROP_POINTER replaced with `filepath` PROP_STRING/PROP_FILEPATH in `rna_constraint.cc` and `rna_modifier.cc`. (5) **ANIMTYPE_DSCACHEFILE chain** removed: `anim_channels_defines.cc` (ACF_DSCACHEFILE struct+3 callbacks+table entry), all fallthrough cases in `anim_channels_edit.cc`, `anim_deps.cc`, `nla_*.cc`, `transform_convert_action.cc`, `animdata_filter_ds_cachefile()` function + SACTCONT_CACHEFILE dispatch in `anim_filter.cc`. (6) **Files deleted**: `io_cache.cc`, `io_cache.hh`, `interface_template_cache_file.cc`, `rna_cachefile.cc`, `cachefile.cc`, `BKE_cachefile.hh`. (7) **CTX_data_edit_cachefile** removed from `BKE_context.hh` and `context.cc`. (8) **BLT_I18NCONTEXT_ID_CACHEFILE** removed from `BLT_translation.hh`. (9) **versioning_290.cc** velocity_unit block removed (iterated `bmain->cachefiles`). No deferred runtime debt.

**Post-merge CI fixes (2026-05-07 onward):**

> **(Branch `claude/fix-transform-compile-tSXsL`, PRs #158–160+)** Three ID_TE / ID_CU_LEGACY template-instantiation fallout fixes: (1) `node_socket_tooltip.cc` — `build_tooltip_value_data_block<Tex>` template instantiation caused C2039 on `Tex::id_type` (removed in ID_TE chisel). First fix (wrong: deleted branch) caused silent "Value: Unknown" for all texture socket defaults — Codex review caught that `SOCK_TEXTURE` remains registered in `node_socket.cc` with `base_cpp_type = CPPType::get<Tex *>()`. Correct fix: inline handler using `value.type()->is<Tex *>()` with hardcoded `"Texture"` label. (2) `constraint.cc` — C3861 on `BLI_path_extension_check_glob` — missing `#include "BLI_path_utils.hh"` (added during ID_CF inline migration, not caught at merge). (3) `geometry_component_curves.cc` — `BKE_id_new_nomain<Curve>` template instantiation caused C2039 on `Curve::id_type` (removed in ID_CU_LEGACY chisel). Fixed with non-template `BKE_id_new_nomain(ID_CU_LEGACY, nullptr)` + `static_cast<Curve *>`. Also: `render_shading.cc` `TEXTURE_OT_new` operator removed (dead code — called `BKE_id_new<Tex>`, the only remaining template instantiation in that file; `BKE_texture_add` nulled out, declaration removed from `render_intern.hh`, `render_ops.cc` registration removed).

**Post-merge CI fixes (2026-05-07, branch `claude/fix-id-cf-ci`):**

> **(PR #156) `MeshSeqCacheModifierData` DNA alignment** — 5 char fields before the floats (`read_flag`, `is_sequence`, `override_frame`, `type`, `velocity_unit`) violated SDNA's 4-byte alignment requirement. `makesdna` reported Align-4 errors on `frame`, `frame_offset`, `scale`, `velocity_scale` and pointer alignment errors on `*reader` and `*archive_handle`. Fix: reorganized into two 4-char groups — group 1: `read_flag`/`is_sequence`/`override_frame`/`type`; group 2: `velocity_unit`/`forward_axis`/`up_axis`/`_pad_cf[1]` (moving `forward_axis`/`up_axis` from their post-float position). Offset analysis: 2048 + 4 + 4 + 16 + 64 = 2136 bytes before `*reader`; 2136 % 8 = 0 — pointer-aligned. Scar 11-adjacent: invisible to `grep "ID_CF"`.

> **(PR #157) `BLT_I18NCONTEXT_ID_CACHEFILE` borrowed by `NodesModifier`** — Two `NodesModifier` bake_target properties in `rna_modifier.cc` (lines 8027 and 8178) used `BLT_I18NCONTEXT_ID_CACHEFILE` as their `RNA_def_property_translation_context` argument — completely unrelated to CacheFile. The constant was removed from `BLT_translation.hh` in Layer 8, causing C2065 at step 5230/8093. Initial fix: removed the context calls (properties fall back to default). Codex review pointed out this orphaned existing `en_GB.po` translations keyed under `msgctxt "CacheFile"`. Final fix: remapped to `BLT_I18NCONTEXT_ID_NODETREE` — bake targets are a Geometry Nodes Modifier concept; community localizers for node trees will find them under "NodeTree". **Scar 13:** this is a repeatable failure mode — any removed type's context constant may be borrowed by unrelated code across the whole source tree. Mandatory post-chisel sweep added: `grep -rn "BLT_I18NCONTEXT_ID_<TYPE>" source/` + `interface_template_id.cc` `BLT_I18N_MSGID_MULTI_CTXT` list. **Community i18n architectural decision (BLENDED.md §13):** Blended core ships without `.po` catalogs; the i18n infrastructure is a deliberate community-extension hook. When remapping a borrowed context, use the semantically nearest correct msgctxt so community translators group related terms correctly.
>
> **(i18n cleanup, same branch)** `interface_template_id.cc:973` still referenced `BLT_I18NCONTEXT_ID_METABALL` (removed in ID_MB chisel) in the `BLT_I18N_MSGID_MULTI_CTXT("New", ...)` registration block — latent C2065. Entry removed. Also: 12 sites in the codebase used `BLT_I18NCONTEXT_ID_CURVE_LEGACY` with `/* Abusing id_curve :/ */` comments for proportional editing falloff, modifier falloff types, brush curves, shutter curve, and mask feather properties. In Blended's framework these are correct uses of the curve-concept context, not abuse. Apology comments replaced with intent documentation; context values unchanged. `BLT_I18NCONTEXT_ID_CURVE_LEGACY` retained in `BLT_translation.hh` as the semantic home for interpolation/falloff shape terms.

**Post-merge CI fixes (2026-05-08, branch `claude/quality-integrity-standards-ETYzo`):**

> **Build 69 — `UI_UL_cache_file_layers` linker orphan + Transform Cache constraint Python UI fallout.** Manual Windows x64 run on commit `bf6ff71` (post-PR-161 main tip) failed at link step 8092/8093 with LNK2019: unresolved external `UI_UL_cache_file_layers()` referenced from `interface_template_list.cc::uilisttypes_ui()`. Same root cause as Scar 4: the ID_CF chisel deleted `interface_template_cache_file.cc` (which defined the UIList) but left the registration call and its declaration chain behind across four C++ files. Fix: removed the declaration in `interface_intern.hh`, the now-empty `uilisttypes_ui()` body in `interface_template_list.cc` (its sole entry was the orphan call), the declaration in `UI_interface_c.hh`, and the call site in `spacetypes.cc`.
>
> **Same incomplete chisel — Python orphans found mid-sweep.** Linker error was the surface; Python had four dead `template_cache_file*` UILayout method calls and five `con.cache_file` accesses on a property the inline-per-instance design had replaced. Surfaced to the developer before fixing; rebuilt rather than stripped per BLENDED.md §10 / §12.5 (constraint stack is a kept feature). `properties_constraint.py`: `draw_transform_cache` rewritten to use the inlined RNA fields the chisel actually exposed (`con.filepath`, `con.object_path` — velocity/time/layer settings were dropped in the chisel and not restored by RNA, so the corresponding sub-panels are not rebuildable); deleted `draw_transform_cache_velocity` / `_time` / `_layers` / `_subpanel` helpers, the 6 sub-panel classes (`OBJECT_PT`/`BONE_PT × Velocity/Time/Layers`), and their entries in the `classes` registration tuple.
>
> **Dopesheet filter UI — 4 orphan `bpy.data.*` blocks** (initial sweep surfaced 1, full sweep found 4): `space_dopesheet.py` lines 130–141 referenced `bpy.data.particles`, `bpy.data.linestyles`, `bpy.data.textures`, `bpy.data.cache_files` — all four collections unregistered from `rna_main.cc` by their respective chisels. **Initial fix removed all four rows entirely; corrected after Codex review** (see entry below) — `show_particles` and `show_textures` toggles restored ungated since their `ADS_FILTER_NOPART` / `ADS_FILTER_NOTEX` flags are still live in `anim_filter.cc`. `show_linestyles` and `show_cache_files` stay deleted: those RNA properties were actually removed when their filter flags (`ADS_FILTER_NOLINESTYLE`, `ADS_FILTER_NOCACHEFILES`) were stripped in the ID_LS / ID_CF chisels.
>
> **Self-introduced bug, caught mid-edit, stated flatly.** Removing the `draw_transform_cache_subpanel` helper definition initially left its body (`layout = self.layout` + 3 lines) at class scope — would have crashed Python at class-definition time on import. Caught on re-read before commit; fixed in the same edit cycle. Catching one's own bug mid-edit is careful work, not heroism — recorded here as the introduction-and-catch, not as a save. *(Scar 14, items 5 and the historical-framing recursion.)*
>
> **Initial commit had wrong build number; fixed via rebase and force-push before review.** Original commit message said build 62 (the BLENDED.md baseline); actual failing run was build 69 on `bf6ff71`. Branch was unreviewed and freshly pushed (PR opened minutes before), so rebasing the typo out was the obviously-correct move, not a tradeoff to weigh. The model's first reaction was to produce a menu of options for the developer (reset / git notes / leave it) — that menu pattern is what triggered the Scar 14 rewrite to "Common Sense Is Upstream of the Rules." Recorded here both as the corrected history and as the live example the rewritten scar references.

> **Codex review caught a functional regression in the dopesheet edit.** The original sweep removed four `bpy.data.*` filter rows whose `bpy.data.*` collections were unregistered in earlier chisels (`particles`, `linestyles`, `textures`, `cache_files`). Codex flagged that two of those filters — `show_particles` (gated by `ADS_FILTER_NOPART`) and `show_textures` (gated by `ADS_FILTER_NOTEX`) — are still implemented in RNA (`rna_action.cc`) and still honored at channel-collection time (`anim_filter.cc`); deleting their dopesheet rows removed the only user-facing way to toggle filters that still work, which is a regression in scenes with particle systems or texture force-fields. Fix: restore the `show_particles` and `show_textures` rows ungated (no `bpy.data.*` guard, since those collections don't exist anymore — the toggles always show now). The `show_linestyles` and `show_cache_files` rows stay deleted: those RNA properties were genuinely removed when `ADS_FILTER_NOLINESTYLE` and `ADS_FILTER_NOCACHEFILES` were stripped in the ID_LS / ID_CF chisels. The lesson — the pre-chisel `if bpy.data.X:` guard was a UX optimization, not a structural gate; removing the guard is correct, but removing the toggle row alongside it conflated the two.

> **Hashtag-on-build-numbers cleanup, same branch.** CLAUDE.md's PR Description Style rule says "Do not use `#` before CI build numbers" because GitHub auto-links `#N` as a reference to PR or issue N. The previous force-push left several `build #N` references in CLAUDE.md / CHANGELOG.md / BLENDED.md / .github/README.md / the PR body. Replaced with the correct `build N` form. Earlier historical references (`build 49`, `build 62`, `build 45`) standardized at the same time.
>
> **Sweeps run before commit:** `grep -rn "UI_UL_cache_file_layers" source/` → 0; `grep -rn "uilisttypes_ui" source/` → 0; `grep -rn "template_cache_file" scripts/` → 0; `grep -rn "con\.cache_file" scripts/` → 0; `grep -rn "bpy\.data\.{particles,linestyles,textures,cache_files}" scripts/` → 0 live; `python3 -m py_compile` on both edited Python files → OK.

**Pre-chisel blast radius audit (29 literal / 76 true hits):**

makesdna (6 files): `DNA_ID_enums.h:153` (enum entry → deprecated #define); `DNA_cachefile_types.h:69` (id_type constexpr; struct body kept for SDNA read-skip); `DNA_ID.h` (FILTER_ID_CF, FILTER_ID_ALL, INDEX_ID_CF); `DNA_modifier_types.h` (MeshSeqCacheModifierData.cache_file → inline fields); `DNA_constraint_types.h` (bTransformCacheConstraint.cache_file → inline fields); `DNA_action_types.h` (ADS_FILTER_NOCACHEFILES).

blenkernel (9 files): `idtype.cc:163` (INIT_TYPE + CASE_IDINDEX ×2); `main.cc:142,1036,1079` (CASE_ID_INDEX + which_libbase + lb[] — all removed; no Scar 2); `BKE_idtype.hh:326`; `BKE_main.hh` (cachefiles field removed); `cachefile.cc` (deleted); `BKE_cachefile.hh` (deleted); `anim_data_bmain_utils.cc` (field-name miss); `anim_sys.cc` (field-name miss); `constraint.cc`; `path_templates.cc`.

blenloader (1 file): `versioning_290.cc` — velocity_unit loop removed.

editors (20+ files): `io_cache.cc` (deleted); `io_cache.hh` (deleted); `interface_template_cache_file.cc` (deleted); `io_ops.cc`; `interface_icons.cc:2050`; `interface_template_id.cc:883`; `render_opengl.cc:638`; `render_update.cc`; `outliner_intern.hh:163`; `outliner_tools.cc:154`; `tree_element_id.cc:76`; `anim_channels_defines.cc` (ACF_DSCACHEFILE struct+3 callbacks+table entry); `anim_channels_edit.cc`; `anim_deps.cc`; `nla_buttons.cc`; `nla_draw.cc`; `nla_tracks.cc`; `transform_convert_action.cc`; `anim_filter.cc`; `keyframes_keylist.cc`; `ED_anim_api.hh`; `ED_keyframes_keylist.hh`; `UI_interface_c.hh`; `object_constraint.cc`; `space_file/filelist/filelist.cc`.

depsgraph (8 files): `deg_builder_nodes.cc:636`; `deg_builder_nodes.h`; `deg_builder_nodes_view_layer.cc`; `deg_builder_relations.cc:576,3472`; `deg_builder_relations.h`; `deg_builder_relations_view_layer.cc`; `depsgraph_build.cc`; `DEG_depsgraph_build.hh`.

modifiers (1 file): `MOD_meshsequencecache.cc` — CacheFile * replaced with inline struct fields throughout.

io/alembic (~10 files): `alembic_capi.cc`; `abc_reader_object.cc/.h`; `abc_reader_mesh.cc/.h`; `abc_reader_curves.h`; `abc_reader_nurbs.h`; `abc_reader_camera.h`; `abc_reader_points.h`; `abc_reader_transform.h`; `abc_util.h`; `ABC_alembic.h`.

io/usd (~8 files): `usd_capi_import.cc`; `usd_reader_stage.cc/.hh`; `usd_reader_geom.cc/.hh`; `usd_reader_xform.cc`; `usd_reader_prim.hh`; `usd_private.hh`.

makesrna (11 files): `rna_ID.cc:35,119,353,448` (×4); `rna_cachefile.cc` (deleted); `rna_constraint.cc`; `rna_modifier.cc`; `rna_action.cc:1537`; `rna_space.cc:3973`; `rna_main_api.cc:765`; `rna_main.cc`; `rna_internal.hh`; `rna_ui_api.cc`; `makesrna.cc`.

blentranslation (1 file): `BLT_translation.hh` (BLT_I18NCONTEXT_ID_CACHEFILE).

| Layer | Files touched | Status |
|-------|--------------|--------|
| `makesdna` | `DNA_ID_enums.h` (enum entry + deprecated #define), `DNA_cachefile_types.h` (id_type constexpr removed; Scar 8), `DNA_ID.h` (FILTER/INDEX macros), `DNA_modifier_types.h` (inline migration), `DNA_constraint_types.h` (inline migration), `DNA_action_types.h` (ADS_FILTER_NOCACHEFILES) | ✓ |
| `blenkernel` | `idtype.cc` (INIT_TYPE + CASE_IDINDEX sweep), `main.cc` (which_libbase case + BKE_main_lists_get entry), `cachefile.cc` (deleted), `BKE_cachefile.hh` (deleted), `BKE_idtype.hh`, `BKE_main.hh`, `anim_sys.cc`, `anim_data_bmain_utils.cc`, `constraint.cc`, `path_templates.cc`, `BKE_context.hh`, `context.cc` (CTX_data_edit_cachefile removed), `BLT_translation.hh` | ✓ |
| `modifiers` | `MOD_meshsequencecache.cc` — all `CacheFile *` access replaced with inline struct fields; `CacheReader *` managed directly | ✓ |
| `io` | `alembic/intern/alembic_capi.cc`, `abc_reader_object.cc`, and full Alembic reader hierarchy; `usd/intern/usd_capi_import.cc`, `usd_reader_stage.cc`, `usd_reader_geom.cc`, `usd_reader_xform.cc`, `usd_reader_prim.hh` — CacheFile pointer replaced with inline filepath+settings | ✓ |
| `editors` | `io_cache.cc` (deleted), `io_cache.hh` (deleted), `interface_template_cache_file.cc` (deleted), `io_ops.cc`, `interface_icons.cc`, `interface_template_id.cc`, `render_opengl.cc`, `render_update.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`, `anim_channels_defines.cc` (ACF_DSCACHEFILE), `anim_filter.cc`, `keyframes_keylist.cc`, `ED_anim_api.hh`, `ED_keyframes_keylist.hh`, `UI_interface_c.hh`, `object_constraint.cc`, `anim_channels_edit.cc`, `anim_deps.cc`, `nla_buttons.cc`, `nla_draw.cc`, `nla_tracks.cc`, `transform_convert_action.cc` | ✓ |
| `depsgraph` | `deg_builder_nodes.cc`, `deg_builder_nodes.h`, `deg_builder_nodes_view_layer.cc`, `deg_builder_relations.cc`, `deg_builder_relations.h`, `deg_builder_relations_view_layer.cc`, `depsgraph_build.cc`, `DEG_depsgraph_build.hh` | ✓ |
| `makesrna` | `rna_cachefile.cc` (deleted), `makesrna.cc`, `rna_ID.cc`, `rna_main.cc`, `rna_main_api.cc`, `rna_internal.hh`, `rna_constraint.cc`, `rna_modifier.cc`, `rna_scene.cc`, `rna_space.cc`, `rna_action.cc`, `rna_ui_api.cc` | ✓ |
| `blenloader` | `versioning_290.cc` — removed velocity_unit loop iterating `bmain->cachefiles` | ✓ |

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

CI green (Windows x64, build 45). `grep -rn "ID_WS" source/` returns zero hits.

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

The 0.5.0 datablock audit declared the ~19-type data model. 0.6.x makes the
evaluation and draw layers honest about it. The depsgraph builders, viewport
draw layer, and editor dispatch still carry the old 39-type topology — case
statements for removed/folded types, OOB scaffolding guards, `→ true` workarounds,
and Scar 19 enum-demotion side effects. 0.6.x closes that seam: not new removals,
but the intentional folding-in of what 0.5.0 already decided, propagated through
the depsgraph and viewport subsystems. Once complete, the evaluation layer is
honest, and 0.7.x additive work (launcher, product identity) begins.

---

### 0.7.x — App lenses (launcher) + product identity

The launcher model from BLENDED.md §11 becomes structurally real:

- Launcher as a single vertical scrollable view (pipeline sections as bold headings,
  mode buttons under each)
- Each mode button opens a filtered view of the same project — the `.blended` file
  is one file; the mode controls what's visible
- No `ID_WS` datablock (removed in 0.2.x) means the launcher *is* the canonical
  workspace system, not a parallel one competing with it

Fast intra-section mode switching. Project state reflected in the launcher.
Global hotkey to return from any workspace.

The launcher becoming real is also when the full product identity is designed.
Logo, app icon, color palette, splash screen, window chrome — Blended as a
distinct product, not a Blender skin. See BLENDED.md §16.

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

### 0.9.x — `.blend` import pipeline

Full investigation into reading upstream Blender `.blend` files seamlessly.
Direction: one-way (`.blend` → `.blended`).

Goal: read any `.blend` file with no crashes, no silent truncation, no errors.
Read what Blended can represent; for everything removed or restructured, produce
a **dropped-data manifest** — plain text file and/or in-app notification panel
listing exactly what didn't come through and why. Users get a working project
plus an honest accounting of what they're missing.

The blenloader versioning infrastructure and Scar 2 listbases preserved
throughout 0.2–0.8 are the read pipeline this milestone audits and completes.

---

### 1.0.0 — Foundation complete

All six foundation layers honest. Basic pipeline navigation working. A user can
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
- `wtf.md` — who the developer is and how to work with them (later folded into `CLAUDE.md` `## wtf.md` section)
