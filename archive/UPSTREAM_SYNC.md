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

- Window title shows "Blended 0.x.x"
- Splash screen shows "Blended" name and tagline
- `blended_rig_compat.py` loads — pre-5.0 Rigify rigs can access `action.fcurves`
- Update checker starts background thread on `load_post`

### 7. Open a PR

Push your sync branch and open a pull request against `main` for review.

## Conflict-Prone Files

These files are modified by Blended and will likely conflict on upstream merges.
For each, keep the Blended additions and integrate new upstream changes around them.

| File | Blended Changes |
|------|-----------------|
| `source/blender/blenkernel/BKE_blender_version.h` | `BLENDED_VERSION_MAJOR/MINOR/PATCH` defines, `BKE_blended_version_string()` declaration |
| `source/blender/blenkernel/intern/blender.cc` | `blended_version_string` buffer, `BKE_blended_version_string()` implementation, `blender_version_init()` call |
| `source/blender/windowmanager/intern/wm_window.cc` | Fallback title `"Blended"`, title suffix uses `BKE_blended_version_string()` |
| `source/blender/windowmanager/intern/wm_splash_screen.cc` | Splash version label, about dialog name/description, tagline `"Blender, simplified."` |
| `CMakeLists.txt` | `project(Blended)` at line ~81 |
| `build_files/cmake/config/blended_release.cmake` | Blended-only file — won't conflict, just preserve |
| `scripts/startup/blended_rig_compat.py` | Blended-only file — won't conflict, just preserve |
| `scripts/startup/blended_update_check.py` | Blended-only file — won't conflict, just preserve |
| `release/windows/icons/winblender.rc` | `CompanyName`, `FileDescription`, `LegalCopyright`, `ProductName` updated to Blended |
| `release/freedesktop/blender.desktop` | `Name`, `Comment`, `StartupWMClass`, `Keywords` updated to Blended identity |
| `.github/` | Workflows, README — Blended-only, won't conflict with upstream |
| `BLENDED.md` | Blended-only file |
| `CLAUDE.md` | Blended-only file |
| `CHANGELOG.md` | Blended-only file |

## After Syncing: Version Bump Checklist

When syncing to a new Blender release (e.g., 5.2 to 5.3):

1. Check `BLENDER_VERSION` integer in `BKE_blender_version.h` matches the new upstream value
2. Consider bumping `BLENDED_VERSION_MINOR` or `BLENDED_VERSION_PATCH`
3. Update `.github/README.md` to reference the new upstream base version
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
- It's a feature directly relevant to animation or the pipeline identity

Full merge instead when:
- Many related commits need to land together to work correctly
- Dependency libraries (OpenVDB, OpenColorIO, etc.) are updated
- Python version is bumped (requires coordinated changes across many files)

### How to Verify a Cherry-Pick Candidate

```bash
# Find the commit on the upstream branch
git fetch upstream
git log upstream/blender-v5.3-release --oneline --grep="asset librar"

# Preview what the commit changes
git show <commit-sha> --stat

# Cherry-pick onto your sync branch
git cherry-pick <commit-sha>
```

If the cherry-pick conflicts with a Blended-modified file, resolve by keeping
the Blended addition and integrating the upstream change around it — same
strategy as a full merge conflict. See "Conflict-Prone Files" above.
