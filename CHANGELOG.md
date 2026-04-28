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
| 0.2.x | Datablock audit — UI-state removals (ID_WS, then ID_SCR, ID_WM) |
| 0.3.x | Datablock audit — fossil removals (Buckets 5 + 6) |
| 0.4.x | Datablock audit — complete (Bucket 3 fold-downs; 39 → ~19 ID types) |
| 0.5.x | Evaluation model — depsgraph audit |
| 0.6.x | App lenses — launcher as canonical workspace system |
| 0.7.x | File format — `.blended` is the project, import/export is the boundary |
| 1.0.0 | Foundation complete; basic pipeline navigation working |

---

## Unreleased — 0.3.0

Next target: `ID_SCR` and `ID_WM` — Bucket 4 completions. Same chisel pattern as ID_WS, larger blast radius. Each gets its own point release.

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

### 0.2.x — Datablock audit: UI-state

**0.2.0 — ID_WS removed** *(see above)*

**0.2.x — ID_SCR and ID_WM**

`bScreen` and `WindowManager` are also Bucket 4 (BLENDED.md §10) — per-user,
per-machine state that currently leaks into `.blend` files. Removal scope is
larger than ID_WS (both are more deeply wired into the window system), so they
get their own point releases after 0.2.0.

---

### 0.3.x — Datablock audit: fossils

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

Each fossil follows the same chisel pattern as ID_WS. The breakage is the audit.

---

### 0.4.x — Datablock audit: complete

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

### 0.5.x — Evaluation model

Depsgraph audit under Blended's scope. Current depsgraph has had three rewrites
and carries assumptions from all three eras. Questions:

- What evaluation paths actually exist in Blended's scope?
- Which depsgraph node types survive the datablock cuts above?
- What's kept because it's right vs kept because removing it is hard?

The ID type cuts in 0.2–0.4 will have already removed dead branches from the
depsgraph. This milestone cleans up what remains.

---

### 0.6.x — App lenses (launcher)

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

### 0.7.x — File format

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
