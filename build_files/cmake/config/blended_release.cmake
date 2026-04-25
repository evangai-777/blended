# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Blended CI release config.
#
# Inherits blender_release.cmake (all features ON) then overrides options
# that are either locked cuts per the BLENDED design doc or unsuitable for
# GitHub Actions CI runners.
#
# Usage:
#   cmake -C build_files/cmake/config/blended_release.cmake .
#
# Cycles GPU rendering still works — kernels are compiled at runtime on the
# user's machine the first time GPU rendering is used.

include("${CMAKE_CURRENT_LIST_DIR}/blender_release.cmake")

# -----------------------------------------------------------------------------
# GPU compute kernel pre-compilation — OFF for CI
#
# CUDA / HIP / OneAPI binary pre-compilation adds ~2 hours to the build and
# requires GPU SDKs that are not on GitHub Actions runners. Runtime kernel
# compilation covers all the same hardware; end users see a one-time compile
# delay the first time they use GPU rendering, not a missing feature.

set(WITH_CYCLES_CUDA_BINARIES   OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_HIP_BINARIES    OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_ONEAPI_BINARIES OFF CACHE BOOL "" FORCE)

# -----------------------------------------------------------------------------
# Locked cuts from BLENDED.md §10 Bucket 6 (Fossils)
#
# FreestyleLineStyle (ID_LS): niche NPR renderer. NPR in Blended is handled
# via shader nodes / Grease Pencil. Locked cut.

set(WITH_FREESTYLE OFF CACHE BOOL "" FORCE)

# -----------------------------------------------------------------------------
# Format cuts — pending final decisions (BLENDED.md §5)
#
# These are noted here for visibility. The BLENDED doc marks per-format
# decisions as OPEN; don't flip them OFF until each is individually confirmed.
#
# Candidates when ready:
#   set(WITH_IMAGE_CINEON    OFF CACHE BOOL "" FORCE)  # §5 Group 3: drop DPX/Cineon
#   set(WITH_IMAGE_OPENJPEG  OFF CACHE BOOL "" FORCE)  # §5 Group 3: drop JP2
#   set(WITH_IO_PLY          OFF CACHE BOOL "" FORCE)  # §5 Group 2: drop PLY
#   set(WITH_IO_STL          OFF CACHE BOOL "" FORCE)  # §5 Group 2: drop STL
