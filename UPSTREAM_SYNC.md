# Syncing with Upstream Blender

Blended is a fork of [Blender](https://www.blender.org/). This document describes
how to manually merge upstream Blender changes into Blended.

> *"Appreciate what already is."* — [PHILOSOPHY.md](PHILOSOPHY.md)
>
> Upstream Blender is the foundation we build on. Syncing is an act of
> appreciation — integrating the work of hundreds of contributors while
> preserving the Blended layer on top. Don't fight the upstream; be WITH it.

## One-Time Setup

Add the upstream Blender repository as a remote:

```bash
git remote add upstream https://projects.blender.org/blender/blender.git
```

Verify:

```bash
git remote -v
# origin    https://github.com/EvangAI-777/Blended.git (fetch)
# origin    https://github.com/EvangAI-777/Blended.git (push)
# upstream  https://projects.blender.org/blender/blender.git (fetch)
# upstream  https://projects.blender.org/blender/blender.git (push)
```

## Merge Workflow

### 1. Fetch upstream

```bash
git fetch upstream
```

### 2. Create a sync branch

```bash
git checkout main
git checkout -b sync/blender-v5.3-release
```

### 3. Merge the upstream release branch

```bash
git merge upstream/blender-v5.3-release
```

### 4. Resolve conflicts

See the "Conflict-Prone Files" section below for guidance on each file.

### 5. Update library submodules

```bash
python build_files/utils/make_update.py
```

### 6. Build and test

Verify the build succeeds and Blended-specific features still work:
- Tier system (Simple/Standard/Advanced)
- Splash screen tier selector
- Blended version string displays correctly
- Panel gating per tier

### 7. Open a PR

Push your sync branch and open a pull request against `main` for review.

## Conflict-Prone Files

These files are modified by Blended and will likely conflict on upstream merges.
For each, keep the Blended additions and integrate new upstream changes around them.

| File | Blended Changes |
|------|-----------------|
| `source/blender/blenkernel/BKE_blender_version.h` | `BLENDED_VERSION_*` defines, `BLENDED_VERSION_STRING`, `BLENDED_TAGLINE` |
| `source/blender/blenkernel/intern/blender.cc` | Version string formatting uses `BLENDED_VERSION_*` |
| `source/blender/python/intern/bpy_app.cc` | `bpy.app.version` returns Blended version |
| `source/blender/makesdna/DNA_userdef_types.h` | `ui_tier` field in UserDef |
| `source/blender/makesdna/DNA_userdef_enums.h` | `eUserPref_UI_Tier` enum |
| `source/blender/makesrna/intern/rna_userdef.cc` | Tier RNA property + callback |
| `source/blender/makesdna/DNA_workspace_types.h` | `blended_min_tier` field |
| `source/blender/makesrna/intern/rna_workspace.cc` | `blended_min_tier` RNA property |
| `source/blender/makesrna/intern/rna_screen.cc` | Tier filtering for panels |
| `source/blender/windowmanager/intern/wm_splash_screen.cc` | Tier selector in splash |
| `source/blender/windowmanager/intern/wm_window.cc` | Window title uses Blended string |
| `source/blender/editors/interface/templates/interface_template_id.cc` | Workspace tab filtering |
| `scripts/startup/bl_ui/` (multiple files) | `blended_min_tier` on panels, simplified menus |
| `scripts/startup/blended_defaults.py` | Smart defaults per tier |
| `scripts/modules/blended_utils.py` | Tier utility functions |
| `CMakeLists.txt` | `project(Blended)` |
| `build_files/cmake/macros.cmake` | `BLENDED_VERSION_*` extraction |
| `build_files/cmake/config/blended_release.cmake` | Custom build config |
| `release/windows/icons/winblender.rc` | Blended branding strings (CompanyName, ProductName, etc.) |
| `release/freedesktop/blender.desktop` | Blended Name, Exec, Icon, StartupWMClass |
| `CHANGELOG.md` | Blended-only file (does not exist upstream) |
| `.github/` | Workflows, README, PR template |

## After Syncing: Version Bump Checklist

When syncing to a new Blender release (e.g., 5.2 to 5.3):

1. Update `BLENDED_BASED_ON_BLENDER` in `BKE_blender_version.h`
2. Consider bumping `BLENDED_VERSION_MINOR` or `BLENDED_VERSION_PATCH`
3. Update `.github/README.md` to reference the new upstream version
4. Test that `.blend` file compatibility is maintained (`BLENDER_FILE_VERSION` tracks upstream)

---

## Cherry-Picking

Rather than doing a full merge, individual upstream commits can be cherry-picked
when they fix something Blended already touches or add a capability that fits
the project's goals without conflict risk.

### When to Cherry-Pick vs Full Merge

Cherry-pick when:
- The change is isolated to one or two files with no Blended overlap
- It's a bug fix in a subsystem Blended modifies (safer to pull in early)
- It's a UX improvement directly relevant to learners

Full merge instead when:
- Many related commits need to land together to work correctly
- Dependency libraries (OpenVDB, OpenColorIO, etc.) are updated
- Python version is bumped (requires coordinated changes across many files)

### Blender 5.1 Changes Worth Evaluating (as of April 2026)

Blender 5.1 was released March 17, 2026. The fork currently tracks the
Blender 5.2 alpha branch (`BLENDER_VERSION 502`). These 5.1 changes may or
may not be present in the 5.2 alpha base — verify before cherry-picking.

| Change | Affected Blended Areas | Priority | Notes |
|--------|----------------------|----------|-------|
| **Preferences search field added** | Tier dropdown in Preferences UI | High | Blended adds a tier selector to Preferences — verify it's findable by search and renders correctly alongside the new search widget |
| **Asset Libraries individually disableable** | `blended_defaults.py` | High | Ideal Simple-tier smart default: disable non-essential asset libraries for new users. `bpy.context.preferences.filepaths.asset_libraries` |
| **`template_list` `columns` param deprecated** | Any panel using `template_list` | Medium | Audit all tier-gated panels that call `template_list` — remove `columns=` argument if present |
| **Node Tools require global unique `idname`** | `space_node.py` node add menu | Medium | Blended hides node submenus by tier; verify no node tool `idname` conflicts after this change |
| **Grease Pencil fills revamp** (auto-converted on open) | GP tier-gated panels | Medium | GP panels are gated to Standard/Advanced tier — confirm panel visibility logic still holds after fill system conversion |
| **Python upgraded to 3.13** | `blended_utils.py`, `blended_defaults.py`, `blended_update_check.py` | Low | Likely compatible, but run `make check_mypy` and `make check_pep8` against 3.13 to confirm |
| **Outliner auto-scroll on rename** | No Blended overlap | Low | Pure UX polish, safe to include |

### How to Verify a Cherry-Pick Candidate

```bash
# Find the commit on the upstream blender-v5.1-release branch
git fetch upstream
git log upstream/blender-v5.1-release --oneline --grep="asset librar"

# Preview what the commit changes
git show <commit-sha> --stat

# Cherry-pick onto your sync branch
git cherry-pick <commit-sha>
```

If the cherry-pick conflicts with a Blended-modified file, resolve by keeping
the Blended addition and integrating the upstream change around it — same
strategy as a full merge conflict. See "Conflict-Prone Files" above.
