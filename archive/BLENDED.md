# BLENDED — Identity & Design Agreements

**Status:** Living document. Working agreements from the rebuild conversation.
**Branch:** `claude/rebuild-blender-simplified-GNIWm`
**Purpose:** So any future session, contributor, or Claude instance can pick up without re-litigating what's already been decided. Read this first before proposing changes to scope, identity, or architecture.

---

## 0. How to read this document

Sections are tagged:

- **[LOCKED]** — agreed and not up for re-debate without explicit reopen.
- **[OPEN]** — still being headhunted. Contribute here.
- **[REJECTED]** — explicitly considered and cut. Don't propose again without new evidence.
- **[GUARDRAIL]** — patterns to watch for. These are the failure modes of this project.

---

## 1. Slogan [LOCKED]

> **Blended provides free 2D and 3D software tools, with an explicit focus on the craft of animation.**

Every word in this sentence is load-bearing. Before weakening any of it, read §2.

- **"provides ... tools"** (plural) — commits to a suite/launcher identity, not a single monolithic app.
- **"2D and 3D software tools"** — scope is broader than animation alone. Legitimate uses include game asset work, product viz, arch viz, modeling for export, sprite art, etc.
- **"explicit focus on the craft of animation"** — the rhetorical move that resolves the Blender pathology. Blender's current mess comes from *implicit, unstated* priorities. Saying the priority out loud is the fix.
- **"craft"** — implies discipline, care, mastery. Not just feature presence.

---

## 2. Identity Structure [LOCKED]

Blended has **three layers** and **two substrates**. Both structures must be honored.

### Three layers (vertical)

1. **Core — Vision A discipline.**
   Animation shapes the core. Non-negotiable core components:
   - Dependency graph (depsgraph)
   - Keyframes
   - Timeline / dope sheet / F-curve editor
   - Armatures
   - Actions / NLA
   Every core design decision is made to serve animation first.

2. **Shell — Vision B accessibility.**
   The launcher and focused workspace doors. A user walks up, picks what they are doing, and gets a focused lens onto the core.

3. **Boundary — Vision B+C absorbed at the edge.**
   Import/export, industry format compatibility, familiar-workflow hotkeys for returning Blender users. **The boundary translates; it does not shape the core.** Currently, industry formats deform Blender's internals. In Blended, the core is sovereign and boundaries are translators.

### Two substrates (horizontal)

Co-equal at the **medium** level, not at the **identity** level. This distinction is what keeps Blended out of the Blender pathology (see §3).

- **3D substrate.** Shaping discipline: animation. Houses the headline launcher doors.
- **2D substrate.** Grease Pencil, annotation, 2D drawing, 2D animation. Its own top-level door. Community-extensible — the 2D toolset is smaller and less opinionated than 3D; letting it grow organically is honest.

Under the hood: **one engine, two lenses.** The depsgraph, keyframes, timeline, F-curves serve both substrates. 2D animation is still animation.

---

## 3. Historical Context & Why This Matters [LOCKED]

Blender carries three stacked, unreconciled visions. Blended explicitly reconciles them.

| Era | Blender | Vision |
|---|---|---|
| 1993–1998 | NeoGeo in-house tool | **A** — opinionated studio tool |
| 1998–2002 | NaN / Foundation rescue | **B** — "free 3D software for everyone," access mission, identity undefined |
| 2003–2018 | Open Movies era | **A-redux** — dogfood-driven, studio-shaped |
| 2019–today | Industry DCC era | **C** — feature parity with Maya/Houdini/ZBrush/Premiere/Nuke |

**Blended = A's discipline + B's accessibility, with C explicitly rejected.**

The substrate split (§2) resolves the secondary tension between "animation tool" (A's origin) and "3D software for everyone" (B's stated mission). Animation is the *shaping discipline*; 2D/3D software is the *scope*.

---

## 4. Methodology [LOCKED]

### Foundation-first order

UI is a projection of a data model. If the model is wrong, UI either papers over it (complexity) or fights it (workarounds). Both are the Blender disease. So the build order is:

1. **Project file format** — what a Blended project *is*. [Principles locked, per-format decisions OPEN]
2. **Datablocks** — what primitives live inside a project. [OPEN, next]
3. **Evaluation model** — depsgraph, how primitives change when anything changes. [OPEN]
4. **App lenses** — how a launcher door is technically a filtered view of the model. [OPEN]
5. **UI** — only after 1–4 are honest. [OPEN, later]

Prior attempts started at UI and failed because the underlying model was still Blender's. This is why this time is different.

### Dogfooding

The project's author is currently using Blended for real animation coursework. The previous prototype passed for Blender with their animation professor (professor did not notice). **Animation coursework is Blended's Open-Movies-equivalent.** Every design decision must survive use in actual animation work.

---

## 5. File Format Principle [LOCKED]

> **Everything is inside `.blended` by default. Import/export is an explicit boundary, not a default workflow.**

Currently Blender is schizophrenic: textures can be packed or linked, caches live on disk, image sequences are referenced paths, addons ship as external `.py`. Every one of those is a leak where "the project" escapes onto the filesystem. The leaks are why collaboration, moving files, and versioning all hurt.

Commit to "the `.blended` file *is* the project, period" and most of the format proliferation disappears.

### Format audit buckets [mostly OPEN]

Identified, principles locked, final per-format decisions still being headhunted:

- **Group 1 — Spine (native).** Collapse to one `.blended`. Drop userpref-as-blend, startup-as-blend, filesystem backup rotations — those are state, not documents. [Principle LOCKED, per-item OPEN]
- **Group 2 — 3D interchange.** Keep USD, glTF/GLB, FBX, OBJ, Alembic. Drop or demote PLY, STL, BVH, X3D/VRML, Collada, 3DS. [OPEN]
- **Group 3 — Images.** Keep PNG, JPEG, EXR (multilayer), WebP, TIFF. Drop IRIS (SGI, 1990s), Targa, BMP, DDS, DPX/Cineon, JP2, HDR-radiance, PSD-read. [OPEN]
- **Group 4 — Video/audio.** Defer to ffmpeg as a single I/O boundary. The sin is exposing the full combinatorial codec/container space as first-class UI. [OPEN]
- **Group 5 — Simulation/cache.** Currently a swamp (`.bphys`, `.vdb`, Mantaflow intermediates, point caches, Alembic-as-cache). **Collapse into the project file as datablocks**, or unify into one cache format. Each sim system growing its own cache format is the purest example of "Blender fighting its own file formats." [OPEN]
- **Group 6 — Scripts/presets.** The existence of both external `.py` and text datablocks is the anxiety. Pick one. Asset library being "a folder of `.blend` files" is a workaround for not having a real asset primitive. [OPEN]
- **Group 7 — Fonts.** Leave alone. System-provided via FreeType. [LOCKED]

---

## 6. Launcher Architecture [SUPERSEDED by §11 — preserved for history]

### Structure

- Splash screen (logo, copyright) → launcher.
- Launcher greets with something like **"What would you like to blend today?"**
- **Tier 1:** Substrate choice — 3D or 2D.
- **Tier 2 (within 3D):** Focused doors. Animator is the **headline door** (larger/central); supporting doors sit under it — Model, Sculpt, View, Edit, Render.
- **Tier 2 (within 2D):** Doors are more co-equal (small coherent space, no clear headline). Draw, Animate, Ink, Paint, and whatever the community adds.

### Principles

- **Visual hierarchy tells the truth.** Inside 3D, animation is the shaping discipline, so the Animator door is visibly headline. Inside 2D, no single door dominates.
- **Launcher is a committed router**, not a file-template picker buried in a splash screen.
- **Each door is a filtered view of the shared project model.** Same `.blended` file, radically different UI per door. The Animator literally cannot see sculpt brushes; the Sculptor literally cannot see the NLA editor.
- **"Viewing"** is not a workflow door — it is either the implicit default state of the launcher, or a small "drop into viewport without picking a workflow" affordance.

### What each door serves (supporting roles)

- **Sculpt** — asset prep for animation *or* end-use sculpting. Not a ZBrush replacement. Scope: enough to finish a shot.
- **Model** — geometry creation. Serves animation and also legitimate 3D software use (game asset export, product viz).
- **Video Edit (VSE)** — cross-substrate post tool. Finishes animation into a final cut. Not a Resolve replacement.
- **Compositor** — cross-substrate. Finishes shots.
- **Render** — animation has to output.
- **Grease Pencil** — primary citizen of 2D substrate. Can optionally appear as a bridging object in 3D scenes.

---

## 7. What's In Scope [LOCKED]

Default rule: **include if it fits 2D or 3D software AND doesn't deform the animation-shaped core.** Exclude only for genuine identity/runtime conflicts.

### In scope
- 3D and 2D animation (the core)
- 3D modeling, sculpting, rendering (supporting, but competent as standalone uses)
- Rigging and character animation
- Game asset creation, rigging for game engines, animation export to Unity/Unreal/Godot, sprite sheets, 2D game art
- Still-frame renders, product visualization, architectural visualization
- Grease Pencil (2D) with optional bridging into 3D scenes
- Compositing, video sequence editing (as post-animation finishers)
- Geometry nodes (procedural rigs, procedural motion)
- USD / glTF / FBX / OBJ / Alembic at the boundary

### Out of scope [REJECTED]
- **Runtime game engine inside Blended** (BGE/UPBGE style), gameplay scripting, interactive runtime as an output medium
- **Feature-parity arms race** with Maya/Houdini/ZBrush/Premiere/Nuke (the Vision C trap)
- **Legacy/fossil formats** that exist because no one dared delete them (see §5 Groups 2–3)
- **"Co-equal at identity level"** — animation-and-generalist as twin identities. That is Blender's current pathology. See §3.

---

## 8. Biases & Guardrails [GUARDRAIL]

Named failure modes. If you catch yourself or a collaborator doing these, push back.

1. **Exclusion bias.** Once the broader substrate frame was agreed, the first instinct was still to *cut* things (Framing A energy). Wrong under our agreed frame. Correct default: **include if it fits the scope and doesn't deform the core.** Claude flagged this bias in-session; it applies to humans too.
2. **Co-equal trap (Framing C).** "Both X and Y are co-equal identities" is indistinguishable in practice from "we haven't decided." If two ideas sound co-equal, find the structural layer where one is scope and the other is shaping discipline, or find a substrate split (§2). Do not leave identity co-equality unresolved.
3. **Implicit priority drift.** Blender's disease is that priorities exist but are never stated. If Blended starts accumulating features justified by implicit priorities, stop and re-read §1.
4. **UI-first temptation.** UI before data model = the failure mode of the previous attempt. Don't.
5. **Format-as-shaper.** If an industry format's quirks are deforming the core data model, the boundary is leaking. Fix the boundary, not the core.
6. **"Feature parity" language.** If someone argues for a feature on the grounds that Maya/Houdini/ZBrush has it, that is a Vision C tell. Evaluate on whether it serves the core + scope, not on parity.

---

## 9. Open Questions [OPEN]

Next in the foundation chain:

1. **Datablock audit.** ~40 ID types in Blender. Separate the central, the supporting, and the fossils. Known fossils to verify: Texture (BI legacy), VectorFont (duplicate of Font?), Metaball (1990s), Surface (NURBS legacy), LineStyle (freestyle niche), Speaker, LightProbe (merge into Light?). WindowManager-as-datablock is UI state leaked into project data.
2. **Evaluation model / depsgraph audit.** Current depsgraph has had three rewrites. Audit what it actually needs to do under Blended's scope.
3. **2D-inside-3D bridge.** Grease Pencil's power includes drawing in 3D space, parenting to cameras, 2D characters on 3D sets. Under separate substrates, is the bridge preserved? Proposed: GP datablock is 2D-primary but available as a bridging object type in 3D. Different tool UI, same datablock.
4. **Project-mode flag.** One `.blended` file that can hold 3D-primary, 2D-primary, or mixed projects — or enforce single-mode per file?
5. **Per-format final decisions** in §5 Groups 2–6.
6. **Detailed launcher UI** — deferred until foundation is solid.

---

## 10. Datablock Audit

> **An ID type earns its place by being: (a) a thing the user directly creates, names, and reuses across contexts, (b) serialized as project data, not per-user state, (c) not a property bag that only makes sense inside one parent.**

### Current state

39 ID types in Blender's DNA (`source/blender/makesdna/DNA_ID_enums.h`). Blended target: ~19. Cut in half without losing legitimate scope.

### Bucket 1 — Core keepers [LOCKED]

The 13 non-negotiable ID types:

| ID | Name | Role |
|---|---|---|
| `ID_SCE` | Scene | Top-level project container |
| `ID_OB` | Object | The animatable entity |
| `ID_GR` | Collection | Scene organization |
| `ID_ME` | Mesh | 3D primary geometry |
| `ID_CV` | Curves | Modern curves/hair |
| `ID_AR` | Armature | Rigging |
| `ID_AC` | Action | **Where keyframes live. Animation core.** |
| `ID_NT` | NodeTree | Materials, compositor, geometry nodes |
| `ID_CA` | Camera | |
| `ID_LA` | Light | |
| `ID_MA` | Material | |
| `ID_IM` | Image | |
| `ID_GP` | Grease Pencil | 2D substrate headline |

### Bucket 2 — Supporting, keep [LOCKED in principle, details OPEN]

| ID | Name | Note |
|---|---|---|
| `ID_VO` | Volume | OpenVDB |
| `ID_PT` | PointCloud | Geometry nodes uses it |
| `ID_SO` | Sound | Animation sync + VSE |
| `ID_MC` | MovieClip | Footage for tracking/VSE |
| `ID_TXT` | Text | **Packed project scripts only.** Kill external `.py` workflow (resolves §5 Group 6) |
| `ID_KE` | ShapeKey | OPEN — audit whether it's actually shared across meshes in practice. If not, collapse into geometry. |

### Bucket 3 — Fold down from ID to struct [LOCKED direction]

Property bags pretending to be first-class entities:

| ID | Name | Fold into |
|---|---|---|
| `ID_BR` | Brush | User state + shareable brush packs |
| `ID_PAL` | Palette | Brush property or inline |
| `ID_LT` | Lattice | Modifier, not a datablock |
| `ID_LP` | LightProbe | Merge into `ID_LA` with a type flag |
| `ID_MSK` | Mask | Hang off compositor NodeTree |
| `ID_VF` | VFont | System font reference; FreeType handles the rest |

### Bucket 4 — UI state removals [LOCKED]

**Not project data.** Per-user, per-machine state that currently leaks into `.blend` files and scrambles other people's workspaces.

| ID | Name | Where it belongs |
|---|---|---|
| `ID_SCR` | bScreen | User state |
| `ID_WM` | WindowManager | User state |
| `ID_WS` | WorkSpace | **Replaced by the launcher model (§6).** Workspaces as a datablock go away. |

**Load-bearing for §6:** once Workspace is not project data, the launcher becomes the canonical workspace system and `.blended` files travel cleanly between users.

### Bucket 5 — Finish upstream's already-marked deprecations [LOCKED]

| ID | Name |
|---|---|
| `ID_CU_LEGACY` | Curve — tagged LEGACY by upstream, replaced by `ID_CV` |
| `ID_GD_LEGACY` | Old Grease Pencil — tagged LEGACY, replaced by `ID_GP` |

Blender itself has marked these for replacement. Blended finishes the job.

### Bucket 6 — Fossils [LOCKED cut]

| ID | Name | Why cut |
|---|---|---|
| `ID_TE` | Texture | Blender Internal renderer fossil; residual folds into NodeTree |
| `ID_PA` | ParticleSettings | Replaced by Geometry Nodes (purest example of §5 Group 5 swamp) |
| `ID_MB` | MetaBall | 1990s implicit surfaces; sculpt/remesh covers it |
| `ID_LS` | FreestyleLineStyle | Niche NPR renderer; NPR via shader nodes / Grease Pencil |
| `ID_SPK` | Speaker | 3D positional audio on scene objects; niche. Audio flows through VSE timeline. |
| `ID_PC` | PaintCurve | Niche stroke guide |
| `ID_CF` | CacheFile | External Alembic/USD cache reference — boundary concern, not project data |

### Open tensions [OPEN]

- **`ID_WO` (World).** Keep as reusable environment asset, or fold into Scene properties? Weak vote: keep.
- **`ID_LI` (Library).** Cross-project asset reuse under "everything in `.blended`" needs design. Options: (a) library = directory of `.blended` files with a proper asset primitive inside; (b) true external linking.
- **Brush user state.** Design where brushes live once they're not project data (user prefs + shareable brush packs).
- **`ID_KE` ShapeKey ID-ness.** Survey real projects before collapsing into geometry.

### Tally

**39 → ~19 ID types.** No legitimate scope lost.

### Consequences

1. Removing SCR/WM/WS makes `.blended` files portable between users.
2. Launcher model (§6) becomes the canonical workspace system — no competing datablock.
3. Compounds with §5 file-format simplification: fewer types to serialize means smaller, simpler files.

---

## 11. Pipeline as UX [LOCKED, supersedes §6]

The user-facing structure of Blended **is** the production pipeline. Not a menu of workspaces. Not a grid of tool doors. A visible flow through animation production, with animation as the apex and everything else feeding into or out from it.

### The pipeline (canonical order)

> Storyboard / Annotation → 2D Animation *(pre-vis or final)* → Assets *(Sculpt / Model / Rigs)* → Environments → VFX + Sound → **KEYFRAMES / TIMELINES [apex]** → Compositing / Video Editing

Animation is the apex because every upstream stage feeds into it and every downstream stage serves it. This replaces §6's "Animator is the headline door because we said so" with something structurally honest: the pipeline shows *why* animation sits at the center.

### Launcher structure [LOCKED in principle, pixel-level UI still OPEN]

**The whole launcher is a single vertical scrollable view.** Not a grid of tiles. Not a modal door that expands. Everything visible at once; scroll to see what's below the fold.

- **"Blending?"** — the prompt at the very top. Single word, question mark intended. Reads both as status ("what's blending?") and invitation ("want to blend?"). Sets the playful register.
- **Below that, each pipeline stage as a bold heading with its mode buttons listed underneath it**, stacked vertically down the page in pipeline order:
  - **Storyboarding** — `Board` *(and possibly `Animatic`)*
  - **2D Animation** — `Draw`, `Ink`, `Animate` *(community-extensible per §2)*
  - **Assets** — `Sculpt`, `Model`, `Rigs`
  - **Environments** — environment-specific modes
  - **VFX + Sound** — sim, particle, sound-edit modes
  - **Animate** — timeline / dope sheet / F-curve / NLA entries *(the apex)*
  - **Composite / Edit** — compositor, VSE
- **Scan, scroll, click.** That's the whole interaction. No drill-downs, no hover states, no modal expansions. The apex stage may be styled to draw the eye, but nothing is hidden behind a click.

### Principles [LOCKED]

- **Freely jumpable.** Enter any stage at any time, regardless of project state. Pipeline is the organizing metaphor, not a forced sequence.
- **Directly enterable.** A sculptor who just wants to sculpt clicks Sculpt under Assets and is there. No pipeline-walk required.
- **Project state reflected back.** The launcher shows *where the current project has content* — stages with data look different from empty stages. First-time new user sees all stages neutral/inviting.
- **Pipeline is the default view, not mandatory.** Re-enterable from any workspace via a global hotkey or equivalent.
- **Each stage button opens a filtered view of the same project.** The `.blended` file is one file; the mode controls what's visible. (This is why §10's UI-state datablock removals are load-bearing — once SCR/WM/WS are not project data, the launcher becomes the canonical workspace system.)

### What this settles

- **§6 Launcher** is superseded by this section. Preserved there for history.
- **§2 substrate split** is still true but more nuanced inside the pipeline: 2D is early/optional at the front (Storyboard, 2D Animation); 3D is mid-pipeline (Assets, Environments); Animation and post are substrate-agnostic.
- **§10 datablock ownership** gets natural homes — each datablock type belongs to a stage, which helps "filtered view per workspace" land cleanly.

### Still open

- **VFX placement.** Pre-animation VFX (particles, sims, assets-that-move) vs post-animation VFX (compositing-style). Both exist; may need two tiles or one with sub-differentiation. Parked.
- **Stage granularity.** Assets = Sculpt + Model + Rigs as one stage with three mode buttons (current plan) vs three sibling stages. Current plan: one stage.
- **Project state visualization.** How "where you are in your project" renders on the pipeline — progress indicators, filled tiles, content markers — all UI detail for later.

---

## 12. Pipeline Stages — Detailed Specs

This section fills in concrete specs for each pipeline stage from §11 as they get head-hunted. Stages without entries here are placeholders for future work.

---

### 12.1 Storyboarding [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The earliest form of the project. Rough panels drawn to establish narrative flow. Under the principle *"storyboard IS the first pass of animation,"* storyboard panels literally become the keyframes of the 2D Animation stage and the timing markers of the Animate stage. Nothing is exported, nothing is rebuilt when moving downstream.

**Mode buttons under the tile:** `Board` (and possibly `Animatic` as a playback-focused view; TBD — most likely just `Board`).

**Screen layout (Board mode):**

- **Canvas** — left ~2/3, dominant. The current panel. You draw here with Grease Pencil.
- **Panel strip** — bottom of the canvas area. Thumbnails of panels in order, scrollable. Click to jump; drag to reorder. Playhead visible as a highlighted thumbnail.
- **Timeline** — below the panel strip. Thin band showing time, playhead visible. Sound waveform runs along it if scratch audio exists.
- **Storyboard outliner** — right side (replaces the standard properties/outliner). Tree of narrative scenes (`Scene 1`, `Scene 2`, …), each renameable exactly like entries in the regular outliner. Click a scene to view its panels.
- **Minimal drawing toolbar** — pen, eraser, color, brush size. That's it. No material editor, no modifier stack, no N-panel.
- **Play button** — one big one. Plays the animatic with scratch sound.

**Data model — no new datablocks needed.** Everything reuses existing IDs kept in §10:

| Storyboard concept | Existing datablock |
|---|---|
| Narrative scene (e.g. "Scene 1") | `ID_SCE` |
| Panels within a scene | Keyframes on the scene's timeline |
| Panel drawings | `ID_GP` (Grease Pencil) strokes |
| Scratch dialogue / music | `ID_SO` (Sound) |
| Camera arrows, action paths, notes | Grease Pencil strokes on a non-rendering annotation layer |

This uses `ID_SCE` for what it was always supposed to be good at — a narrative section with its own timeline — instead of the current Blender mess where Scenes mean "render-settings containers" more often than "narrative sections."

**The A/B resolution (panel-as-frame vs panel-as-scene) — both, at different UI layers:**

- **Timeline layer:** each panel is a keyframe of one scene. Animation-native. "Storyboard IS the first pass" cashes out literally here.
- **Outliner layer:** panels are grouped under named narrative scenes shown as a tree. Traditional storyboard structure, narrative-native.

These aren't competing models — they're the same data viewed through two different UI surfaces.

**Transition to the next stage.** When the user moves to **2D Animation**, the same scenes, same Grease Pencil strokes, same timeline, same sound are present. More detail gets added — strokes per frame, in-betweens, cleaner linework. The storyboard never dies. It gets more real. No export. No rebuild. No retiming.

**Still open within Storyboarding:**
- Whether `Animatic` is its own mode button or just Board-with-play-pressed.
- Exact hotkey for "add new panel" (the single most common operation — needs to be one key).
- Whether the panel strip lives at the bottom of the canvas or docked to the right *above* the storyboard outliner. Current call: bottom.

---

## 13. Document Conventions

- Tag new sections with [LOCKED] / [OPEN] / [REJECTED] / [GUARDRAIL].
- When reopening a LOCKED section, state why and what new evidence changed the call.
- When a decision closes an OPEN question, move it out and summarize in the relevant section.
- Keep this doc tight. It is a working agreement, not a history. History lives in git.
