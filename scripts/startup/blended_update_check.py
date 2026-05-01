# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Background update checker for Blended.

Polls the GitHub Releases API at startup (non-blocking). If a newer version
is available, a notification appears in the top bar. The check is cached for
24 hours so it does not run on every launch.
"""

import bpy
import threading
import json
import os
import time
from urllib.request import urlopen, Request
from urllib.error import URLError

import bpy.app.handlers

_CURRENT_VERSION = (
    bpy.app.version[0],  # placeholder — overridden in register()
)
_RELEASES_URL = "https://api.github.com/repos/EvangAI-777/Blended/releases/latest"
_CACHE_TTL = 86400  # 24 hours in seconds

_update_available = False
_latest_version_str = ""
_latest_url = ""


def _cache_path():
    return os.path.join(bpy.utils.user_resource('CONFIG'), "blended_update_cache.json")


def _read_cache():
    try:
        with open(_cache_path(), 'r') as f:
            return json.load(f)
    except (OSError, json.JSONDecodeError):
        return None


def _write_cache(data):
    try:
        with open(_cache_path(), 'w') as f:
            json.dump(data, f)
    except OSError:
        pass


def _parse_version(tag):
    """Parse 'v0.1.0' or '0.1.0' into a tuple of ints."""
    tag = tag.lstrip('v')
    try:
        return tuple(int(x) for x in tag.split('.'))
    except ValueError:
        return (0,)


def _fetch_latest():
    try:
        req = Request(_RELEASES_URL, headers={"User-Agent": "Blended-UpdateChecker/1.0"})
        with urlopen(req, timeout=10) as resp:
            data = json.loads(resp.read())
        return data.get("tag_name", ""), data.get("html_url", "")
    except (URLError, OSError, json.JSONDecodeError):
        return None, None


def _check_thread(current_version):
    global _update_available, _latest_version_str, _latest_url

    cache = _read_cache()
    now = time.time()

    if cache and (now - cache.get("timestamp", 0)) < _CACHE_TTL:
        tag = cache.get("tag_name", "")
        url = cache.get("html_url", "")
    else:
        tag, url = _fetch_latest()
        if tag:
            _write_cache({"tag_name": tag, "html_url": url, "timestamp": now})

    if not tag:
        return

    latest = _parse_version(tag)
    if latest > current_version:
        _update_available = True
        _latest_version_str = tag.lstrip('v')
        _latest_url = url


class BLENDED_OT_open_update_page(bpy.types.Operator):
    bl_idname = "blended.open_update_page"
    bl_label = "Download Update"
    bl_description = "Open the Blended releases page in your browser"

    def execute(self, context):
        import webbrowser
        webbrowser.open(_latest_url or _RELEASES_URL.replace("/latest", ""))
        return {'FINISHED'}


class BLENDED_MT_update_topbar(bpy.types.Menu):
    bl_label = "Blended Update"
    bl_idname = "BLENDED_MT_update_topbar"

    def draw(self, context):
        layout = self.layout
        layout.operator("blended.open_update_page",
                        text=f"Blended {_latest_version_str} available — click to download",
                        icon='URL')


def _draw_topbar_update(self, context):
    if _update_available:
        self.layout.menu("BLENDED_MT_update_topbar", icon='INFO')


class BLENDED_PT_update_prefs(bpy.types.Panel):
    bl_label = "Blended Updates"
    bl_idname = "BLENDED_PT_update_prefs"
    bl_space_type = 'PREFERENCES'
    bl_region_type = 'WINDOW'
    bl_context = "system"

    def draw(self, context):
        layout = self.layout
        if _update_available:
            layout.label(text=f"Blended {_latest_version_str} is available.", icon='INFO')
            layout.operator("blended.open_update_page")
        else:
            layout.label(text="Blended is up to date.", icon='CHECKMARK')


@bpy.app.handlers.persistent
def _startup_check(_):
    current = (
        bpy.app.blended_version_major,
        bpy.app.blended_version_minor,
        bpy.app.blended_version_patch,
    )
    t = threading.Thread(target=_check_thread, args=(current,), daemon=True)
    t.start()


def register():
    bpy.utils.register_class(BLENDED_OT_open_update_page)
    bpy.utils.register_class(BLENDED_MT_update_topbar)
    bpy.utils.register_class(BLENDED_PT_update_prefs)
    bpy.types.TOPBAR_HT_upper_bar.append(_draw_topbar_update)
    bpy.app.handlers.load_post.append(_startup_check)


def unregister():
    bpy.app.handlers.load_post.remove(_startup_check)
    bpy.types.TOPBAR_HT_upper_bar.remove(_draw_topbar_update)
    bpy.utils.unregister_class(BLENDED_PT_update_prefs)
    bpy.utils.unregister_class(BLENDED_MT_update_topbar)
    bpy.utils.unregister_class(BLENDED_OT_open_update_page)
