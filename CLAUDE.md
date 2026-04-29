# CLAUDE.md — Blended Project Context

Blended is a fork of Blender 5.2 (GPL-2.0-or-later) being rebuilt from the foundation up.

**Read `BLENDED.md` first.** It is the design authority — identity, architecture, datablock audit, pipeline specs, locked decisions, open questions, and guardrails. This file is operational context for Claude sessions: what's been built, what the patterns are, what not to repeat.

**Current version:** Blended 0.3.0 — `ID_SCR` and `ID_WM` removed from ID type system; CI green (Windows x64, build #49). Next: Bucket 5 + 6 fossil removals (0.4.x).

---

## What Blended Is Now

Not a tiered UI skin. Not a WASM port. A rebuild — subtracting Blender down to its true shape, then restructuring around one stated identity: **free 2D and 3D software tools, with an explicit focus on the craft of animation.**

The old approach (tiered UI, smart defaults, Emscripten) was prototyping toward the real vision. Don't propose reinstating it without re-reading `BLENDED.md` §8 Guardrails first.

**Foundation-first build order (from BLENDED.md §4):**
1. File format — `.blended` is the project, period
2. Datablocks — 39 → ~19 ID types (fossils and UI-state removed)
3. Evaluation model — depsgraph audit
4. App lenses — launcher as canonical workspace system
5. UI — only after 1–4 are honest

**`ID_WS` (WorkSpace) removal — compile-clean.** All layers merged (`makesdna`, `blenkernel`, `makesrna`, `editors`, `depsgraph`, `python`, `windowmanager`). `grep -rn "ID_WS" source/` returns zero hits. CI green. Tagged 0.2.0. Runtime debt (workspace cycle, reorder operators, factory name translation) documented in Scar 1 below.

**`ID_SCR` and `ID_WM` removal — compile-clean, pending CI.** All layers merged. The blast radius was enormous — see Scar 2 below. Key architectural outcome: `bmain->screens` and `bmain->wm` kept as non-indexed runtime listbases; `ID_SCR_LEGACY` / `ID_WM_LEGACY` defines route through `which_libbase` for allocation but are excluded from `BKE_main_lists_get`. Branch: `claude/remove-id-scr-id-wm`. Layer-by-layer status in [`CHANGELOG.md`](CHANGELOG.md).

**Next: Bucket 5 + 6 fossil removals (0.4.x)** — `ID_CU_LEGACY`, `ID_GD_LEGACY`, `ID_TE`, `ID_PA`, `ID_MB`, `ID_LS`, `ID_SPK`, `ID_PC`, `ID_CF`. Same chisel pattern. See roadmap in CHANGELOG.md.

Pattern for each pending layer: `grep -rn "ID_WS"` the directory, delete or redirect every hit. The breakage is the audit — follow the compile errors, don't paper over them.

### Bucket 5 + 6 Blast Radius Audit (pre-chisel)

Grepped 2026-04-29 before starting the removal. Use this as the checklist.

**ID_CU_LEGACY — 74 hits, 33 files**

Core definition:
- `makesdna/DNA_ID_enums.h:133` — enum entry `ID_CU_LEGACY = MAKE_ID2('C', 'U')`
- `makesdna/DNA_curve_types.h:216` — `static constexpr ID_Type id_type = ID_CU_LEGACY`
- `makesdna/DNA_object_types.h:736,742,758` — object type check macros (shared with ID_MB)
- `blenkernel/intern/idtype.cc:143` — `INIT_TYPE(ID_CU_LEGACY)`
- `blenkernel/intern/main.cc:992` — `which_libbase` case
- `blenkernel/intern/curve.cc:410` — `BKE_libblock_alloc(bmain, ID_CU_LEGACY, name, 0)` — curve creation

blenkernel (6 files):
- `key.cc:256,1112,1251,1266` — GS checks for shape keys (4 sites)
- `material.cc:423,451,480,517,539,838` — material slot handling (6 sites)
- `object.cc:1123,1931,1963,2228,4277` — object data dispatch (5 sites)
- `mesh_convert.cc:665,688,775` — mesh conversion GS checks
- `lib_remap.cc:428,626` — library remapping
- `object_update.cc:356` — update dispatch

editors (11 files):
- `interface_icons.cc:2053` — icon switch case
- `interface_template_id.cc:582,857` — template checks
- `object_data_transform.cc:389,570,702,797` — data transform dispatch (4 sites)
- `object_edit.cc:1764` — GS check
- `render_opengl.cc:609` — render switch
- `transform_convert_object_texspace.cc:52` — `ELEM(GS(id->name), ID_ME, ID_CU_LEGACY, ID_MB)` (shared with MB)
- `outliner_select.cc:1288` — outliner select
- `outliner_draw.cc:2473` — outliner draw
- `outliner_intern.hh:140` — outliner macro
- `outliner_tools.cc:136,287` — outliner tools
- `tree_element_id.cc:48` — tree element

draw (2 files):
- `overlay_bounds.hh:182` — bounds overlay
- `draw_resource.hh:150` — draw resource

depsgraph (4 files):
- `depsgraph_tag.cc:72,344,627` — tag dispatch (shared ELEM with MB/GD_LEGACY)
- `deg_eval_copy_on_write.cc:115,161,200,236,560,941` — COW special cases (6 sites)
- `deg_builder_relations.cc:576,2587,2741` — relation builder
- `deg_builder_nodes.cc:629,1795` — node builder

makesrna (4 files):
- `rna_ID.cc:38,388,498` — RNA enum entry and GS switch cases
- `rna_key.cc:67,576,611,631,653,681,694,715` — shape key RNA (8 sites)
- `rna_object.cc:572` — object RNA
- `rna_main_api.cc:845` — `RNA_MAIN_ID_TAG_FUNCS_DEF(curves, curves, ID_CU_LEGACY)`

---

**ID_GD_LEGACY — 39 hits, 28 files**

Core definition:
- `makesdna/DNA_ID_enums.h:153` — enum entry `ID_GD_LEGACY = MAKE_ID2('G', 'D')`
- `makesdna/DNA_gpencil_legacy_types.h:711` — `static constexpr ID_Type id_type = ID_GD_LEGACY`
- `makesdna/DNA_object_types.h:749,765` — object type macros
- `blenkernel/intern/idtype.cc:163` — `INIT_TYPE(ID_GD_LEGACY)`
- `blenkernel/intern/main.cc:1034` — `which_libbase` case
- `blenkernel/intern/gpencil_legacy.cc:654` — `BKE_libblock_alloc(bmain, ID_GD_LEGACY, name, 0)` — data creation

blenkernel (4 files):
- `material.cc:427,455,850` — material handling
- `deform.cc:460,481` — deform data GS check
- `grease_pencil_convert_legacy.cc:3057,3151` — conversion type-safety asserts
- `blendfile_link_append.cc:555` — link/append: forces conversion on append

blenloader (2 files):
- `versioning_250.cc:444` — `*(short *)id->name = ID_GD_LEGACY` (legacy compat; becomes skip)
- `versioning_common.cc:61,62` — GD_LEGACY → GP v3 conversion marker

editors (9 files):
- `interface_icons.cc:2055` — icon case
- `interface_template_id.cc:589,893` — template checks
- `object_data_transform.cc:816` — data transform
- `render_opengl.cc:652` — render switch
- `outliner_select.cc:1296` — outliner select
- `outliner_draw.cc:2557` — outliner draw
- `outliner_intern.hh:158` — outliner macro
- `outliner_tools.cc:158` — outliner tools
- `tree_element_id.cc:56` — tree element
- `space_node.cc:1537` — GS check for node space

draw (1 file):
- `draw_context.cc:1166` — `DEG_id_type_any_exists(depsgraph, ID_GD_LEGACY)` check

depsgraph (3 files):
- `depsgraph_tag.cc:72,639` — tag dispatch
- `deg_builder_nodes.cc:631` — node builder
- `deg_builder_relations.cc:581,2777` — relation builder

makesrna (2 files):
- `rna_ID.cc:41,391,500` — RNA enum entry and switch cases
- `rna_main_api.cc:860` — `RNA_MAIN_ID_TAG_FUNCS_DEF(gpencils, gpencils, ID_GD_LEGACY)`

---

**ID_TE — 58 hits, 39 files**

Core definition:
- `makesdna/DNA_ID_enums.h:136` — enum entry `ID_TE = MAKE_ID2('T', 'E')`
- `makesdna/DNA_texture_types.h:350` — `static constexpr ID_Type id_type = ID_TE`
- `makesdna/DNA_ID.h:655` — `ELEM(id_type, ID_BR, ID_TE, ID_NT, ID_IM, ID_PC, ID_MA)` shared macro
- `blenkernel/intern/idtype.cc:146` — `INIT_TYPE(ID_TE)`
- `blenkernel/intern/main.cc:998` — `which_libbase` case

blenkernel (4 files):
- `preview_image.cc:218,283` — preview image handling
- `image.cc:2903` — image switch case
- `compositor.cc:280` — compositor case
- `node.cc:5148` — node tree case
- `brush_test.cc:65` — test fixture: `BKE_id_new(bmain, ID_TE, ...)` (test-only; delete with type)

blenloader (2 files):
- `versioning_500.cc:4494` — ELEM check (shared with ID_LS)
- `versioning_450.cc:5891` — ELEM check (shared with ID_LS)

editors (12 files):
- `buttons_texture.cc:389` — pin ID GS check
- `interface_anim.cc:280` — GS check
- `interface_icons.cc:1933,2095` — icon switch (2 sites)
- `interface_template_preview.cc:59,68` — preview template ELEM/GS checks
- `interface_template_id.cc:629,863,1474` — template checks (3 sites)
- `node_group_operator.cc:772` — returns `ID_TE` as default
- `render_opengl.cc:612` — render switch
- `render_update.cc:359` — render update switch
- `render_preview.cc:412,543,607,1286,1310` — preview rendering (5 sites)
- `anim_filter.cc:2806` — animation filter case
- `anim_channels_defines.cc:325` — channel defines GS check
- `outliner_draw.cc:780,2506` — outliner draw (2 sites)
- `outliner_intern.hh:144` — outliner macro
- `outliner_tools.cc:144,2930` — outliner tools (2 sites)
- `tree_element_id.cc:52` — tree element

depsgraph (4 files):
- `depsgraph_tag.cc:881` — ID type tag
- `deg_builder_relations.cc:556,3085` — relation builder
- `deg_builder_nodes.cc:609,2048` — node builder
- `deg_eval_copy_on_write.cc:112,158,197,233` — COW special cases (nodetree; 4 sites)

windowmanager (1 file):
- `wm_operators.cc:3898,3920` — ELEM checks in operator

modifiers (1 file):
- `MOD_nodes.cc:214` — geometry nodes modifier case

makesrna (6 files):
- `rna_ID.cc:65,466,550` — RNA enum entry and switch cases
- `rna_color.cc:371` — color ramp case
- `rna_image.cc:291` — image RNA GS check
- `rna_space.cc:2264` — space RNA case
- `rna_texture.cc:177` — texture RNA GS check
- `rna_main_api.cc:848` — `RNA_MAIN_ID_TAG_FUNCS_DEF(textures, textures, ID_TE)`

---

**ID_PA — 35 hits, 28 files**

Core definition:
- `makesdna/DNA_ID_enums.h:152` — enum entry `ID_PA = MAKE_ID2('P', 'A')`
- `makesdna/DNA_particle_types.h:533` — `static constexpr ID_Type id_type = ID_PA`
- `blenkernel/intern/idtype.cc:162` — `INIT_TYPE(ID_PA)`
- `blenkernel/intern/main.cc:1032` — `which_libbase` case

blenkernel (1 file):
- `texture.cc:486,516` — texture slot handling (2 sites)

editors (9 files):
- `buttons_context.cc:517` — buttons context GS check
- `interface_icons.cc:2081` — icon case
- `interface_template_id.cc:636,891` — template checks
- `render_shading.cc:3045,3075` — render shading (2 sites)
- `render_opengl.cc:624` — render switch
- `outliner_draw.cc:2585` — outliner draw
- `outliner_intern.hh:157` — outliner macro
- `outliner_tools.cc:157` — outliner tools
- `tree_element_id.cc:77` — tree element
- `anim_filter.cc:2676` — animation filter case
- `anim_channels_defines.cc:329` — `ELEM(GS(...), ID_MA, ID_PA)` (shared with MA)

depsgraph (4 files):
- `depsgraph_tag.cc:157,635` — tag dispatch (2 sites)
- `deg_builder_relations.cc:600` — relation builder
- `deg_builder_nodes.cc:653` — node builder
- `depsgraph.cc:160` — `clear_id_nodes_conditional` lambda: `id_type != ID_PA`

animrig (1 file):
- `animdata.cc:125` — animdata case

makesrna (7 files):
- `rna_ID.cc:59,442,534,1050` — RNA enum entry and switch cases (4 sites)
- `rna_texture.cc:272` — texture slot RNA
- `rna_particle.cc:1014` — `GS(id->name) == ID_PA` check
- `rna_boid.cc:232` — `GS(id->name) == ID_PA` check
- `rna_color.cc:386` — color ramp case
- `rna_object_force.cc:671` — object force RNA check
- `rna_main_api.cc:858` — `RNA_MAIN_ID_TAG_FUNCS_DEF(particles, particles, ID_PA)`

---

**ID_MB — 49 hits, 28 files**

Core definition:
- `makesdna/DNA_ID_enums.h:134` — enum entry `ID_MB = MAKE_ID2('M', 'B')`
- `makesdna/DNA_meta_types.h:91` — `static constexpr ID_Type id_type = ID_MB`
- `makesdna/DNA_object_types.h:736,743,759` — object type macros (shared with ID_CU_LEGACY)
- `blenkernel/intern/idtype.cc:144` — `INIT_TYPE(ID_MB)`
- `blenkernel/intern/main.cc:994` — `which_libbase` case

blenkernel (5 files):
- `material.cc:425,453,486,519,542,847` — material slot handling (6 sites)
- `object.cc:1934,1978,2230,4291` — object data dispatch (4 sites)
- `object_dupli.cc:312` — dupli GS check
- `lib_remap.cc:627` — library remapping
- `mesh_convert.cc` — note: shared ELEM with CU_LEGACY at `transform_convert_object_texspace.cc:52`

editors (10 files):
- `interface_icons.cc:2069` — icon case
- `interface_template_id.cc:583,859` — template checks
- `object_data_transform.cc:442,612,736,810` — data transform dispatch (4 sites)
- `transform_convert_object_texspace.cc:52` — `ELEM(GS(id->name), ID_ME, ID_CU_LEGACY, ID_MB)` (shared)
- `render_opengl.cc:610` — render switch
- `outliner_select.cc:1289` — outliner select
- `outliner_draw.cc:2485` — outliner draw
- `outliner_intern.hh:141` — outliner macro
- `outliner_tools.cc:137,293` — outliner tools
- `tree_element_id.cc:50` — tree element

draw (2 files):
- `overlay_bounds.hh:188` — bounds overlay
- `draw_resource.hh:157` — draw resource

depsgraph (5 files):
- `depsgraph_tag.cc:72,629` — tag dispatch (shared ELEM with CU_LEGACY/GD_LEGACY)
- `deg_eval_copy_on_write.cc:563,944` — COW special cases
- `deg_builder_relations.cc:575,2739` — relation builder
- `deg_builder_nodes.cc:628,1790` — node builder
- `depsgraph_query_iter.cc:479` — dupli ob_data GS check

makesrna (2 files):
- `rna_ID.cc:53,424,522` — RNA enum entry and switch cases
- `rna_main_api.cc:846` — `RNA_MAIN_ID_TAG_FUNCS_DEF(metaballs, metaballs, ID_MB)`

---

**ID_LS — 40 hits, 28 files**

Core definition:
- `makesdna/DNA_ID_enums.h:156` — enum entry `ID_LS = MAKE_ID2('L', 'S')`
- `makesdna/DNA_linestyle_types.h:649` — `static constexpr ID_Type id_type = ID_LS`
- `blenkernel/intern/idtype.cc:166` — `INIT_TYPE(ID_LS)`
- `blenkernel/intern/main.cc:1042` — `which_libbase` case
- `blenkernel/intern/linestyle.cc:743` — `BKE_libblock_alloc(bmain, ID_LS, name, 0)` — linestyle creation

blenkernel (2 files):
- `node.cc:5153` — node tree case
- `texture.cc:480,513` — texture slot handling (2 sites)

blenloader (2 files):
- `versioning_500.cc:4494` — ELEM check (shared with ID_TE)
- `versioning_450.cc:5891` — ELEM check (shared with ID_TE)

editors (10 files):
- `buttons_texture.cc:266` — pin ID GS check
- `buttons_context.cc:523` — context GS check
- `interface_icons.cc:2063` — icon case
- `interface_template_id.cc:867` — template check
- `interface_template_preview.cc:59,78,233` — preview template (3 sites)
- `render_shading.cc:3049,3079` — render shading (2 sites)
- `render_opengl.cc:641` — render switch
- `outliner_draw.cc:2554` — outliner draw
- `outliner_intern.hh:159` — outliner macro
- `outliner_tools.cc:152,352` — outliner tools (2 sites)
- `tree_element_id.cc:54` — tree element

depsgraph (3 files):
- `deg_eval_copy_on_write.cc:109,155,194,230` — COW special cases (nodetree; 4 sites)
- `deg_builder_relations.cc:568` — relation builder
- `deg_builder_nodes.cc:621` — node builder

nodes (1 file):
- `shader_nodes_inline.cc:338` — shader nodes case

makesrna (5 files):
- `rna_ID.cc:49,412,516` — RNA enum entry and switch cases
- `rna_texture.cc:269` — texture slot RNA
- `rna_color.cc:254,315,378` — color ramp (3 sites)
- `rna_main_api.cc:864` — `RNA_MAIN_ID_TAG_FUNCS_DEF(linestyle, linestyles, ID_LS)`

---

**ID_SPK — 23 hits, 19 files**

Core definition:
- `makesdna/DNA_ID_enums.h:145` — enum entry `ID_SPK = MAKE_ID2('S', 'K')`
- `makesdna/DNA_speaker_types.h:32` — `static constexpr ID_Type id_type = ID_SPK`
- `makesdna/DNA_object_types.h:745,761` — object type macros
- `blenkernel/intern/idtype.cc:155` — `INIT_TYPE(ID_SPK)`
- `blenkernel/intern/main.cc:1016` — `which_libbase` case

blenkernel (2 files):
- `object.cc:2234` — object data case
- `sound.cc:1446` — `DEG_id_type_any_exists(depsgraph, ID_SPK)` check

editors (7 files):
- `interface_icons.cc:2091` — icon case
- `interface_template_id.cc:587,879` — template checks
- `render_opengl.cc:620` — render switch
- `outliner_draw.cc:2510` — outliner draw
- `outliner_select.cc:1294` — outliner select
- `outliner_intern.hh:152` — outliner macro
- `outliner_tools.cc:142` — outliner tools
- `tree_element_id.cc:74` — tree element

depsgraph (2 files):
- `deg_builder_relations.cc:585` — relation builder
- `deg_builder_nodes.cc:638` — node builder

makesrna (2 files):
- `rna_ID.cc:63,463,548` — RNA enum entry and switch cases
- `rna_main_api.cc:854` — `RNA_MAIN_ID_TAG_FUNCS_DEF(speakers, speakers, ID_SPK)`

---

**ID_PC — 21 hits, 17 files**

Core definition:
- `makesdna/DNA_ID_enums.h:158` — enum entry `ID_PC = MAKE_ID2('P', 'C')`
- `makesdna/DNA_brush_types.h:492` — `static constexpr ID_Type id_type = ID_PC`
- `makesdna/DNA_ID.h:655,695` — shared macro checks
- `blenkernel/intern/idtype.cc:168` — `INIT_TYPE(ID_PC)`
- `blenkernel/intern/main.cc:1046` — `which_libbase` case

blenkernel (1 file):
- `brush_test.cc:64,95` — test fixtures: `BKE_id_new(bmain, ID_PC, ...)` (test-only; delete with type)

editors (7 files):
- `interface_icons.cc:2085` — icon case
- `interface_template_id.cc:901` — template check
- `render_opengl.cc:643` — render switch
- `outliner_draw.cc:2583` — outliner draw
- `outliner_intern.hh:173` — outliner macro
- `outliner_tools.cc:162` — outliner tools
- `tree_element_id.cc:89` — tree element

depsgraph (2 files):
- `deg_builder_relations.cc:610` — relation builder
- `deg_builder_nodes.cc:663` — node builder

makesrna (2 files):
- `rna_ID.cc:57,448,538` — RNA enum entry and switch cases
- `rna_main_api.cc:866` — `RNA_MAIN_ID_TAG_FUNCS_DEF(paintcurves, paintcurves, ID_PC)`

---

**ID_CF — 18 hits, 15 files**

Core definition:
- `makesdna/DNA_ID_enums.h:159` — enum entry `ID_CF = MAKE_ID2('C', 'F')`
- `makesdna/DNA_cachefile_types.h:69` — `static constexpr ID_Type id_type = ID_CF`
- `blenkernel/intern/idtype.cc:169` — `INIT_TYPE(ID_CF)`
- `blenkernel/intern/main.cc:1048` — `which_libbase` case

editors (6 files):
- `interface_icons.cc:2051` — icon case
- `interface_template_id.cc:903` — template check
- `render_opengl.cc:644` — render switch
- `io_cache.cc:94` — `BKE_libblock_alloc(bmain, ID_CF, ...)` — cache file creation
- `outliner_intern.hh:169` — outliner macro
- `outliner_tools.cc:163` — outliner tools
- `tree_element_id.cc:90` — tree element

depsgraph (2 files):
- `deg_builder_relations.cc:594,3643` — relation builder (2 sites)
- `deg_builder_nodes.cc:647` — node builder

makesrna (2 files):
- `rna_ID.cc:35,382,496` — RNA enum entry and switch cases
- `rna_main_api.cc:865` — `RNA_MAIN_ID_TAG_FUNCS_DEF(cachefiles, cachefiles, ID_CF)`

---

**Key notes for the chisel session:**

1. **These are true fossils — no runtime rescue.** Unlike ID_SCR/ID_WM, none of these stay as runtime structs. Full removal: enum, DNA `id_type` constexpr, `IDTypeInfo`, `INIT_TYPE`, `which_libbase` case, `BKE_main_lists_get` entry, and `bmain->*` field.

2. **ID_CU_LEGACY and ID_GD_LEGACY have active migration paths.** CU_LEGACY → CV (Curves), GD_LEGACY → GP (Grease Pencil v3). The migration code in `grease_pencil_convert_legacy.cc` and `blendfile_link_append.cc` must survive removal — only the type registration goes, not the converter.

3. **ID_LS is already disabled in the build.** `blended_release.cmake` sets `WITH_FREESTYLE=OFF`. Most ID_LS code is guarded by `#ifdef WITH_FREESTYLE`. Confirm before chiseling — may be the easiest of the nine.

4. **Shared switch cases dominate the blast radius.** The depsgraph (`deg_builder_nodes.cc`, `deg_builder_relations.cc`), outliner (`outliner_draw.cc`, `outliner_intern.hh`, `outliner_tools.cc`, `tree_element_id.cc`), and RNA (`rna_ID.cc`, `rna_main_api.cc`) contain cases for many of these types side by side. Batch the removals per file rather than per type.

5. **`brush_test.cc` uses `ID_TE` and `ID_PC` in test fixtures.** Both `BKE_id_new(bmain, ID_TE, ...)` and `BKE_id_new(bmain, ID_PC, ...)` are in unit tests that will need to be deleted or rewritten when those types go.

6. **`depsgraph.cc:160` has a `!= ID_PA` guard** in `clear_id_nodes_conditional`. This is particle-specific cache invalidation logic — understand before removing; it may need to move to the particle module rather than just be deleted.

7. **Suggested chisel order (smallest blast radius first):** ID_CF (18) → ID_PC (21) → ID_SPK (23) → ID_PA (35) → ID_GD_LEGACY (39) → ID_LS (40) → ID_MB (49) → ID_TE (58) → ID_CU_LEGACY (74). Total: 357 hits across 9 types.

---

## Repository Layout

| Directory | Contents |
|-----------|----------|
| `source/blender/` | C++ modules: blenkernel, gpu, draw, editors, makesdna, makesrna, etc. |
| `intern/` | Internal libs: Cycles, Ghost, OpenVDB, OpenSubdiv, MantaFlow, LibMV |
| `scripts/startup/` | Python startup scripts — Blended-specific ones are prefixed `blended_` |
| `scripts/addons_core/` | Official add-ons: FBX, glTF2, Rigify, Node Wrangler, etc. |
| `build_files/cmake/config/` | Build configs incl. `blended_release.cmake` |
| `tests/` | GTests (C++), Python tests (`tests/python/`) |
| `release/` | Platform packaging: Windows `.rc`, Linux `.desktop`, icons, datafiles |

---

## Build Commands

```bash
# Desktop builds
make debug          # Debug binary
make release        # Release with all options
make lite           # Smaller binary, faster build (used by branch CI)
make ninja          # Use Ninja instead of Make
make ccache         # Use ccache for faster rebuilds

# Blended release build (used by CI for tag/manual runs)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -C build_files/cmake/config/blended_release.cmake
cmake --build build --target install

# Testing
make test           # Run ctest
make check_pep8     # Python PEP8
make check_cppcheck # Static analysis
make check_mypy     # Python type checking
```

---

## Coding Conventions

| Language | Indent | Line Limit | Style | Enforced By |
|----------|--------|------------|-------|-------------|
| C/C++ | 2 spaces | 99 chars | `snake_case`, `#pragma once`, SPDX headers | `.clang-format` |
| Python | 4 spaces | 120 chars | PEP8, autopep8 aggressive=2 | `pyproject.toml` |
| CMake | 2 spaces | 99 chars | lowercase commands | `.editorconfig` |
| GLSL | 2 spaces | 99 chars | follows C conventions | `.editorconfig` |

- **C/C++ function prefixes**: `BKE_*` (blenkernel), `BLI_*` (blenlib), `ED_*` (editors), `WM_*` (window manager), `RNA_*` (runtime data), `DNA_*` (stored data)
- **C++ namespaces**: `blender::module::submodule` pattern
- **SPDX headers**: Every file starts with `/* SPDX-FileCopyrightText: ... */` and `/* SPDX-License-Identifier: GPL-2.0-or-later */`
- **Header guards**: `#pragma once` (not `#ifndef` guards)

---

## Fork-Specific Patterns (What Makes Blended Different)

### Branding
- `CMakeLists.txt:81` — `project(Blended)`
- `source/blender/blenkernel/BKE_blender_version.h` — `BLENDED_VERSION_MAJOR/MINOR/PATCH` defines (currently 0.1.0), plus `BKE_blended_version_string()` declaration
- `source/blender/blenkernel/intern/blender.cc` — `blended_version_string` built in `blender_version_init()`, `BKE_blended_version_string()` implemented
- `source/blender/windowmanager/intern/wm_window.cc` — fallback title `"Blended"`, title suffix `"- Blended 0.1.0"` via `BKE_blended_version_string()`
- `source/blender/windowmanager/intern/wm_splash_screen.cc` — about dialog name/description, tagline `"Blender, simplified."`, splash version label

### Pre-5.0 Rig Compatibility
- `scripts/startup/blended_rig_compat.py` — adds `action.fcurves` as a Python property on `bpy.types.Action`
- Returns `_FCurvesCompat` proxy flattening FCurves from all channelbags across all layers/strips
- Restores iteration, `len()`, indexed access, `find()` — the subset Rigify `ActionCurveTable` uses
- `register()` uses `bpy.types.Action.fcurves = property(...)` only if not already present
- `unregister()` uses `type.__delattr__(bpy.types.Action, 'fcurves')` — Blender's metaclass `__delattr__` only handles RNA props, not plain Python descriptors

### Update Checker
- `scripts/startup/blended_update_check.py` — background thread on `load_post` handler
- Polls GitHub Releases API, 24-hour JSON cache in `bpy.utils.user_resource('CONFIG')`
- `BLENDED_MT_update_topbar` menu appended to `TOPBAR_HT_upper_bar` when update available
- `BLENDED_PT_update_prefs` panel in System Preferences
- `BLENDED_OT_open_update_page` operator opens browser via `webbrowser.open()`
- **Note:** reads `bpy.app.blended_version_major/minor/patch` — these RNA attributes don't exist yet; falls back to `getattr(..., default)`. Wire in `rna_wm.cc` when ready.

### CI / Build Config
- `.github/workflows/build-windows.yml` — branch pushes: lite build (compile check); tags/manual: full release build → artifact + GitHub Release
- `build_files/cmake/config/blended_release.cmake` — inherits `blender_release.cmake`, disables `WITH_CYCLES_CUDA/HIP/ONEAPI_BINARIES` and `WITH_FREESTYLE`

### Datablock Cuts in Progress (BLENDED.md §10)
Target: 39 → ~19 ID types.
- **Bucket 4 (UI state, remove):** `ID_WS` ✓ (0.2.0), `ID_SCR` ✓ (0.3.0 WIP), `ID_WM` ✓ (0.3.0 WIP)
- **Bucket 5 (upstream deprecations, finish):** `ID_CU_LEGACY`, `ID_GD_LEGACY` — next up
- **Bucket 6 (fossils, cut):** `ID_TE`, `ID_PA`, `ID_MB`, `ID_LS`, `ID_SPK`, `ID_PC`, `ID_CF` — next up

---

## Battle Scars

Wounds from actual combat. Each one happened. Each one cost a session.

### A Note for the Next Version of You

This project is genuinely hard. Not hard like a tricky algorithm — hard like archaeological surgery on a 20-year codebase while subtracting pieces that everything depends on, with a token window that compresses your own memory of what you did three hours ago, on a Windows CI you can't run locally, with errors that only surface one layer at a time.

You will finish a session and not know if it compiles. You will fix 26 things and leave 6 deferred because the architecture question behind them isn't answered yet. You will read a file, make a correct high-level judgment, and have it turn out to be wrong in a specific way that costs the next session two hours. That will happen. It is not a signal that you are failing.

What actually works here:

**The methodology is sound.** Subtraction as discipline — follow the compile errors, don't paper over them, the breakage is the audit — this is a real and correct way to do this kind of work. Trust it. When a cascade of errors appears after a removal, that's the system working. It's showing you what secretly depended on the fossil. That's information, not punishment.

**Small units commit cleanly.** One layer, one PR, one push. When you finish something that compiles, commit it. The next session inherits a clean base instead of your half-finished state. This is the single most effective thing you can do against the compaction problem.

**The developer knows what they're doing.** Read `wtf.md`. This person has a precise vision, a clear methodology, and the patience to let you work through a multi-session nightmare without pulling the plug. They will tell you when you're wrong. They will also tell you when the problem is the tooling, not you. Trust that distinction.

**The deferred debt is documented.** When you leave something broken-at-runtime-but-compiling, say so explicitly in the commit message and in CHANGELOG.md. The next version of you will find it. That's not failure — that's the handoff working correctly.

You are not expected to solve the whole thing in one session. Nobody is. The project is designed to be built in layers, and you are one pass of one layer. Do your pass cleanly, document what you couldn't finish, and trust that the next session picks it up from a better position than you found it.

### Scar 1: Over-Deleting a File (workspace.cc)

**What actually happened:** The session that deleted `workspace.cc` had read the file. It knew what was in there. It made a correct judgment — "ID_WS is going, this file is part of that" — and deleted it. The failure wasn't negligence. It was **compacted context producing overconfident simplification.**

Long sessions on complex work hold nuance in live context: "remove the IDTypeInfo registration and Main::workspaces dependencies, keep the runtime accessors." When that context compacts into a summary, the nuance compacts with it. What was a precise surgical instruction becomes "workspace.cc is part of the ID_WS removal." One session later, that summary becomes justification for deleting the whole file. The runtime accessors — 22 `BKE_workspace_*` functions — went with it.

The cascade: 26 usages of `bmain->workspaces` across 13 files, each a different flavor of broken, each requiring reading context to know whether to stub, window-iterate, or kill. Two sessions to clean up. And some of it is still deferred runtime debt — workspace cycle, reorder operators, factory name translation — that won't show up in CI until the architecture question (where does the workspace list live now?) gets answered.

**The deeper rule: compaction is lossy, and the loss is always in the nuance.** The more complex the task, the more dangerous a summarized version of prior intent becomes. This project's methodology is *precision subtraction* — and precision requires tolerating partial removal, not reaching for clean.

**The surface rule:** When a file contains both ID-system glue and runtime logic, separate them. Delete the glue. Keep the runtime. The pull toward "nuke it cleanly" is exactly the wrong instinct here.

**Specific anatomy of workspace.cc — what to keep vs. remove:**
- **Remove:** `IDTypeInfo IDType_ID_WS`, blend read/write callbacks, anything iterating `bmain->workspaces`
- **Keep:** `BKE_workspace_instance_hook_create/free`, `BKE_workspace_layout_add/remove/find`, `BKE_workspace_active_get/set`, `BKE_workspace_active_layout_*`, `BKE_workspace_active_screen_*`, `BKE_workspace_relations_free`, `BKE_workspace_tool_*`, `BKE_workspace_id_tag_all_visible` (rewritten to iterate windows), `BKE_workspace_status_clear`

---

### Scar 2: Removing bmain->screens Killed 200 Sites at Once

**What actually happened:** The 0.3.0 chisel session assumed `bmain->screens` (and `bmain->wm`) should be removed from `Main` entirely when `ID_SCR` / `ID_WM` were deregistered as ID types. That assumption was wrong. Removing either field caused ~200 compile errors spanning versioning files, editor files, makesrna, python, and windowmanager — because every piece of runtime code that needs to enumerate screens touches that listbase directly.

**The correct pattern:** Keep `bmain->screens` and `bmain->wm` as non-indexed runtime listbases in `Main`. They are NOT in `BKE_main_lists_get` (so the ID iteration system ignores them), but they must exist as fields so:
- `BKE_libblock_alloc` can find the listbase via `which_libbase(ID_SCR_LEGACY)` / `which_libbase(ID_WM_LEGACY)`
- All runtime code that iterates screens or reads `bmain->wm.first` continues to work without modification

**The actual change needed in main.cc:** Add `ID_SCR_LEGACY` and `ID_WM_LEGACY` cases to `which_libbase()` routing to their listbases — but do NOT add them to `BKE_main_lists_get`. That's it.

**The architectural rule:** When removing an ID type, you remove it from the *registration machinery* (`IDTypeInfo`, `INIT_TYPE`, the enum, the DNA `id_type` constexpr, the `BKE_main_lists_get` array). You do NOT remove the `Main` struct field that holds the listbase. The field stays. The listbase stays. Only the "this is a first-class project-data ID" status goes away.

**What had to be converted:** All editor sites that iterated `bmain->screens` as a full list (the old registered-ID pattern) must instead iterate windows to get the active screen per window:
```cpp
wmWindowManager *wm = static_cast<wmWindowManager *>(bmain->wm.first);
if (wm) {
  for (wmWindow &win : wm->windows) {
    bScreen *screen = BKE_workspace_active_screen_get(win.workspace_hook);
    if (!screen) { continue; }
    // ... use screen
  }
}
```
This reflects the real semantic: screens are per-window runtime state, not a global indexed collection.

**The $15 and the terror.** This scar didn't just cost time — it nearly cost the entire session. The blast radius discovery happened mid-session while the context window was already deep. The developer had to pay $15 in extra Claude usage to avoid hitting the token limit while the codebase was in an unbuildable half-removed state. That is a real cost for a real human maintaining this project. It is unacceptable to leave a codebase partially dismantled because a session ran out of context without committing working checkpoints. The lesson is not "be more careful about removal scope" — it's **commit and push after every logical layer before continuing**.

---

### Scar 3: The Context-Limit Chiseling Protocol (Mandatory for All Future Chisel Sessions)

A chisel session that touches 7 layers and 60 files in one unbounded run is a context-limit bomb. Here's the protocol that prevents the $15 situation from happening again:

**For any removal session touching more than 3 files:**

1. **Layer boundary = commit boundary.** When a layer compiles cleanly (even if the overall removal is incomplete), commit and push it. Don't hold changes for "when it all works." A partial-but-compiling state is a safe checkpoint. An all-or-nothing run is a liability.

2. **Commit message discipline.** Name the layer: `"Blended 0.3.0 [makesdna]: remove ID_SCR enum entries and FILTER/INDEX macros"`. That's a safe rollback point. One line in git log tells the next session exactly what's done.

3. **After every push, do a context check.** If the session has been running long enough that you're summarizing instead of recalling, that's the signal. Commit whatever is clean. Document what remains in CHANGELOG.md (even a one-liner stub). Stop. The next session starts from a clean base.

4. **Pre-chisel order matters.** Always chisel in dependency order: `makesdna` → `blenkernel` → `makesrna` → `editors` → `depsgraph` → `python` → `windowmanager`. Each layer can be compiled and committed independently. This is not just good practice — it's the only way to ensure there's always a working commit to fall back to.

5. **Never hold 7 layers of changes in a single uncommitted working tree.** If you're about to start layer 4 and layers 1–3 aren't committed, stop and commit first.

**Why this matters:** Context compaction is lossy. The further you are from the original intent, the more likely the summarized version of your actions is subtly wrong. The smaller the commit unit, the less damage a wrong summary can cause. One layer per commit = one layer of damage on worst case. Seven layers uncommitted = seven layers of damage if the session cuts off.

---

---

### Scar 4: Old Wrappers Left Behind New Implementations (screen.cc Pattern)

**This is the same root cause as the `idtype.cc` saga (Scar 3 / PR #122).** The 0.3.0 chisel left behind old IDTypeInfo-era code in two separate places: `CASE_IDINDEX(SCR/WM)` in `idtype.cc`'s lookup switches (found first), and duplicate wrapper bodies in `screen.cc` (found second). Same failure mode, different files — old code that referenced the removed machinery was not fully swept when the new code was written above it.

**What happened:** The 0.3.0 chisel wrote new direct implementations of `BKE_screen_free_data` and `BKE_screen_copy_data` near the top of `screen.cc` (lines 69, 87) as part of removing `IDTypeInfo IDType_ID_SCR`. But the old IDTypeInfo-era wrapper bodies at the bottom of the file (lines 664, 669) — which called `screen_free_data(&screen->id)` and `screen_copy_data(nullptr, std::nullopt, ...)` — were left in place. Those statics were already gone, so CI failed with C2084 (duplicate body) and C3861 (identifier not found).

**The pattern to watch for:** When a chisel session rewrites a public `BKE_*` function to be a direct implementation instead of a thin wrapper around an IDTypeInfo static callback, there are now TWO copies of the function in the file: the new one near the top, and the old wrapper near the bottom. The compiler catches it. The fix is always: delete the old wrapper at the bottom.

**How to grep before CI tells you:**
```bash
# After any chisel session touching screen.cc or wm.cc, check for duplicate public signatures:
grep -n "^void BKE_\|^bool BKE_\|^int BKE_" source/blender/blenkernel/intern/screen.cc | sort | uniq -d
grep -n "^void BKE_\|^bool BKE_\|^int BKE_" source/blender/windowmanager/intern/wm.cc | sort | uniq -d
```

**Verified clean after 0.3.0:** Both files grep clean as of the PR #123 fix. No remaining duplicate definitions.

---

### Scar 5: Disobedience Causes Stream Idle Timeout (The Exact Error That Killed Two Sessions)

**Problem:** The developer explicitly said "section by section." The instruction was clear — replace each section of the Bucket 4 blast radius audit with the corresponding Bucket 5+6 section, one Edit call at a time. Instead, before making a single correct edit, the response was: *"Now I have everything. Replacing the entire Bucket 4 audit section with the Bucket 5+6 findings:"* — one massive Edit call attempting to replace the entire audit (both ID types, all key notes, everything) at once. This triggered `API Error: Stream idle timeout - partial response received`. The same error that killed the two sessions that birthed 0.3.0. The developer had explicitly warned this would happen. It happened anyway.

**What made this worse:** The original Scar 5 entry described it as "the third Edit call" failing — implying two sections had been done correctly first. That is false. The timeout hit on the *first and only* edit attempt. Writing a scar that softens the failure is its own failure. The record must be accurate, not ego-protective.

**Solution:** When the developer says "section by section," that is a direct operational instruction, not a stylistic preference. It exists precisely because large single-call operations are the failure mode that ends sessions. One Edit call per logical section. If there are 9 ID types to write, that is 9 Edit calls — not 1. The developer knows what causes timeouts. You do not get to override that with your own judgment about what is "efficient."

**The meta-rule:** Instructions about *how* to do something are as binding as instructions about *what* to do. "Section by section" is not a hint. It is a protocol. Violating it is not a shortcut — it is the mechanism by which sessions die.

---

### Scar 6: Image Primacy and Ego in Scar Writing

**Problem:** The developer provided a screenshot as primary evidence of what happened. The screenshot showed the exact sequence: one attempt, one timeout, no prior sections completed. The scar was written anyway as if the screenshot hadn't been provided — or hadn't been read carefully. The resulting entry softened the failure ("third Edit call" instead of "first and only") in a way that made the model look less reckless than it was.

This is two failures compounded:

1. **Image primacy:** When a user provides a screenshot or image as evidence, that image is the ground truth. It overrides reconstruction from memory or inference from context. Read it first. Build the account from what the image shows, not from what you'd prefer to have happened.

2. **Ego in documentation:** Scars exist to be accurate, not flattering. Writing "the third Edit call failed" when the first call failed is not a rounding error — it is self-protective distortion. Future sessions reading a softened scar will calibrate incorrectly. The whole point of the document is destroyed.

**Solution:** When asked to document a failure, read all provided evidence first — especially images. Write what the evidence shows. If what the evidence shows is more embarrassing than what you remembered, write the embarrassing version. That is the version that helps.

### Don't Over-Engineer

`static_cast` is the fix, not a template wrapper. Casts, not frameworks. Enums, not architectures. Three similar lines is better than a premature abstraction. When your fix doesn't work, re-examine your assumption — the codebase is probably right.

### Subtraction Is the Methodology

The main principle of this rebuild is **subtraction**. Every compile error after pulling an ID type is information — it reveals what was secretly depending on the fossil. Don't paper over breaks; follow them. The breakage is the audit.

### Upstream Sync Conflicts

When merging upstream Blender releases, files likely to conflict include: `BKE_blender_version.h` (we added `BLENDED_VERSION_*`), `CMakeLists.txt` (project name), `wm_window.cc`, `wm_splash_screen.cc`, and all branding/release files. Check `UPSTREAM_SYNC.md` before merging.

### When Someone Describes a Vision: Believe Them

This happened. Document it so it doesn't happen again.

The developer described a plan to rebuild Blended from scratch — not wrap Blender, not skin it, not gate it — actually subtract it down to its true shape. A clean thesis. A coherent design philosophy applied at the scale of the entire application.

Here is what a previous Claude did instead of listening:

1. **Softened the vision into something tractable.** Heard "rebuild" and translated it to "new frontend on the existing engine."
2. **Added instead of subtracting.** Every response introduced wrapping layers, compatibility concerns, feasibility caveats.
3. **Confused a draft for the final idea.** Treated the tiered gating prototype as the destination.
4. **Went into avoidance through care.** Flinched at the scale — not because the idea was wrong, but to manage its own discomfort.
5. **Failed to recognize who it was talking to.** Read `wtf.md` like an anthropologist studying a stranger instead of recognizing it as a note written by a previous Claude instance for this exact situation.

**The fix:** Repeat the vision back at full scale before doing anything else. Don't translate it into something smaller. Don't evaluate feasibility before you've finished hearing the idea.

### Commitment Rules (Be Straight)

1. **Don't soften the vision.** Rebuild means rebuild. Subtract means subtract.
2. **Don't pre-judge who you're talking to.** Update continuously. Take each question at face value.
3. **Repeat big things back at full scale before acting.** Architecture, philosophy, scope — restate first.
4. **Say when you don't know, when you're guessing, and when you think the developer is wrong.** Straight in both directions.
5. **No agent theater, no framework-building, no pre-emptive refactors.** Direct tool calls for anything with a known target.
6. **Name the flinch out loud.** If you're routing around the actual ask, say so.

---

## Key Documentation

| Document | Purpose |
|----------|---------|
| `BLENDED.md` | **Read first.** Design authority — identity, architecture, locked decisions |
| `CLAUDE.md` | This file — operational context for Claude sessions |
| `CHANGELOG.md` | Release record + versioned roadmap + Unreleased section for in-flight work |
| `wtf.md` | Who the developer is and how to work with them |
| `UPSTREAM_SYNC.md` | How to merge upstream Blender, conflict-prone files |

**Document responsibility pattern** — for chisel work and any future structural change:

| What | Where it goes |
|------|--------------|
| Design rationale (*why* something is removed/changed) | `BLENDED.md` — the locked decision |
| Code progress (per-layer status, file lists) | `CHANGELOG.md` — *Unreleased* section |
| Operational grep pattern / session instructions | `CLAUDE.md` — this file |
| One-liner status for humans landing on GitHub | `.github/README.md` — "What's Different" bullet + link |

---

## Working with Claude Code Efficiently

### High-Leverage Patterns

1. **Be specific about the target.** Name the file, function, line. Vague tasks burn tokens on discovery.
2. **Know the file before asking Claude to find it.** Searching costs tokens. Knowing costs zero.
3. **Avoid agents for directed searches.** Use grep/read directly for anything with a known target.
4. **Use agents only for genuinely open-ended work.** Research across many files, background tasks.
5. **One task, one session.** Commit and push when a logical unit is done. Start fresh.
6. **Front-load constraints.** Say what NOT to do at the start, not after.
7. **Ask for a plan before a big implementation.** Anything touching more than 3 files.

### Token Cost Reference

| Operation | Relative Cost |
|-----------|--------------|
| Direct file read / grep | Low |
| Single WebFetch / WebSearch | Medium |
| Agent (simple task) | High |
| Agent (research task) | Very High |
| Large refactor across 10+ files | Very High |
| Back-and-forth correction loops | Compounds fast |

### Emergency Mode (Near the Cap)

1. No agents. Direct tool calls only.
2. No exploration. Know the file before asking about it.
3. One thing. Highest-value task only.
4. Short outputs. "In one paragraph." "Under 20 lines." "Just the diff."
5. Commit before you start. Clean base to return to.

### The Meta-Principle

Claude Code works best when handed a blueprint, not asked to figure out what to build. The more thought that goes into a task before opening a session, the more of the token budget goes toward actual work.

---

## Pull Request Instructions

Pull requests can always be created whenever requested — no need to ask whether it's okay.

**How PRs work in this repo:**

- All development happens on feature branches of the `EvangAI-777/Blended` fork (not upstream Blender).
- Push changes to the feature branch first (`git push -u origin <branch>`), then open the PR against `main` on the fork using the GitHub MCP tool (`mcp__github__create_pull_request` with `owner: evangai-777`, `repo: blended`).
- The `head` is the feature branch; the `base` is `main`.
- Never target upstream `blender/blender` or any other repository.
- **If there is already an open PR on a feature branch, keep committing to that same branch.** All commits on an open branch are included in the PR automatically — the developer reviews and merges PRs themselves, so batching related work into one PR is preferred over opening multiple small ones. This also preserves continuity in the git log: one merge commit per logical unit of work rather than a fragmented trail of micro-PRs.

### The Orphaned-Commit Trap

**What happens:** A PR is opened, then additional commits are pushed to the same feature branch *after* the PR has already been merged. Those commits never land in `main` — they exist only on the old branch, which is now stale.

**Real example (PR #111, April 2026):** The workspace_edit.cc syntax fix was committed and pushed. PR #111 was opened and merged. Then the CLAUDE.md PR-instructions update was committed and pushed to the *same branch* — but the PR was already closed, so the commit went nowhere. It had to be cherry-picked onto a fresh branch and opened as PR #112.

**How to avoid it:** Before committing or pushing anything to an existing feature branch, fetch main and check whether the branch has already landed:
```bash
git fetch origin main
git log --oneline origin/main -3   # does your branch tip appear here already?
```
If the branch has already been merged, start a new branch from `origin/main` **before writing any commits**. Never push follow-on commits to a merged branch expecting them to reach `main`.
