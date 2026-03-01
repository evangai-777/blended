# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Blended Update Checker.

Checks GitHub Releases for new versions of Blended on startup.
Respects the user's internet access preference (USER_INTERNET_ALLOW).
Caches results to avoid repeated checks (max once per day).
"""

import bpy
import json
import time
import os
import threading

# Configuration - update these for your fork.
GITHUB_OWNER = "EvangAI-777"
GITHUB_REPO = "Blended"
CHECK_INTERVAL_SECONDS = 86400  # 24 hours

# Module-level state for the update check result.
_update_info = {
    "available": False,
    "latest_version": "",
    "download_url": "",
    "release_notes": "",
    "checked": False,
}


def _get_cache_filepath():
    """Return the path to the update check cache file."""
    config_dir = bpy.utils.user_resource('CONFIG')
    if not config_dir:
        return None
    return os.path.join(config_dir, "blended_update_cache.json")


def _read_cache():
    """Read cached update check result."""
    filepath = _get_cache_filepath()
    if not filepath or not os.path.exists(filepath):
        return None
    try:
        with open(filepath, "r") as f:
            return json.load(f)
    except (json.JSONDecodeError, OSError):
        return None


def _write_cache(data):
    """Write update check result to cache."""
    filepath = _get_cache_filepath()
    if not filepath:
        return
    try:
        os.makedirs(os.path.dirname(filepath), exist_ok=True)
        with open(filepath, "w") as f:
            json.dump(data, f)
    except OSError:
        pass


def _parse_version(version_string):
    """Parse a version string like 'v1.0.0' or '1.0.0' into a tuple."""
    version_string = version_string.lstrip("v").strip()
    parts = []
    for part in version_string.split("."):
        try:
            parts.append(int(part))
        except ValueError:
            # Handle suffixes like '0-alpha'.
            num = ""
            for ch in part:
                if ch.isdigit():
                    num += ch
                else:
                    break
            parts.append(int(num) if num else 0)
    return tuple(parts)


def _current_version():
    """Return the current Blended version as a tuple."""
    return (bpy.app.version[0], bpy.app.version[1], bpy.app.version[2])


def _check_for_updates_thread():
    """Background thread to check GitHub for updates."""
    global _update_info

    try:
        import urllib.request
        import urllib.error

        url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/releases/latest"
        req = urllib.request.Request(url)
        req.add_header("Accept", "application/vnd.github.v3+json")
        req.add_header("User-Agent", "Blended-Update-Checker")

        with urllib.request.urlopen(req, timeout=10) as response:
            data = json.loads(response.read().decode("utf-8"))

        tag = data.get("tag_name", "")
        latest = _parse_version(tag)
        current = _current_version()

        download_url = ""
        for asset in data.get("assets", []):
            download_url = asset.get("browser_download_url", "")
            break  # Take first asset.

        if not download_url:
            download_url = data.get("html_url", "")

        _update_info = {
            "available": latest > current,
            "latest_version": tag,
            "download_url": download_url,
            "release_notes": data.get("body", "")[:500],
            "checked": True,
        }

        # Cache the result.
        cache_data = {
            "timestamp": int(time.time()),
            "latest_version": tag,
            "download_url": download_url,
            "available": latest > current,
        }
        _write_cache(cache_data)

    except Exception:
        # Silently fail - don't bother users with update check errors.
        _update_info["checked"] = True


def check_for_updates():
    """Initiate an update check (respects preferences and caching)."""
    # Respect the user's internet access preference.
    prefs = bpy.context.preferences
    if not (prefs.system.use_preferences_save if hasattr(prefs.system, 'use_preferences_save') else True):
        return

    # Check cache first.
    cache = _read_cache()
    if cache:
        last_check = cache.get("timestamp", 0)
        if (time.time() - last_check) < CHECK_INTERVAL_SECONDS:
            # Use cached result.
            _update_info["available"] = cache.get("available", False)
            _update_info["latest_version"] = cache.get("latest_version", "")
            _update_info["download_url"] = cache.get("download_url", "")
            _update_info["checked"] = True
            return

    # Run check in background thread to avoid blocking startup.
    thread = threading.Thread(target=_check_for_updates_thread, daemon=True)
    thread.start()


def get_update_info():
    """Return the current update info dict."""
    return _update_info


# Operator to open the download URL.
class BLENDED_OT_open_download(bpy.types.Operator):
    bl_idname = "blended.open_download"
    bl_label = "Download Update"
    bl_description = "Open the download page for the latest Blended release"

    def execute(self, context):
        info = get_update_info()
        if info["download_url"]:
            bpy.ops.wm.url_open(url=info["download_url"])
        return {'FINISHED'}


# Draw function appended to the top bar for update notification.
def _draw_update_indicator(self, context):
    info = get_update_info()
    if info["available"]:
        layout = self.layout
        row = layout.row(align=True)
        row.alert = True
        row.operator(
            "blended.open_download",
            text=f"Update: {info['latest_version']}",
            icon='INFO',
        )


# Handler to trigger update check after file load.
@bpy.app.handlers.persistent
def _on_load_post(_):
    check_for_updates()


def register():
    bpy.utils.register_class(BLENDED_OT_open_download)
    bpy.types.TOPBAR_HT_upper_bar.append(_draw_update_indicator)
    bpy.app.handlers.load_post.append(_on_load_post)
    # Also trigger on first register (startup).
    check_for_updates()


def unregister():
    bpy.app.handlers.load_post.remove(_on_load_post)
    bpy.types.TOPBAR_HT_upper_bar.remove(_draw_update_indicator)
    bpy.utils.unregister_class(BLENDED_OT_open_download)
