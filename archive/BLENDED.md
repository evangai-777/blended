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

## 6. Launcher Architecture [LOCKED in principle, UI details OPEN]

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

## 10. Document Conventions

- Tag new sections with [LOCKED] / [OPEN] / [REJECTED] / [GUARDRAIL].
- When reopening a LOCKED section, state why and what new evidence changed the call.
- When a decision closes an OPEN question, move it out and summarize in the relevant section.
- Keep this doc tight. It is a working agreement, not a history. History lives in git.
