<!--
Keep this document short & concise,
linking to external resources instead of including content in-line.
See 'release/text/readme.html' for the end user read-me.
-->

Blended
=======

**Blender, simplified.**

Blended is a fork of [Blender](https://www.blender.org) that adds a tiered UI complexity system, smart defaults, and built-in update notifications. The full power of Blender is still there — Blended just makes it easier to approach by letting users choose how much of it they see.

Blended 1.0.0 — Based on Blender 5.2.

What's Different from Blender
-----------------------------

### Tiered UI System

Blended introduces three UI tiers, selectable from the splash screen or Preferences > Interface > Display:

| Tier | Who it's for | What changes |
|------|-------------|--------------|
| **Simple** | Beginners and casual users | Hides advanced editors (Geometry Nodes, Video Sequencer, Compositor, etc.). Shows a curated Add menu (Mesh, Light, Camera, Empty, Text) and ~15 essential modifiers. Hides Physics, Particles, Constraints, World, and Effects panels. Hides advanced Render, Object, Scene, View Layer, and Output properties. |
| **Standard** | Intermediate users (default) | Shows most editors and panels. Hides niche editors like the Video Sequencer. A balanced middle ground. |
| **Advanced** | Power users | Full Blender — everything visible, nothing hidden. Enables Python tooltips for scripting workflows. |

The tier setting is stored in user preferences and can be changed at any time.

### Smart Defaults

When creating a new file, Blended applies tier-aware defaults:

- **Simple tier**: Forces EEVEE renderer, increases viewport TAA samples (32) and render samples (128), enables Ambient Occlusion, and hides Python tooltips.
- **Standard tier**: Hides Python tooltips to reduce UI noise.
- **Advanced tier**: Enables Python tooltips for scripting users.

Existing `.blend` files are never modified — defaults only apply to new scenes.

### Branding

- Window titles, splash screen, and about dialog show "Blended" with the tagline "Blender, simplified".
- Includes a custom Blended.xml theme preset with softer dark colors and a blue accent.

### Update Notifications

Blended checks [GitHub Releases](https://github.com/EvangAI-777/Blended/releases) for new versions on startup (in a background thread, non-blocking). Results are cached for 24 hours. When an update is available, a notification appears in the top bar with a one-click download button.

### CI / Prebuilt Binaries

A GitHub Actions workflow builds portable Windows x64 `.zip` packages on every push to `main`/`master`. Tagged releases (`v*`) automatically create GitHub Releases with the build attached.

Building from Source
--------------------

Blended builds the same way as Blender. Follow the standard Blender build instructions:

- [Build Instructions](https://developer.blender.org/docs/handbook/building_blender/)

Syncing Upstream Changes
------------------------

See [UPSTREAM_SYNC.md](../UPSTREAM_SYNC.md) for how to merge new Blender releases into Blended.

Upstream Blender Resources
--------------------------

- [Blender Website](https://www.blender.org)
- [Reference Manual](https://docs.blender.org/manual/en/latest/index.html)
- [User Community](https://www.blender.org/community/)
- [Developer Documentation](https://developer.blender.org/docs/)

License
-------

Blended, like Blender, is licensed under the GNU General Public License, Version 3.
Individual files may have a different but compatible license.

See [blender.org/about/license](https://www.blender.org/about/license) for details.
