# SPDX-FileCopyrightText: 2026 CHJ 3 Productions LLC
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Blended launcher header — file management chrome per BLENDED.md §11.

Draws the compact top strip above the pipeline scroll:
  [Blended]  [New Project]  [Open...]  [Open Recent ▸]
"""

import bpy
from bpy.types import Header


class LAUNCHER_HT_header(Header):
    bl_space_type = 'BLENDED_LAUNCHER'

    def draw(self, _context):
        layout = self.layout

        # Wordmark — identity anchor on the left.
        layout.label(text="Blended")

        layout.separator()

        # New project — reads the startup file (blank project).
        layout.operator("wm.read_homefile", text="New Project", icon='FILE_NEW')

        # Open — invokes the file browser.
        layout.operator("wm.open_mainfile", text="Open…", icon='FILE_FOLDER')

        # Recent files — reuses Blender's existing TOPBAR menu, same as the splash screen.
        layout.menu("TOPBAR_MT_file_open_recent", text="Open Recent")


classes = (LAUNCHER_HT_header,)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
