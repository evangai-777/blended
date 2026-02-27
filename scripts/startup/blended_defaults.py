# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Blended Smart Defaults.

Applies sensible default settings based on the current UI tier.
Runs on startup and when the tier changes.
"""

import bpy


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
        # EEVEE is already the default engine - ensure it stays.
        if render.engine not in {'BLENDER_EEVEE', 'BLENDER_EEVEE_NEXT'}:
            render.engine = 'BLENDER_EEVEE_NEXT'

        # Higher viewport quality for a better out-of-box experience.
        eevee.taa_samples = 32
        eevee.taa_render_samples = 128

        # Enable common quality settings.
        eevee.use_gtao = True

        # Simple: hide Python tooltips to reduce clutter.
        view.show_tooltips_python = False

    elif tier == TIER_STANDARD:
        # Standard: Python tooltips off by default (less noise).
        view.show_tooltips_python = False

    else:
        # Advanced: enable Python tooltips for scripting users.
        view.show_tooltips_python = True


@bpy.app.handlers.persistent
def _on_load_post(_):
    """Apply tier defaults after loading a new file."""
    # Only apply to truly new files (factory startup).
    # Don't override settings in existing .blend files.
    if not bpy.data.filepath:
        apply_tier_defaults()


def register():
    bpy.app.handlers.load_post.append(_on_load_post)


def unregister():
    bpy.app.handlers.load_post.remove(_on_load_post)
