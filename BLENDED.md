# BLENDED — Identity & Design Agreements

**Status:** Living document. Working agreements from the rebuild conversation.
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

**Timeline-format sections — Finalizing, Mixing, Scoring, and the Storyboard editor — support a toggle between keyframe mode and each format's traditional editing UI** (clips on tracks for video, faders and automation lanes for audio mixing, notes/sequencer for scoring, panel manipulation for storyboard). The underlying representation is always keyframes; the *view* conforms to industry conventions on request. A video editor never has to leave clip-and-track thinking if they don't want to; a mixer never has to leave their faders. But keyframes are one toggle away, on the same data, for anyone who wants the animation-native view.

Per-section specifics of what "keyframe" and "frame-by-frame" mean (and where the toggle lives, and what the traditional UI looks like) live in §12 as each section's spec gets written.

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
- **From C — *scope, not pathology*.** Blended commits to the breadth of professional creative work C has always been reaching for: 2D animation, 3D animation, game asset creation, compositing, editing, audio, illustration (when in scope). The pipeline sections in §11 (Storyboarding, 2D Animation, 3D Animation, Game, Design, Finalizing, Compositing, Audio) *are* C's scope, honestly inhabited.

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

## 6. Launcher Architecture [MOVED]

The original v1 launcher spec — tier-based door menu (3D vs 2D substrate → focused workspace doors) — was superseded by §11 Pipeline as UX. Full original content preserved in §13 Notes for historical context.

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

**Code removal in progress** — `makesdna`, `blenkernel`, `makesrna` done; `editors`, `depsgraph`, `python`, `windowmanager` pending. Per-layer file detail in [`CHANGELOG.md`](CHANGELOG.md) — *Unreleased 0.2.0*.

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
> **Post:** Finalizing → Compositing → Audio

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

Finalizing
  [Storyboard] [2D] [3D] [Game] [Design] [Mixed]

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
- **Fast intra-section mode switching.** Once inside a heading, modes switch with a single click or hotkey — no launcher round-trip required. Move between `Sculpt` / `Model` / `Rig` inside 3D Animation as fluidly as you'd switch tools in any mode. Applies universally: any heading's modes are swap-able inline.

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

- **Finalizing vs Compositing scope line.** Finalizing = assembly + delivery preparation (timeline-assembly, QC, export-ready cuts). Compositing = per-frame polish (color grading, effects layers, node-based image work). Current call: clean separation; the distinction is clear enough to keep them as separate sections.
- **Creative / Post visual separators.** Actual UI dividers vs implicit spacing — pixel detail for later.
- **Design mode expansion.** Initial modes (`Graphic`, `Illustration`, `Concept`) are starters; community- and industry-expandable. Typography / Layout / Print etc. can be added as separate modes when the use case is real, without changing the heading.

---

## 12. Pipeline Stages — Detailed Specs

This section fills in concrete specs for each pipeline section from §11 as they get head-hunted. Sections without entries here are placeholders for future work.

**Subsection template.** Every §12.x subsection specs the same five things, in this order: *what it is* (one paragraph), *mode buttons*, *screen layout* (per mode if multiple), *data model* (existing datablocks only per §10), *transitions out* (where users go after this section), and a *frame-by-frame note* (per §2 universal keyframe, stated explicitly even when it's implicit in the medium). This keeps sections comparable and makes the pipeline's consistency visible.

---

### 12.1 Storyboarding [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The earliest form of the project. Rough panels drawn to establish narrative flow. Under the principle *"storyboard IS the first pass of animation,"* storyboard panels literally become the keyframes of the 2D Animation section and the timing markers inside 3D Animation > Animate. Nothing is exported, nothing is rebuilt when moving downstream.

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

**Transition to the next section.** When the user moves to **2D Animation**, the same scenes, same Grease Pencil strokes, same timeline, same sound are present. More detail gets added — strokes per frame, in-betweens, cleaner linework. The storyboard never dies. It gets more real. No export. No rebuild. No retiming.

**Frame-by-frame note.** Storyboarding is inherently frame-by-frame — each panel *is* a discrete frame on the timeline. Keyframes (per §2) are also available for camera-on-still moves, scene transition timing, scratch audio cues. Both modalities coexist by default.

**Still open within Storyboarding:**
- Whether `Animatic` is its own mode button or just Board-with-play-pressed.
- Exact hotkey for "add new panel" (the single most common operation — needs to be one key).
- Whether the panel strip lives at the bottom of the canvas or docked to the right *above* the storyboard outliner. Current call: bottom.

---

### 12.2 2D Animation [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The section where storyboard panels become frame-accurate animation. Under the principle that *storyboard IS the first pass of animation,* no data is converted or imported — the same scenes, same Grease Pencil strokes, same timeline, same audio from §12.1 are all still present. What changes is the UI lens and what the user does with the existing data.

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
- **"Play Animation" button** — renamed from Storyboarding's `Play Animatic` because once you're in 2D Animation with real frames and in-betweens, it's no longer an animatic; it's animation. Same button position, same interaction, same project — name shifts with the substance.

**Data model — same primitives as Storyboarding. No new datablocks:**

| 2D Animation concept | Existing datablock |
|---|---|
| Animation engine settings (framerate, renderer, output) | `ID_SCE` properties (shared via §11 project-level config) |
| Frames | Keyframes on scene timeline |
| Drawings | `ID_GP` (Grease Pencil) strokes |
| Stroke interpolation | F-curves on stroke point data (existing animation system) |
| Layers | Grease Pencil layers on the `ID_GP` datablock |
| Audio tracks | `ID_SO` datablocks on the scene |

The entire section is a UI lens over data Storyboarding already created.

**Transition from Storyboarding.** Click **2D Animation** in the launcher. Same scenes, same panels, same audio. UI changes:

- Panel strip → frame strip (every frame drawable)
- Onion skinning on
- Layer panel appears
- Timeline granular (per-frame, not per-panel)
- Drawing toolbar richer
- F-curve access on stroke data

Storyboard panels become literal keyframes; in-betweens are either drawn by hand (`Frame-by-Frame`) or generated by F-curve interpolation (`Animate`) with curves the user shapes.

**Transition to next section:**

- **2D-final project** → Finalizing → Compositing → Audio. Locked cut, color, mix, export.
- **3D project using 2D as pre-vis** → 3D Animation (Sculpt / Model for assets, Environment for environments, Animate for shot animation). 2D strokes become hidden reference layers in 3D scenes, toggle-able from the outliner.

**Still open within 2D Animation:**

- Whether `Frame-by-Frame` is its own mode or just `Animate` with interpolation disabled. Current call: its own mode because the artistic philosophy differs meaningfully.
- How the F-curve editor integrates visually — dedicated panel, embedded in timeline, or popover. UI detail.
- Rig as a fourth mode vs living under Assets. Parked.

---

### 12.3 3D Animation [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The Creative section where all 3D authoring happens — asset creation, environment composition, simulation, and the animation work that's the apex of the whole pipeline. Collapses what were previously four separate stages (Assets, Environments, VFX+Sound, Animate) into one section with six mode buttons. The honest framing: Sculpt, Model, Rig, Environment, and VFX aren't production phases — they're **tools used while doing 3D animation.** Animate is the convergence point within the section; the other five modes feed into it.

**Mode buttons:** `Sculpt` / `Model` / `Rig` / `Environment` / `VFX` / `Animate`

**Mode switching is fast.** The user doesn't return to the launcher to switch within 3D Animation — modes switch with a single click or hotkey inside the section. Sculpt a form, drop to Model to retopologize, jump to Rig to bind, hit Animate to pose. The section *is* the workflow.

**Animate is the local apex.** The other modes produce the assets and setups that Animate orchestrates over time. Per §2, animation is the shaping discipline of 3D Animation — every design decision inside the section serves the animation work.

---

#### Sculpt

**Primary activity:** high-density mesh manipulation. Form-finding. Clay.

**Screen layout:**
- Dominant viewport — the sculpture
- Minimal toolbar: brush picker, size, strength
- Brush-specific settings on the right panel
- Masks and stroke layers accessible but unobtrusive

**Data model:** `ID_ME` with multires modifier and/or dyntopo. No new datablocks.

**Transitions out:** Model (retopologize), Rig (skin the sculpt directly), Environment (place as set piece).

---

#### Model

**Primary activity:** polygonal modeling. Hard-surface. Clean topology.

**Screen layout:**
- Dominant viewport, edit-mode overlays (vertex/edge/face)
- Modifier stack on the right
- Compact modeling toolbar (extrude, bevel, inset, loop cut, knife)
- N-panel minimized by default

**Data model:** `ID_ME`, `ID_CV` (curves for curve-based modeling), modifiers on the object. No new datablocks.

**Transitions out:** Rig (bind to armature), Environment (place in scene), Game (if asset is game-bound).

---

#### Rig

**Primary activity:** armature construction. Skinning. Constraint and controller setup.

**Screen layout:**
- Viewport with armature overlay and bone orientation visible
- Bone properties + constraint stack on the right
- Weight paint accessible as a quick sub-mode
- IK/FK switching, shape-key hooks, custom controllers

**Data model:** `ID_AR` (Armature), `ID_ME` (deformed geometry), `ID_AC` (Action for rest/bind poses). Existing datablocks only.

**Transitions out:** Animate (use the rig), Model (adjust topology for deformation).

---

#### Environment

**Primary activity:** scene-scale composition. Set dressing. World-building. Lighting. Atmosphere.

**Screen layout:**
- Wide viewport, framed for whole-scene work
- Outliner / collections prominent on the right (scene hierarchy)
- World settings (sky, fog, atmosphere) one click away
- Light placement and matching tools
- Instancing and array tools for repeated elements

**Data model:** `ID_SCE`, `ID_GR` (Collection), `ID_WO` (World), `ID_OB` placements, `ID_LA` (Light). Existing datablocks.

**Transitions out:** Animate (scene is the stage), VFX (add sims into the environment).

---

#### VFX

**Primary activity:** simulations, procedural effects, particles.

**Screen layout:**
- Viewport showing the simulation domain or effect result
- Geometry Nodes editor visible (procedural effects) or simulation settings (baked sims)
- Cache controls (bake, free, re-sim) accessible but unobtrusive
- Force fields and collider setup

**Data model:** `ID_NT` (NodeTree for geo-node effects), `ID_VO` (Volume for smoke/fluids), `ID_PT` (PointCloud), `ID_CV` (hair curves), `ID_ME`. No new datablocks — legacy `ID_PA` particles are cut per §10 Bucket 6; Geometry Nodes is the one path forward.

**Transitions out:** Animate (sim baked into timeline), Environment (drop the effect into a scene).

---

#### Animate [local apex]

**Primary activity:** keyframe work. Posing. Timing. Camera animation. The convergence point where rigs, assets, environments, and effects all get orchestrated over time.

**Screen layout:**
- Viewport with animation overlays (motion paths, trail visualization)
- Timeline, dope sheet, and F-curve editor — dockable or popover
- NLA editor for action mixing
- Rig controllers visible and selectable directly in the viewport
- **"Play Animation" button** — same button position as Storyboarding's `Play Animatic` and 2D Animation's `Play Animation`, same interaction. Name stays `Play Animation` from 2D Animation onward; `Play Animatic` lives only in Storyboarding where it's actually an animatic.

**Data model:** `ID_AC` (Action — where keyframes live), `ID_OB` (transforms), `ID_AR` (rigs), `ID_SCE` (timeline), plus everything from other modes that's being animated.

**Frame-by-frame available** per §2 — hand-pose every frame without F-curve interpolation, for stop-motion-style workflows or when an animator wants full manual control.

**Transitions out:** Finalizing → Compositing → Audio (post pipeline), or Game (if the animation is headed for a game engine export).

---

### Connection to §2 universal keyframe

Every mode's properties are keyframeable, not just Animate. Sculpt brush pressure, Model modifier settings, Rig bone orientations, Environment light intensity, VFX force-field strength — all animatable. Animate is the mode where animation is *the primary activity*; in other modes, animation is the ambient capability from §2.

### Transition out of 3D Animation

- **Animation-final project** → Finalizing → Compositing → Audio.
- **Game project** → Game section for game-specific finishing (baking, LODs, export).
- **Still-frame project** (product viz, arch viz) → Compositing directly, skipping animation if the project is a single frame.

### Still open within 3D Animation

- Whether Sculpt and Model should be one mode with sub-tools (some workflows blur them) or two adjacent modes. Current call: two modes for clarity.
- VFX sub-modes — geo-node effects vs baked simulations likely deserve different UI affordances. TBD.
- How tightly Rig integrates with Animate — one-click "use this rig for animation" handoff or explicit scene setup? UX detail.

---

### 12.4 Game [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The Creative section for producing assets, levels, and exports destined for game engines — Unity, Unreal, Godot, or custom runtimes. Structurally parallel to 3D Animation but with different quality metrics and output targets. Game assets aren't just "3D models"; they're 3D models that must meet specific performance, memory, and runtime constraints (polycount, UV efficiency, lightmap packing, LOD generation, engine-compatible materials). Giving Game its own section honors those constraints instead of treating them as an afterthought inside 3D Animation.

**Mode buttons:** `Asset` / `Level` / `Bake` / `Export` — industry-expandable (e.g. `Retopo`, `Collision`, `Nav` as future separate modes if usage demands).

**Division of labor with 3D Animation.** Modeling and animating happen in 3D Animation. Game-readying happens here. The same `ID_ME` can live in both contexts — the section controls which metrics are surfaced and which tools are available. A mesh modeled in 3D Animation > Model can be opened in Game > Asset and immediately show polycount budgets, UV efficiency, and engine-compatibility warnings. No data migration, just a different lens.

**Division of labor with 2D Animation.** Drawing, frame authoring, and animation happen in 2D Animation. Game-readying (sprite sheet packing, frame-count optimization, UI art export, atlas generation) happens here. The same `ID_GP` strokes or `ID_IM` pixel art can live in both contexts — in 2D Animation you see the frame timeline, drawing tools, and F-curve interpolation; in Game > Asset you see sprite atlas metrics, frame counts, and engine-format warnings. Same data, different lens — no migration.

---

#### Asset

**Primary activity:** building game-ready individual objects (characters, weapons, props, UI art). Game-specific quality metrics surfaced prominently.

**Screen layout:**
- Dominant viewport with edit-mode overlays
- **Polycount HUD** — triangle count, vertex count, budget (user-set or preset-driven)
- **UV efficiency display** — wasted UV space highlighted
- Material slot count + engine-compatibility flags
- Compact modeling toolbar (same tools as 3D Animation > Model, different metrics surfaced)

**Data model:** `ID_ME`, `ID_MA`, `ID_IM`. Existing datablocks. Game-specific concerns (polycount budgets, LOD levels, collision flags) exposed as properties on the object, not new datablock types.

**Transitions out:** Level, Bake, 3D Animation > Rig (rig for game animation), 3D Animation > Animate (animate then come back to export).

---

#### Level

**Primary activity:** composing game levels. Placing assets. Setting up collision, occlusion, nav, spawn points.

**Screen layout:**
- Wide viewport (whole-level framing)
- Outliner with game-specific properties (collision flag, nav-mesh flag, spawn type)
- Lightmap UV overview
- Level streaming / chunk visualization where relevant

**Data model:** `ID_SCE`, `ID_GR`, `ID_OB` (with game-specific properties), `ID_LA`. Same base as Environment in 3D Animation; different concerns surfaced.

**Transitions out:** Bake (lightmaps, nav mesh), Export.

---

#### Bake

**Primary activity:** the optimization pass. Normal maps high-to-low, lightmaps, ambient occlusion, LOD generation, texture atlas compilation.

**Screen layout:**
- Viewport preview (toggle between high-poly source and low-poly target)
- Baking queue — multi-asset, batched
- Settings per bake type (resolution, samples, padding, margin)
- Progress / cache status

**Data model:** `ID_IM` (baked textures land in image datablocks), `ID_ME` (source and target meshes), modifiers on objects. No new datablocks.

**Transitions out:** Asset (iterate if bake reveals issues), Export.

---

#### Export

**Primary activity:** handoff to the game engine. Format selection with engine-specific presets. Validation before export.

**Screen layout:**
- Export preset picker — Unity / Unreal / Godot / Custom
- Per-preset settings (coordinate system, units, animation format, material conversion rules)
- Validation checklist (polycount over budget, UV overlaps, missing materials, unsupported features flagged)
- Destination path + batch export options

**Data model:** existing datablocks serialized to `.gltf` / `.fbx` / `.usd` / etc. per §5 format boundaries. Export translates at the boundary; does not deform the internal data model.

**Transitions out:** back to the game engine itself — Game's primary consumer. Also Finalizing / Compositing / Audio if producing a trailer or cinematic from game content.

---

### Connection to §2 universal keyframe

Sprite transforms, UI opacity, material parameters, particle settings — all keyframeable in Game. Animated game UI, animated sprites, procedurally varying game assets fall out of the same engine. **Frame-by-frame** available for traditional sprite animation (sprite sheets, frame cycles) — though keyframed interpolation is almost always more efficient.

### 2.5D — the 2D↔3D cross-cycle

Games and media that mix 2D and 3D — 2D characters in 3D environments, 3D models rendered with 2D-style shading, hand-drawn animation composited onto 3D scenes, parallax-scrolled sprites in 3D space — are first-class in Blended. The author works in **both** 2D Animation and 3D Animation freely; assets flow between them without conversion or export; Game (and Finalizing / Compositing) consume the mixed output without distinction.

This isn't a special mode or section — it's what falls out of §2's "one engine, every content type." The depsgraph doesn't know whether it's evaluating a Grease Pencil stroke, a polygon mesh, or a textured sprite; they're all just keyframeable properties. 2.5D is the natural consequence of using both substrates in the same project.

**Examples in scope:**
- Hollow Knight-style 2D characters on 3D parallax environments
- Cuphead-style hand-drawn animation composited onto 3D scenes
- Motion graphics with Grease Pencil annotations layered on 3D objects
- Mobile / indie games with 3D models rendered as 2D sprites at runtime
- Cutout / paper-doll animation rigs in 3D space

This is a sweet spot for Blended that the rest of the industry currently solves with jury-rigged pipelines (Spine + Unity, ToonBoom + Maya, Photoshop + Blender + custom exporters). Blended keeps it all in one project, one file, one engine.

### Transitions out of Game

- **Primary:** export to game engine (Unity / Unreal / Godot / custom runtime).
- **Secondary:** Finalizing / Compositing / Audio if producing a trailer, cinematic, or marketing material from game content.
- **Back-cycle:** 3D Animation > Rig / Animate for 3D game-bound characters; 2D Animation > Animate / Frame-by-Frame / Paint for 2D game sprites, animations, and UI art. The animation *craft* happens in the appropriate Animation section; game-readying happens here.

### Still open within Game

- Whether `Retopo` deserves its own mode or stays as a tool inside Asset. Current call: tool inside Asset.
- `Collision` / `Nav` as dedicated modes vs properties on objects within Level. Current call: properties within Level; promote to modes if usage demands.
- Export preset granularity — how many presets ship by default, how user-extensible. TBD.

---

### 12.5 Design [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The Creative section for graphic design, illustration, and concept art — the 2D visual-design work that isn't specifically animation or games. Posters, logos, editorial illustrations, book covers, character designs, environment concepts, motion graphics source material, promotional art. Per §3, Design is part of C-scope embraced — industry breadth without the pathology of feature-parity chasing.

**Mode buttons:** `Graphic` / `Illustration` / `Concept` — industry-expandable (e.g., `Typography`, `Layout`, `Print`, `UI`, `Paint` if workflow demands promote them to standalone modes).

**Design's dual role.** Unlike most Creative sections, Design work often serves *other sections* as input: concept art feeds 3D Animation > Sculpt/Model; illustration feeds 2D Animation > Animate as reference or keyframe art; game UI art feeds Game > Asset; motion graphics feeds Finalizing or Compositing. But Design work can also stand alone — a printed poster, a published illustration, a branded logo doesn't need a downstream. Both modes of use are first-class.

---

#### Graphic

**Primary activity:** graphic design — layouts, logos, typography, marketing materials, UI mockups. Vector-based, with raster elements as needed.

**Screen layout:**
- **Artboard canvas** — defined dimensions (A4, US Letter, 1920×1080, custom), not an infinite 3D space
- Vector toolbar — pen, shape, text, line, gradient
- Alignment and snapping tools prominent
- Color palette panel — swatches, color-theory helpers
- Layer panel (composition layers, not animation layers)
- Grid / ruler / guides overlay

**Data model:** `ID_CV` (curves for vector graphics and text), `ID_IM` (raster images), `ID_GP` (Grease Pencil for hand-drawn elements), `ID_NT` (effect node trees). No new datablocks.

**Transitions out:** Finalizing (motion graphics), Compositing (design elements as comp layers), standalone export (print, web), cross-feed to Game > Asset (UI mockups → game UI).

---

#### Illustration

**Primary activity:** digital painting and illustration — editorial work, book covers, character portraits, finished rendered art.

**Screen layout:**
- Large canvas, customizable size
- Drawing toolbar — pen, brush, eraser, bucket, smudge, blur
- **Brush presets library** — textures, chalk, watercolor, ink, pencil, oil (industry-expandable)
- Color wheel + swatches
- Layer panel with blend modes
- Reference image viewer (drag images in as reference layers)

**Data model:** `ID_GP` (Grease Pencil for vector-ish illustration), `ID_IM` (raster painting, including pixel art), `ID_CV` (curves where needed). No new datablocks.

**Transitions out:** 2D Animation (promote illustration to animated sequence), 3D Animation (illustration as texture or concept reference), standalone export, cross-feed to Graphic (illustration placed inside a poster layout).

---

#### Concept

**Primary activity:** concept art — rough, iterative, exploratory visual development. Character concepts, environment concepts, mood boards, style frames. Speed over polish.

**Screen layout:**
- Canvas optimized for fast sketching (not pixel-perfect)
- **Multi-panel / thumbnail view** — iterate many variants at once, compare side-by-side
- Rough brush presets (loose, gestural — not detail brushes)
- Quick color-blocking tools
- Mood board panel — reference imagery drag-drop for inspiration
- Thumbnail batch export for review

**Data model:** same as Illustration (`ID_GP`, `ID_IM`), plus `ID_GR` (Collections) for organizing concept iterations.

**Transitions out:** 3D Animation > Sculpt / Model (concept → 3D asset), 2D Animation > Animate (concept → animated character or scene), Illustration (promote selected concept into finished art), Game > Asset (concept → game asset).

---

### Connection to §2 universal keyframe

**Motion graphics emerges for free here.** Every property in every mode is keyframeable: stroke position, color, opacity, path deformation, layout element transforms, typography tracking, page positioning — all animatable. Animated logos, kinetic typography, morphing illustrations, scroll-synced design elements fall out of the same engine, not as special cases.

**Frame-by-frame** available for animated illustration (hand-drawn-feel sequences where the artist wants explicit control over every frame) and for fast-cut design animation (discrete design states in sequence).

### Transitions out of Design

- **Standalone output:** print, web, digital publication — export at §5 boundary.
- **Forward into pipeline:** Finalizing (motion graphics clips), Compositing (design layers in comp trees), Audio (title-sequence sync, lyric videos).
- **Cross-feed into other Creative sections:** 2D Animation (illustration → animation), 3D Animation (concept → 3D asset), Game (UI / sprite art → game asset).
- **Back-cycle:** from any of the above when iteration reveals design changes are needed.

### Still open within Design

- Whether `Typography`, `Layout`, `Print`, `UI`, `Paint` earn standalone modes over time or stay as tool modes within `Graphic` / `Illustration`. Ship with three; expand when usage demands.
- Raster painting vs vector illustration inside Illustration mode — unified workspace (like Krita / Procreate) or split modes. Current call: unified.
- Export preset system for common design formats (PDF, PNG, SVG, PSD-compat). Overlaps with §5; TBD.

---

### 12.6 Finalizing [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The first Post section. Where Creative outputs become finished deliverables. *Authoring* happens in Creative; *finalizing* happens here. The name is deliberate — "Editing" is ambiguous (editing happens in every Creative section when you adjust your own work), but "Finalizing" names the specific activity that lives here: preparing content for delivery, not authoring it.

**Mode buttons:** `Storyboard` / `2D` / `3D` / `Game` / `Design` / `Mixed`

Five modes mirror the five Creative headings (Storyboarding, 2D Animation, 3D Animation, Game, Design) — each tuned to the QC, assembly, and export concerns of that content type. A sixth `Mixed` mode handles blended-content projects and externally-imported footage (live-action, drone, screen recordings, stock video).

**Division of labor with Creative sections.** Creative authors; Finalizing delivers. Same `.blended` file, same scenes, same timeline — different lens. The Finalizing lens surfaces delivery-relevant info (QC warnings, export presets, cut-lock status); the Creative lens surfaces creation-relevant tools. Switching between them is fast intra-section + cross-section with no data migration.

---

#### Storyboard

**Primary activity:** finalize storyboard as a standalone deliverable — pitch decks, director reviews, client previews, production-ready animatic sheets.

**Screen layout:**
- Large-panel review view (not the authoring canvas)
- Panel-timing display + scene grouping
- Review annotations layer (director / client notes, dated)
- Export preset picker — PDF pitch deck, animatic video, printed storyboard sheet
- Review-status per panel (approved / revising / blocked)

**Data model:** `ID_SCE`, `ID_GP`, `ID_SO`. Same datablocks as §12.1. No new datablocks.

**Transitions out:** back to Storyboarding > Board (iterate on notes), forward to 2D Animation / 3D Animation (begin detailed work), forward to Mixed Finalizing (if storyboard is one element in a larger piece).

---

#### 2D

**Primary activity:** assemble 2D animation shots into a final sequence. Frame-rate QC, style-consistency check, locked cut.

**Screen layout:**
- Shot-list panel on the left (all §12.2 sequences)
- Assembly timeline (sequence-level, not frame-level)
- Per-shot QC — frame count, frame-rate match, style-consistency warnings (line weight, color palette drift)
- Locked-cut commit / revert
- Export preset picker

**Data model:** `ID_SCE`, `ID_GP`, `ID_AC`, `ID_SO`. Existing datablocks.

**Transitions out:** back to 2D Animation (fix shots), forward to Compositing, forward to Audio, forward to Mixed if project blends.

---

#### 3D

**Primary activity:** assemble 3D animation shots into a final sequence. Per-shot render QC, camera continuity, lighting match.

**Screen layout:**
- Shot-list panel
- Assembly timeline
- Per-shot QC — render status (complete / pending / failed), resolution check, camera metadata (lens, focal length), lighting-match across shots
- Render queue visibility
- Locked-cut commit
- Export preset picker

**Data model:** `ID_SCE`, `ID_OB`, `ID_CA`, `ID_LA`, `ID_AC`, `ID_IM`. Existing datablocks.

**Transitions out:** back to 3D Animation > Animate (fix shots), forward to Compositing, forward to Audio, forward to Mixed.

---

#### Game

**Primary activity:** assemble game trailer, cinematic, or promotional sequence. Integrate in-engine capture with authored content.

**Screen layout:**
- Shot list including game-engine capture clips + Blended-authored shots
- Brand-asset panel (logos, title cards, end cards)
- Timeline for trailer / cinematic sequence
- Export preset picker for marketing deliverables (16:9 master, social crops, vertical, etc.)

**Data model:** `ID_SCE`, `ID_OB`, `ID_MC` (MovieClip for imported game-engine captures). Existing datablocks; external footage enters via §5 import boundary.

**Transitions out:** standalone marketing export, forward to Compositing (polish), forward to Audio (mix), back to Game section for asset changes.

---

#### Design

**Primary activity:** finalize design sequence for delivery. Motion-graphics assembly, animated-design sequencing, design-iteration final version.

**Screen layout:**
- Sequence panel (motion-graphics beats)
- Assembly timeline
- Per-shot QC — resolution, color-space, export-format checks
- Iteration selector (compare v1 / v2 / v3 variants side-by-side)
- Export preset picker for design-delivery formats (animated GIF, MP4, WebM, Lottie)

**Data model:** `ID_SCE`, `ID_GP`, `ID_CV`, `ID_IM`, `ID_NT`. Existing datablocks.

**Transitions out:** standalone export (direct delivery), forward to Compositing (per-frame polish), forward to Mixed (if design is one element of a larger piece), back to Design section.

---

#### Mixed

**Primary activity:** traditional NLE workspace for blended-content projects and externally-imported footage. The industry-familiar video-editing workflow.

**Screen layout:**
- Multi-track timeline (video tracks + audio preview tracks)
- Transition library — cuts, dissolves, fades, wipes, custom
- **Clip-aware info panel** — surfaces relevant metadata per clip type: 3D render info, 2D frame-rate, imported video codec, etc.
- Basic color-grade preview (full grading is Compositing)
- Audio mix preview (full mixing is Audio)
- Import panel for external footage — live-action, drone, screen recordings, stock video (through §5 boundary)
- Export preset picker — industry-standard delivery formats (MP4 H.264, ProRes, DNxHD, WebM, etc.)

**Data model:** `ID_SCE`, `ID_MC` (MovieClip for external video imports — §10 Bucket 2 keeper), `ID_IM`, `ID_SO`, `ID_GP`. Existing datablocks only. External footage imported through §5's video/audio format boundary.

**Transitions out:** forward to Compositing (per-frame polish), forward to Audio (final mix), standalone export.

---

### Connection to §2 universal keyframe

Every Finalizing mode's properties are keyframeable — transition timing, color-grade preview intensity, audio-level previews, shot durations, title animations, clip opacity. "Editing" is largely about *timing*, and timing is keyframes; this is why §2's universal-keyframe principle is felt most directly here.

**Timeline-format toggle** (per §2) applies strongly: Finalizing's primary interface can switch between keyframe mode and traditional clip-on-track editing on the same underlying data. A video editor never has to leave clip-and-track thinking; keyframes are one toggle away for anyone who wants the animation-native view.

**Frame-by-frame** available throughout — per-frame trim, per-frame rotoscope touch-ups, per-frame transition nudging.

### Transitions out of Finalizing

- **Forward in pipeline:** Compositing (per-frame polish, color grading, effects), Audio (final mix, sound design, scoring).
- **Standalone delivery export** for content types where Finalizing IS the final step (Storyboard pitch decks, Design motion-graphics delivery, quick Mixed edits, simple Game trailers).
- **Back to Creative sections** when QC reveals issues requiring authoring changes.

### Still open within Finalizing

- Whether export presets live at the project level (§11) with mode-aware defaults, or each mode has its own preset library. Current call: project-level with mode-aware defaults.
- External-footage import workflow — drag/drop vs dedicated import dialog, thumbnail panel location. UI detail.
- Depth of color-grade / audio-mix preview inside Mixed mode (preview-only vs limited in-place editing). Current call: preview-only; depth lives downstream in Compositing and Audio.

---

### 12.7 Compositing [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The Post section for per-frame image processing. Color grading, layered effects, masking and rotoscoping, image cleanup, look development. Node-based work that sits between Finalizing (assembly + delivery prep) and Audio (mix). Where individual frames and sequences become finished pictures.

**Mode buttons:** `Composite` / `Color` / `Cleanup` — industry-expandable (e.g., `Track`, `Stylize`, `VFX-Comp` if usage demands promote them to standalone modes).

**Division of labor with related sections.**
- **vs Finalizing > Mixed:** Mixed shows quick color preview during assembly; Compositing > Color does the real grade with scopes, LUTs, and per-shot control.
- **vs 3D Animation > VFX:** 3D VFX *makes* effects — sims, particles, procedural effects authored in 3D space. Compositing *integrates* rendered effects into the final image — layer combining, plate integration, lens-style effects on top.

---

#### Composite

**Primary activity:** node-based image composition. Combining render layers, applying filter / blend / transform operations, building the final pixel through a graph.

**Screen layout:**
- **Node graph workspace** — large, dominant. Where the work is.
- Image preview / viewer — shows the result of the currently-selected node
- Render layer / pass selector (input passes from §12.3 and other Creative sections auto-available)
- Backdrop image visualization
- Common node templates (color correct, blur, mix, transform, filter)

**Data model:** `ID_NT` (NodeTree — the compositor tree), `ID_IM` (image inputs and outputs), `ID_SCE` (compositor config). Masks fold into the NodeTree per §10 Bucket 3 (no separate `ID_MSK`). No new datablocks.

**Transitions out:** Color (grade the composited result), Cleanup (per-frame fixes), Audio (final mix), back to Creative sections if the comp reveals authoring problems.

---

#### Color

**Primary activity:** color grading. Look development. Scope-driven exposure, contrast, and color decisions. Per-shot grade application.

**Screen layout:**
- **Color-accurate image viewer** (calibrated, large)
- **Scopes panel** — waveform, vectorscope, RGB parade, histogram (critical for grading)
- Color wheels — lift/gamma/gain or shadows/midtones/highlights
- Curves editor
- LUT loader — industry-standard `.cube` files at the §5 boundary
- A/B reference frame comparison
- Per-shot grade timeline (which grade applies to which shot range)

**Data model:** `ID_NT` (color nodes within the compositor tree), `ID_IM` (LUT data or external LUT references through §5 boundary). No new datablocks.

**Transitions out:** Cleanup (if grade reveals visual issues), Audio (final mix), standalone export of graded sequence.

---

#### Cleanup

**Primary activity:** per-frame paint-out. Rotoscoping. Dust-busting. Wire / rig removal. Image-level fixes that can't be done procedurally.

**Screen layout:**
- Image viewer with brush / paint tools
- Onion-skin overlays for tracked areas (see context across frames)
- Mask / roto-spline tools
- Per-frame paint-over with brush
- Original / cleaned A/B toggle
- Track-and-paint workflow — paint once, propagate via motion tracking

**Data model:** `ID_NT` (cleanup nodes including roto masks per §10), `ID_IM` (paint layers stored as image data), `ID_MC` (MovieClip when cleaning imported video). No new datablocks.

**Transitions out:** Composite (re-integrate cleaned frames), Color (grade the cleaned-up result), Audio (final mix).

---

### Connection to §2 universal keyframe

Compositing is one of Blender's most natively-keyframed contexts. Every node value, color-grade parameter, mask vertex, and effect intensity can animate. Per-shot grades are just keyframed grade-node values across time. Effect ramps, transition reveals, growing masks — all keyframes.

**Frame-by-frame** is dominant in Cleanup (rotoscoping and paint-out are inherently per-frame) and available throughout for any per-frame adjustment.

### Transitions out of Compositing

- **Forward in pipeline:** Audio (final mix, sound design, scoring).
- **Standalone export** for picture-locked deliverables (final image sequences, video without final mix).
- **Back to Finalizing** if compositing reveals an editing issue (a shot doesn't work, needs replacement or reorder).
- **Back to Creative sections** if compositing reveals an authoring issue (re-render needed, re-animate a problem shot, fix asset topology).

### Still open within Compositing

- Whether `Color` stays its own mode or merges into Composite as a node-cluster preset. Current call: own mode — scopes/wheels ergonomics differ enough from generic node work.
- `Track` (camera tracking, match-move) as a fourth mode. Parked; promote when motion-tracking workflows mature.
- `Stylize` for non-photorealistic look development (painterly, watercolor, etc.). Parked; community interest is real but industry urgency low.
- Per-shot vs per-sequence grade scope — UX detail in Color.

---

### 12.8 Audio [LOCKED in principle, pixel-level UI still OPEN]

**What it is.** The final Post section. Multi-track mixing, musical composition / scoring, sound design, final mastering. Where the project's audio becomes finished — dialogue leveled, music scored, SFX layered, and the whole mix mastered for delivery.

**Mode buttons:** `Mix` / `Score` — industry-expandable (e.g., `Sound Design`, `Foley`, `Denoise`, `Dialog`, `Spatial` if usage demands promote them to standalone modes).

**Division of labor with related sections.**
- **vs Finalizing > Mixed (audio preview):** Mixed shows quick audio preview during assembly; Audio does the real multi-track mix with full automation, mastering, and delivery-format audio.
- **vs Creative sections (scratch audio):** Storyboarding, 2D Animation, 3D Animation, Design, and Game can all place scratch audio during authoring (reference dialogue, temp music, placeholder SFX). Audio replaces scratch tracks with production-ready sound.

---

#### Mix

**Primary activity:** multi-track audio mixing. Balancing levels, pan, EQ, compression, effects. Final mixdown for delivery.

**Screen layout:**
- **Multi-track timeline** — one track per audio source (dialogue, music, SFX, ambience, etc.)
- **Mixing console** — fader per track, pan, mute / solo, send / insert effects
- **Meters** — peak, RMS, loudness (LUFS for delivery compliance)
- **Automation lanes** — keyframe-backed per §2; timeline-format toggle between keyframe mode and traditional fader-drawn automation
- Master bus with final-mix processing
- Monitoring routing (studio monitors, headphones, phone / laptop speaker preview)

**Data model:** `ID_SO` (Sound — one per audio clip / source), `ID_SCE` (scene-level mix state), `ID_NT` (node-based audio effect chains). No new datablocks.

**Transitions out:** standalone export, forward to delivery (final mix as video-embedded audio or standalone file), back-cycle to Finalizing or Creative sections (see below).

---

#### Score

**Primary activity:** musical composition and scoring. Writing music for the project — sequencing, arranging, recording or synthesizing.

**Screen layout:**
- **Piano roll / sequencer** — note entry, MIDI editing, pitch / velocity / timing
- **Score sheet view** (optional) — traditional musical notation for composers who prefer it
- **Instrument / sample library** — Blended-native synths, SoundFonts, VST/AU at the §5 boundary
- **Transport** — tempo, time signature, bar markers synced with the project timeline
- **Scene-sync panel** — current project frame / shot shown alongside musical bar / measure
- **Arrangement panel** — verse / chorus markers, cue points, hit points matched to scene beats

**Data model:** `ID_SO` (rendered / recorded audio output), `ID_NT` (node-based synth and instrument chains; MIDI data lives inside the NodeTree to avoid a new datablock). No new datablocks.

**Transitions out:** Mix (fold composed music into the full multi-track mix), standalone music export, back-cycle (see below).

---

### Connection to §2 universal keyframe

Audio is explicitly named in §2 as a timeline-format section with the keyframe ↔ traditional editing toggle. Fader levels, pan, effect intensity, EQ parameters, MIDI expression, instrument parameters — all keyframeable. Automation lanes in Mix *are* keyframes rendered as curves; the toggle just changes the presentation.

**Frame-by-frame** (per-sample editing, at the extreme) is edge-case for Audio — unusual enough that v1 scopes it out. Keyframes on mix parameters (volume / pan / effects) are the universal commitment; per-sample editing is aspirational and parked.

### Transitions out of Audio

- **Final delivery:** mastered mixdown exported for the project's target — video render with embedded audio, standalone audio file, broadcast master, game audio assets, streaming-format deliverables.
- **Standalone export:** music or SFX as isolated audio files (when Audio is the final product, e.g., a music release, a podcast, a sound library).

### Back-cycling from Audio

Audio work often reveals issues in earlier sections. Back-cycling is first-class:

- **Audio ↔ Finalizing:** music or SFX timing exposes edit-rhythm issues — a cue lands wrong, a hit needs the shot to hold longer. Back to Finalizing to re-cut or retime.
- **Audio ↔ Storyboarding:** scratch dialogue or temp music was misleading; now the real sound is different timing. Back to Storyboarding to re-time panels, or adjust scene transitions.
- **Audio ↔ 2D Animation / 3D Animation:** real voice-actor takes differ from scratch dialogue; mouth shapes, gestures, or camera moves need adjustment. Back to the Animate mode in either section.
- **Audio ↔ Design:** title-sequence sync or lyric-video timing needs the design element to move differently. Back to Design (often Design > Graphic or Design > Illustration).
- **Audio ↔ Compositing:** music hits vs visual cues need synchronized adjustment — a sync sting needs the comp effect to land on the downbeat. Back to Compositing.
- **Audio ↔ Game:** trailer / cinematic audio reveals pacing issues in the visual edit or game-footage selection. Back to Game > Asset or Game > Level for re-capture.

Same `.blended` file, same timeline, same data. The back-cycle is a mode switch, not an export / re-import.

### Still open within Audio

- `Sound Design` as a standalone mode (distinct from Mix for foley / SFX creation). Parked; could promote when usage demands.
- `Denoise` / `Dialog` cleanup as specialized modes vs tools within Mix. Current call: tools within Mix.
- `Spatial` audio (3D positional, binaural, Ambisonic) for VR / XR / game audio. Parked; industry moving this way.
- VST / AU plugin support at the §5 boundary — how external audio plugins integrate. TBD, overlaps with §5.
- Per-sample editing vs keyframed mix parameters — scoped out of v1; aspirational.

---

## 13. Notes [HISTORY]

The project's history that lives in the doc instead of in git. Git has every commit; this section has the narrative — the renames, the superseded designs, the reframes — so a reader can orient without scrubbing the log.

---

### 13.1 Superseded: original Launcher Architecture (was §6)

The v1 launcher was a tier-based door menu: substrate choice (3D vs 2D) at tier 1, focused workspace doors inside each at tier 2. Superseded by §11 Pipeline as UX, which restructured the launcher around production pipeline sections (Storyboarding → 2D Animation → 3D Animation → Game → Design → Finalizing → Compositing → Audio) with mode buttons under each. Original content preserved here verbatim:

#### Structure (original)

- Splash screen (logo, copyright) → launcher.
- Launcher greets with something like **"What would you like to blend today?"**
- **Tier 1:** Substrate choice — 3D or 2D.
- **Tier 2 (within 3D):** Focused doors. Animator is the **headline door** (larger/central); supporting doors sit under it — Model, Sculpt, View, Edit, Render.
- **Tier 2 (within 2D):** Doors are more co-equal (small coherent space, no clear headline). Draw, Animate, Ink, Paint, and whatever the community adds.

#### Principles (original)

- **Visual hierarchy tells the truth.** Inside 3D, animation is the shaping discipline, so the Animator door is visibly headline. Inside 2D, no single door dominates.
- **Launcher is a committed router**, not a file-template picker buried in a splash screen.
- **Each door is a filtered view of the shared project model.** Same `.blended` file, radically different UI per door. The Animator literally cannot see sculpt brushes; the Sculptor literally cannot see the NLA editor.
- **"Viewing"** is not a workflow door — it is either the implicit default state of the launcher, or a small "drop into viewport without picking a workflow" affordance.

#### What each door served (supporting roles, original)

- **Sculpt** — asset prep for animation *or* end-use sculpting. Not a ZBrush replacement. Scope: enough to finish a shot.
- **Model** — geometry creation. Serves animation and also legitimate 3D software use (game asset export, product viz).
- **Video Edit (VSE)** — cross-substrate post tool. Finishes animation into a final cut. Not a Resolve replacement.
- **Compositor** — cross-substrate. Finishes shots.
- **Render** — animation has to output.
- **Grease Pencil** — primary citizen of 2D substrate. Can optionally appear as a bridging object in 3D scenes.

---

### 13.2 Rename log

- **Editing → Finalizing** (§12.6). Renamed because *editing* happens in every Creative section when authors adjust their own work; *finalizing* uniquely names the delivery-prep activity that lives there.
- **Game Design → Game** (§12.4). Naming consistency with the other one-word headings.
- **"One engine, two lenses" → "One engine, every content type"** (§2). Originally framed as just 2D-vs-3D sharing the engine. Expanded once we recognized every section's properties are keyframeable, not only the animation sections.

---

### 13.3 Vision C reframe

§3 originally said *"Blended = A's discipline + B's accessibility, with C explicitly rejected."* Reframed to *"A's discipline + B's accessibility + C's **scope**, with C's **pathology** rejected."* We embrace the breadth Vision C reaches for; we reject the feature-parity arms race. Current statement: §3. In/out-of-scope cash-out: §7.

---

### 13.4 Stage-collapse note

Earlier pipeline drafts had Assets, Environments, VFX, and Animate as standalone top-level stages. They're now modes inside §12.3 3D Animation. Game Design (now Game) was briefly nested under 3D Animation before being promoted to its own §12.4 section.

---

## 14. Document Conventions

- Tag new sections with [LOCKED] / [OPEN] / [REJECTED] / [GUARDRAIL].
- When reopening a LOCKED section, state why and what new evidence changed the call.
- When a decision closes an OPEN question, move it out and summarize in the relevant section.
- Keep the active sections (§1–§12) tight. They're working agreements, not narrative.
- §13 Notes is the doc's history. Renames, supersessions, and reframes go there in narrative form. Git has the full record; §13 has the summary a reader can read in a minute.
- When a section is renamed or superseded, leave a one-line stub at the old location pointing to its new home, and add the entry to §13.
