# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Blended Smart Defaults.

Applies sensible default settings based on the current UI tier.
Runs on startup and when the tier changes.
"""

import bpy

# Workspace tier assignments: minimum tier required to show each workspace tab.
# Workspaces not listed here default to tier 0 (always visible).
_WORKSPACE_TIERS = {
    # Simple tier (0) - always visible
    "Layout": 0,
    "Sculpting": 0,
    "UV Editing": 0,
    # Standard tier (1)
    "Modeling": 1,
    "Animation": 1,
    "Compositing": 1,
    "Texture Paint": 1,
    "Shading": 1,
    # Advanced tier (2)
    "Geometry Nodes": 2,
    "Scripting": 2,
    "Rendering": 2,
}


def _apply_workspace_tiers():
    """Tag each workspace with its minimum tier for tab filtering."""
    for ws in bpy.data.workspaces:
        ws.blended_min_tier = _WORKSPACE_TIERS.get(ws.name, 0)


def _set_all_viewport_shading(shading_type):
    """Set viewport shading type for all 3D viewports in all screens.

    Args:
        shading_type: One of 'WIREFRAME', 'SOLID', 'MATERIAL', 'RENDERED'.
    """
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type == 'VIEW_3D':
                for space in area.spaces:
                    if space.type == 'VIEW_3D':
                        space.shading.type = shading_type


def _setup_startup_scene(scene, tier):
    """Configure the startup scene objects based on tier."""
    from blended_utils import TIER_SIMPLE, TIER_ADVANCED

    if tier == TIER_SIMPLE:
        _setup_simple_scene(scene)
    elif tier == TIER_ADVANCED:
        _setup_advanced_scene(scene)
    # Standard: leave the default Blender scene as-is (cube, camera, light).


def _setup_simple_scene(scene):
    """Simple tier: add ground plane and convert light to sun."""
    import bmesh

    # Add ground plane if not already present.
    if "Ground" not in bpy.data.objects:
        mesh = bpy.data.meshes.new("Ground")
        bm = bmesh.new()
        bmesh.ops.create_grid(bm, x_segments=1, y_segments=1, size=10.0)
        bm.to_mesh(mesh)
        bm.free()

        obj = bpy.data.objects.new("Ground", mesh)
        scene.collection.objects.link(obj)

        mat = bpy.data.materials.new("Ground Material")
        mat.diffuse_color = (0.3, 0.3, 0.3, 1.0)
        obj.data.materials.append(mat)

    # Convert existing point light to sun for nicer default lighting.
    light_obj = bpy.data.objects.get("Light")
    if light_obj and light_obj.type == 'LIGHT':
        light_obj.data.type = 'SUN'
        light_obj.data.energy = 3.0


def _setup_advanced_scene(scene):
    """Advanced tier: remove default cube (experienced users create what they need)."""
    cube = bpy.data.objects.get("Cube")
    if cube:
        bpy.data.objects.remove(cube, do_unlink=True)


def apply_tier_defaults(scene=None):
    """Apply smart defaults to the given scene (or active scene) based on UI tier."""
    from blended_utils import get_ui_tier, TIER_SIMPLE, TIER_STANDARD

    if scene is None:
        scene = bpy.context.scene
    if scene is None:
        return

    tier = get_ui_tier(bpy.context)
    eevee = scene.eevee
    render = scene.render

    prefs = bpy.context.preferences
    view = prefs.view

    if tier == TIER_SIMPLE:
        # Simple: Workbench for fast, distraction-free viewport.
        render.engine = 'BLENDER_WORKBENCH'

        # Simple: hide Python tooltips to reduce clutter.
        view.show_tooltips_python = False

        # Set viewport shading to Solid for all 3D viewports.
        _set_all_viewport_shading('SOLID')

    elif tier == TIER_STANDARD:
        # Standard: EEVEE with quality presets for a polished experience.
        render.engine = 'BLENDER_EEVEE_NEXT'

        # Higher viewport quality for a better out-of-box experience.
        eevee.taa_samples = 32
        eevee.taa_render_samples = 128

        # Enable common quality settings.
        eevee.use_gtao = True

        # Standard: Python tooltips off by default (less noise).
        view.show_tooltips_python = False

        # Set viewport shading to Material Preview for all 3D viewports.
        _set_all_viewport_shading('MATERIAL')

    else:
        # Advanced: EEVEE default, enable Python tooltips for scripting users.
        view.show_tooltips_python = True

    # Tier-aware sidebar visibility: hide N-panel in Simple to reduce clutter.
    if tier == TIER_SIMPLE:
        for screen in bpy.data.screens:
            for area in screen.areas:
                if area.type == 'VIEW_3D':
                    for region in area.regions:
                        if region.type == 'UI':
                            area.spaces[0].show_region_ui = False

    # Tier-aware outliner display: Simple uses VIEW_LAYER, Standard uses default.
    if tier == TIER_SIMPLE:
        for screen in bpy.data.screens:
            for area in screen.areas:
                if area.type == 'OUTLINER':
                    for space in area.spaces:
                        if space.type == 'OUTLINER':
                            space.display_mode = 'VIEW_LAYER'

    # Setup startup scene objects per tier.
    _setup_startup_scene(scene, tier)


@bpy.app.handlers.persistent
def _on_load_post(_):
    """Apply tier defaults after loading a file."""
    # Tag workspace tabs with tier metadata (always, for any file).
    _apply_workspace_tiers()

    # Only apply render/viewport defaults to new files (factory startup).
    # Don't override settings in existing .blend files.
    if not bpy.data.filepath:
        apply_tier_defaults()


_msgbus_owner = object()


def _on_tier_changed():
    """Called when the user changes the UI tier at runtime."""
    # Re-apply workspace tier metadata so tab visibility updates immediately.
    _apply_workspace_tiers()
    # Note: render engine/viewport defaults are NOT re-applied here to avoid
    # overriding the user's current work settings. Those only apply on new files.


def register():
    bpy.app.handlers.load_post.append(_on_load_post)

    # Subscribe to tier changes to refresh workspace tabs immediately.
    bpy.msgbus.subscribe_rna(
        key=(bpy.types.PreferencesView, "ui_tier"),
        owner=_msgbus_owner,
        args=(),
        notify=_on_tier_changed,
    )


def unregister():
    bpy.app.handlers.load_post.remove(_on_load_post)
    bpy.msgbus.clear_by_owner(_msgbus_owner)
