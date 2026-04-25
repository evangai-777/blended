# Blended Changelog

## 0.1.0

First independent Blended release. Based on Blender 5.2 alpha.

### Branding

- CMake project renamed to `Blended`
- Window titles: `Blended 0.1.0`
- Splash screen: name `Blended`, tagline *"Blender, simplified."*
- About dialog updated to Blended
- Windows file metadata (ProductName, FileDescription, CompanyName) updated
- Linux desktop entry (Name, Comment, StartupWMClass) updated

### Version Identity

- `BLENDED_VERSION_MAJOR`, `BLENDED_VERSION_MINOR`, `BLENDED_VERSION_PATCH` defines
  in `BKE_blender_version.h`, independent of Blender's `BLENDER_VERSION` integer
  (which stays at 502 for `.blend` file format compatibility)
- `BKE_blended_version_string()` — Blended-specific version string, used in window
  titles and splash screen
- CI artifact named `Blended-0.1.0-windows-x64.zip`

### Pre-5.0 Rig Compatibility

- `scripts/startup/blended_rig_compat.py` — restores `action.fcurves` as a Python
  property on `bpy.types.Action`
- `_FCurvesCompat` proxy flattens F-Curves from all channelbags across all
  layers/strips in Blender 5.x's layered action system
- Pre-Blender-5.0 Rigify rigs (including CGCookie Vonnbots rigs) that access
  `action.fcurves` directly work again
- IK/FK bake operators no longer fail silently

### Update Notifications

- `scripts/startup/blended_update_check.py` — background GitHub Releases check
  at startup (non-blocking, 24-hour cache)
- Top-bar notification with version string when an update is available
- One-click download via browser (`BLENDED_OT_open_update_page`)
- `BLENDED_PT_update_prefs` panel in System Preferences → System

### CI

- Windows x64 portable `.zip` builds via GitHub Actions (`build-windows.yml`)
- Manual dispatch (`workflow_dispatch`) for development builds
- Tag push (`v*`) for release builds → artifact + GitHub Release
- `blended_release.cmake` — inherits `blender_release.cmake`, disables GPU kernel
  pre-compilation (CUDA/HIP/OneAPI) and Freestyle for CI runners
- LFS handled explicitly: source via `projects.blender.org/blender/blender.git`,
  libraries via `projects.blender.org/blender/lib-windows_x64.git`
