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

Blended has **three layers**, **two substrates**, and **one animation engine that powers every content type**. All three structures must be honored.

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

### One engine, every content type [LOCKED]

The depsgraph, keyframes, timeline, and F-curves aren't limited to 2D and 3D animation — they power everything Blended authors. **Every property in every section is keyframeable; every time-based mode supports frame-by-frame authoring where it makes sense.** Audio mix levels, video clip transforms, storyboard camera-on-still moves, game asset transforms, design element opacity, compositor node values — all keyframeable, using the same engine that drives 2D and 3D animation.

This is the structural cash-out of the slogan (§1). "Explicit focus on the craft of animation" is only honest if animation is *ambient* — not a mode you enter, a capability the tool has wherever you are. Static work is just animation with one frame. Motion graphics is Design with keyframes. Animated game UI is Game with keyframes. No special cases; everything falls out of the same principle.

**Timeline-format sections — Editing, Mixing, Scoring, and the Storyboard editor — support a toggle between keyframe mode and each format's traditional editing UI** (clips on tracks for video, faders and automation lanes for audio mixing, notes/sequencer for scoring, panel manipulation for storyboard). The underlying representation is always keyframes; the *view* conforms to industry conventions on request. A video editor never has to leave clip-and-track thinking if they don't want to; a mixer never has to leave their faders. But keyframes are one toggle away, on the same data, for anyone who wants the animation-native view.

Per-section specifics of what "keyframe" and "frame-by-frame" mean (and where the toggle lives, and what the traditional UI looks like) live in §12 as each stage's spec gets written.

**UI restraint is load-bearing.** Every section surfacing keyframe controls bluntly would bring Blender's knob-clutter back. Affordances stay subtle: right-click any property → insert keyframe; timelines unfurl when used, hidden when not; the toggle to traditional editing is a single button, not a screen-splitting mode change. Breadth doesn't become clutter.

---

## 3. Historical Context & Why This Matters [LOCKED]

Blender carries three stacked, unreconciled visions. Blended blends them.

| Era | Blender | Vision |
|---|---|---|
| 1993–1998 | NeoGeo in-house tool | **A** — opinionated studio tool |
| 1998–2002 | NaN / Foundation rescue | **B** — "free 3D software for everyone," access mission, identity undefined |
| 2003–2018 | Open Movies era | **A-redux** — dogfood-driven, studio-shaped |
| 2019–today | Industry DCC era | **C** — industry-range creative suite (2D, 3D, games, compositing, editing, audio) |

### What Blended takes from each, and what it refuses

**Blended = A's discipline + B's accessibility + C's scope, with C's *pathology* explicitly rejected.**

- **From A** — opinionated discipline. Animation is the shaping discipline; the core is designed around it; decisions are made, not deferred.
- **From B** — accessibility. The tool is openable by a first-time user. Launcher tells the truth about what Blended is. "Free 2D and 3D software tools" in the slogan means access is real, not rhetorical.
- **From C — *scope, not pathology*.** Blended commits to the breadth of professional creative work C has always been reaching for: 2D animation, 3D animation, game asset creation, compositing, editing, audio, illustration (when in scope). The pipeline sections in §11 (Storyboarding, 2D Animation, 3D Animation, Game Design, Editing, Compositing, Audio) *are* C's scope, honestly inhabited.

### What C's *pathology* means, specifically

C's failure mode — the one that made Blender feel pathological — is not having broad scope. It's:

1. **Feature-parity arms race** — adding features because Maya / Houdini / ZBrush / Premiere / Nuke have them, not because the feature serves the core.
2. **Core deformation by industry quirks** — letting FBX, USD, or other interchange-format idiosyncrasies shape the internal data model instead of translating at the boundary (see §5).
3. **Co-equal unreconciled identities** — "we are also a game engine and a video editor and a CAD tool" without a shaping discipline to resolve the conflicts.

Blended embraces C's *breadth* while rejecting those three failure modes. The substrate split (§2) and the Creative/Post pipeline grouping (§11) are what make breadth coherent instead of pathological. Animation is still the shaping discipline; breadth lives around it, not in place of it.

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

Default rule: **include if it fits 2D or 3D creative work AND doesn't deform the animation-shaped core.** Scope is broad (C's breadth, embraced per §3). Exclude only for genuine identity/runtime conflicts or C-pathology tells.

### In scope
- 2D and 3D animation (the shaping discipline)
- 3D modeling, sculpting, rigging, rendering
- Game asset creation, rigging for game engines, animation export to Unity/Unreal/Godot, sprite sheets, 2D game art, LOD/baking workflows
- Still-frame renders, product visualization, architectural visualization
- Grease Pencil (2D) with optional bridging into 3D scenes
- Compositing, video editing, audio mixing (as post-animation finishers)
- Geometry nodes (procedural rigs, procedural motion, procedural environments)
- USD / glTF / FBX / OBJ / Alembic at the boundary
- Illustration, concept art, and graphic design workflows (§11 Design section)

### Out of scope [REJECTED]

Rejected for specific reasons, not because "Vision C." Scope is embraced; only pathology is rejected (see §3).

- **Runtime game engine inside Blended** (BGE/UPBGE style), gameplay scripting, interactive runtime as an output medium. *Reason: identity/runtime conflict — Blended authors content; it doesn't host runtime gameplay.*
- **Feature-parity arms race.** Adding features because Maya/Houdini/ZBrush/Premiere/Nuke/Photoshop/Illustrator have them, not because they serve the core. *Reason: C-pathology tell — see §3.* Scope expansion on its own isn't the pathology; parity-reasoning is.
- **Core deformation by industry format quirks.** Interchange formats translate at the §5 boundary; they do not reshape internal data models. *Reason: C-pathology tell.*
- **"Co-equal at identity level"** — animation-and-generalist as twin unreconciled identities. *Reason: this is Blender's current pathology; the substrate split (§2) and Creative/Post grouping (§11) resolve it.*
- **Legacy/fossil formats** that exist because no one dared delete them (see §5 Groups 2–3, §10 Bucket 6). *Reason: accumulated cruft with no active user base.*

---

## 8. Biases & Guardrails [GUARDRAIL]

Named failure modes. If you catch yourself or a collaborator doing these, push back.

1. **Exclusion bias.** Once the broader substrate frame was agreed, the first instinct was still to *cut* things (Framing A energy). Wrong under our agreed frame. Correct default: **include if it fits the scope and doesn't deform the core.** Claude flagged this bias in-session; it applies to humans too.
2. **Co-equal identities trap.** "Both X and Y are co-equal identities" is indistinguishable in practice from "we haven't decided." (This is distinct from Vision C in §3 — Vision C is about *scope and pathology*, not identity co-equality.) If two ideas sound co-equal, find the structural layer where one is scope and the other is shaping discipline, or find a substrate split (§2). Do not leave identity co-equality unresolved.
3. **Implicit priority drift.** Blender's disease is that priorities exist but are never stated. If Blended starts accumulating features justified by implicit priorities, stop and re-read §1.
4. **UI-first temptation.** UI before data model = the failure mode of the previous attempt. Don't.
5. **Format-as-shaper.** If an industry format's quirks are deforming the core data model, the boundary is leaking. Fix the boundary, not the core.
6. **"Feature parity" language.** If someone argues for a feature on the grounds that Maya/Houdini/ZBrush/Premiere/Nuke has it, that is a **Vision C *pathology*** tell — not a scope argument. C's scope (industry breadth) is embraced per §3; C's pathology (feature-parity arms race) is not. Evaluate every feature on whether it serves the core and the shaping discipline, not on who else has it.

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

The user-facing structure of Blended **is** the production pipeline. Not a menu of workspaces. Not a grid of tool doors. A visible flow through creative work, grouped into **Creative (authoring)** and **Post (finishing)**, with animation as the shaping discipline of both the 2D and 3D authoring sections.

### The pipeline (canonical order)

> **Creative:** Storyboarding → 2D Animation → 3D Animation → Game → Design
>
> **Post:** Editing → Compositing → Audio

Animation is the **shaping discipline** of the Creative section (2D Animation and 3D Animation each have their own internal `Animate` mode as the apex within their section). Post stages serve the animation output.

### Launcher structure [LOCKED in principle, pixel-level UI still OPEN]

**The whole launcher is a single vertical scrollable view.** Not a grid of tiles. Not a modal door that expands. Everything visible at once; scroll to see what's below the fold.

- **"Blending?"** — the prompt at the very top. Single word, question mark intended. Reads both as status ("what's blending?") and invitation ("want to blend?"). Sets the playful register.
- **Below that, each pipeline section as a bold heading with its mode buttons listed underneath it**, stacked vertically into two groups:

```
"Blending?"

╌╌ CREATIVE ╌╌

Storyboarding
  [Board]

2D Animation
  [Animate] [Frame-by-Frame] [Paint]

3D Animation
  [Sculpt] [Model] [Rig] [Environment] [VFX] [Animate]

Game
  [Asset] [Level] [Bake] [Export]        (industry-expandable)

Design
  [Graphic] [Illustration] [Concept]     (industry-expandable)

╌╌ POST ╌╌

Editing
  [Video] [Sketch] [Polish]              (industry-expandable)

Compositing
  [Composite]                            (industry-expandable)

Audio
  [Mix] [Score]
```

- `╌╌ CREATIVE ╌╌` and `╌╌ POST ╌╌` may be literal visual separators or just implicit spacing — UI detail.
- Most headings ship with a placeholder/starter mode set; industry-specific modes can be added as adjacent buttons without changing the section structure.
- **Scan, scroll, click.** That's the whole interaction. No drill-downs, no hover states, no modal expansions.

### Principles [LOCKED]

- **Freely jumpable.** Enter any section / mode at any time, regardless of project state. Pipeline is the organizing metaphor, not a forced sequence.
- **Directly enterable.** A sculptor who just wants to sculpt clicks `Sculpt` under 3D Animation and is there. No pipeline-walk required.
- **Project state reflected back.** The launcher shows *where the current project has content* — sections with data look different from empty ones. First-time new user sees all sections neutral/inviting.
- **Pipeline is the default view, not mandatory.** Re-enterable from any workspace via a global hotkey or equivalent.
- **Each mode button opens a filtered view of the same project.** The `.blended` file is one file; the mode controls what's visible. (This is why §10's UI-state datablock removals are load-bearing — once SCR/WM/WS are not project data, the launcher becomes the canonical workspace system.)

### Project-level settings [LOCKED]

Cross-cutting animation engine settings (framerate, renderer, output, color management, motion blur) apply to both 2D Animation and 3D Animation. They live in a **project-level config** accessible globally from the launcher (gear icon or a "Project" row at the top of the scroll), **not inside any one section.** Changing framerate affects both 2D and 3D content — one engine, one config, two content types. §2's "one engine, two lenses" made literal.

### What this settles

- **§6 Launcher** is superseded by this section. Preserved there for history.
- **§2 substrate split** made structural: 2D Animation and 3D Animation as parallel Creative sections, each with animation as the shaping discipline.
- **§10 datablock ownership** — each datablock type belongs to a section/mode, which makes "filtered view per workspace" land cleanly.
- **Assets / Environments / VFX** are **not** standalone pipeline stages — they're modes *inside* 3D Animation, because they're tools used while doing 3D animation, not discrete production phases.
- **Game is its own Creative section**, not a mode under 3D Animation — game asset creation has genuinely different quality metrics (polycount, UV efficiency, engine-export formats, LODs) that don't belong under "3D Animation."
- **Design is its own Creative section**, covering graphic design, illustration, and concept art workflows. These are C-scope (embraced per §3) — Blended is "2D and 3D software tools," plural. The guardrail is against feature-parity pathology (no "we must match every Illustrator feature"), not against the scope itself. Modes stay industry-expandable and pipeline-adjacent when they can be (concept art feeds animation, UI art feeds Game, etc.).

### Still open

- **Editing vs Compositing merge.** Industry keeps them separate; we probably should too. Parked.
- **Creative / Post visual separators.** Actual UI dividers vs implicit spacing — pixel detail for later.
- **Design mode expansion.** Initial modes (`Graphic`, `Illustration`, `Concept`) are starters; community- and industry-expandable. Typography / Layout / Print etc. can be added as separate modes when the use case is real, without changing the heading.

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

### 12.2 2D Animation [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The stage where storyboard panels become frame-accurate animation. Under the principle that *storyboard IS the first pass of animation,* no data is converted or imported — the same scenes, same Grease Pencil strokes, same timeline, same audio from §12.1 are all still present. What changes is the UI lens and what the user does with the existing data.

**Headline insight — F-curve interpolation driving 2D.** Blender's F-curve system is world-class for 3D (bezier handles, ease-in / ease-out / bounce / constant / linear control). Grease Pencil strokes have point positions; points can be keyframed. Therefore **Blender's F-curve interpolation can drive the tween between Grease Pencil drawings** with full artistic control. Draw five key poses, get 24fps animation with proper easing. This is the thing the rest of the 2D-animation industry (ToonBoom, Moho, CACANi) either doesn't do or does badly. In Blended it's the default workflow, using code Blender is already best at.

**Shared engine, shared settings.** Everything that runs 3D animation — frame rate (24/30/60+), renderer, output resolution, color management, motion blur, all animation engine and properties settings — applies to 2D identically. The engine doesn't care whether it's interpolating object transforms or Grease Pencil stroke points; it's just animating properties. These cross-cutting settings live in the **project-level config** (§11, "Project-level settings"), accessed globally from the launcher — not inside any single Creative section. A 2D animator adjusts framerate, renderer, output using the same UI a 3D animator uses. No duplicated settings, no 2D-specific config — §2's "one engine, two lenses" made literal.

**Mode buttons under the tile:**

- **`Animate`** — keyframe-driven. Draw key poses, adjust F-curves between keys, get smooth tweening with full easing control. The headline mode.
- **`Frame-by-Frame`** — hand-drawn, no interpolation. For animators who want every frame drawn by hand (the slight wobble, the hand-drawn feel — an artistic choice). Onion skinning and drawing tools, no F-curve interpolation on strokes.
- **`Paint`** — coloring pass. Bucket fill, color palettes, shading layers. Different enough from linework (different tools, different headspace) to earn its own mode.

Community-extensible per §2 — ship with these three; `Ink` / `Tween` / `Rig` or specialized modes can grow from community workflows.

**Screen layout (Animate mode):**

- **Canvas** — dominant, left ~2/3. Current frame. Draw here.
- **Frame strip** — bottom of the canvas area. Every tick is a drawable frame, not just key panels. Scroll / jump / scrub.
- **Timeline with F-curve access** — below the frame strip. Shows keyframes on stroke data and the interpolation curves between them. Tweak a curve → tween reshapes.
- **Layer panel** — right side. Character / background / effects / annotation layers. Visibility, lock, onion-skin per layer.
- **Onion skinning** — on by default; previous and next frames ghosted behind the current.
- **Richer drawing toolbar** — pen, eraser, bucket, shape tools, line smoothing, masking.
- **"Play Animatic" button** — same button as in Storyboarding, same naming, because it's the same project and the same animatic, just further along.

**Data model — same primitives as Storyboarding. No new datablocks:**

| 2D Animation concept | Existing datablock |
|---|---|
| Animation engine settings (framerate, renderer, output) | `ID_SCE` properties (shared with §12.X Animate stage) |
| Frames | Keyframes on scene timeline |
| Drawings | `ID_GP` (Grease Pencil) strokes |
| Stroke interpolation | F-curves on stroke point data (existing animation system) |
| Layers | Grease Pencil layers on the `ID_GP` datablock |
| Audio tracks | `ID_SO` datablocks on the scene |

The entire stage is a UI lens over data Storyboarding already created.

**Transition from Storyboarding.** Click **2D Animation** in the launcher. Same scenes, same panels, same audio. UI changes:

- Panel strip → frame strip (every frame drawable)
- Onion skinning on
- Layer panel appears
- Timeline granular (per-frame, not per-panel)
- Drawing toolbar richer
- F-curve access on stroke data

Storyboard panels become literal keyframes; in-betweens are either drawn by hand (`Frame-by-Frame`) or generated by F-curve interpolation (`Animate`) with curves the user shapes.

**Transition to next stage:**

- **2D-final project** → Composite / Edit. Final mix, scene transitions applied, export.
- **3D project using 2D as pre-vis** → Assets → Environments → Animate. 2D strokes become hidden reference layers in 3D scenes, toggle-able from the outliner.

**Still open within 2D Animation:**

- Whether `Frame-by-Frame` is its own mode or just `Animate` with interpolation disabled. Current call: its own mode because the artistic philosophy differs meaningfully.
- How the F-curve editor integrates visually — dedicated panel, embedded in timeline, or popover. UI detail.
- Rig as a fourth mode vs living under Assets. Parked.

---

## 13. Document Conventions

- Tag new sections with [LOCKED] / [OPEN] / [REJECTED] / [GUARDRAIL].
- When reopening a LOCKED section, state why and what new evidence changed the call.
- When a decision closes an OPEN question, move it out and summarize in the relevant section.
- Keep this doc tight. It is a working agreement, not a history. History lives in git.
