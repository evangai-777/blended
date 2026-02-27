# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Blended release build configuration.
#
# Based on Blender's blender_release.cmake but disables GPU compute
# kernel pre-compilation (CUDA/HIP/OneAPI binaries). This cuts ~2 hours
# off CI build time. Cycles still renders on GPU — it just compiles
# kernels at runtime on first use.
#
# Usage:
#   cmake -C build_files/cmake/config/blended_release.cmake ..

# ---- Core features (same as upstream release) ----

set(WITH_ALEMBIC             ON  CACHE BOOL "" FORCE)
set(WITH_ASSERT_ABORT        OFF CACHE BOOL "" FORCE)
set(WITH_AUDASPACE           ON  CACHE BOOL "" FORCE)
set(WITH_BUILDINFO           ON  CACHE BOOL "" FORCE)
set(WITH_BULLET              ON  CACHE BOOL "" FORCE)
set(WITH_CODEC_FFMPEG        ON  CACHE BOOL "" FORCE)
set(WITH_CODEC_SNDFILE       ON  CACHE BOOL "" FORCE)
set(WITH_CYCLES              ON  CACHE BOOL "" FORCE)
set(WITH_CYCLES_EMBREE       ON  CACHE BOOL "" FORCE)
set(WITH_CYCLES_OSL          ON  CACHE BOOL "" FORCE)
set(WITH_CYCLES_PATH_GUIDING ON  CACHE BOOL "" FORCE)
set(WITH_DRACO               ON  CACHE BOOL "" FORCE)
set(WITH_FFTW3               ON  CACHE BOOL "" FORCE)
set(WITH_FREESTYLE           ON  CACHE BOOL "" FORCE)
set(WITH_GMP                 ON  CACHE BOOL "" FORCE)
set(WITH_HARU                ON  CACHE BOOL "" FORCE)
set(WITH_IK_ITASC            ON  CACHE BOOL "" FORCE)
set(WITH_IK_SOLVER           ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_CINEON        ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_OPENEXR       ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_OPENJPEG      ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_WEBP          ON  CACHE BOOL "" FORCE)
set(WITH_INPUT_NDOF          ON  CACHE BOOL "" FORCE)
set(WITH_INPUT_IME           ON  CACHE BOOL "" FORCE)
set(WITH_INTERNATIONAL       ON  CACHE BOOL "" FORCE)
set(WITH_IO_FBX              ON  CACHE BOOL "" FORCE)
set(WITH_IO_GREASE_PENCIL    ON  CACHE BOOL "" FORCE)
set(WITH_IO_PLY              ON  CACHE BOOL "" FORCE)
set(WITH_IO_STL              ON  CACHE BOOL "" FORCE)
set(WITH_IO_WAVEFRONT_OBJ    ON  CACHE BOOL "" FORCE)
set(WITH_LIBMV               ON  CACHE BOOL "" FORCE)
set(WITH_LIBMV_SCHUR_SPECIALIZATIONS ON CACHE BOOL "" FORCE)
set(WITH_MANIFOLD            ON  CACHE BOOL "" FORCE)
set(WITH_MOD_FLUID           ON  CACHE BOOL "" FORCE)
set(WITH_MOD_OCEANSIM        ON  CACHE BOOL "" FORCE)
set(WITH_MOD_REMESH          ON  CACHE BOOL "" FORCE)
set(WITH_UV_SLIM             ON  CACHE BOOL "" FORCE)
set(WITH_NANOVDB             ON  CACHE BOOL "" FORCE)
set(WITH_OPENAL              ON  CACHE BOOL "" FORCE)
set(WITH_OPENCOLORIO         ON  CACHE BOOL "" FORCE)
set(WITH_OPENIMAGEDENOISE    ON  CACHE BOOL "" FORCE)
set(WITH_OPENSUBDIV          ON  CACHE BOOL "" FORCE)
set(WITH_OPENVDB             ON  CACHE BOOL "" FORCE)
set(WITH_OPENVDB_BLOSC       ON  CACHE BOOL "" FORCE)
set(WITH_POTRACE             ON  CACHE BOOL "" FORCE)
set(WITH_PUGIXML             ON  CACHE BOOL "" FORCE)
set(WITH_PYTHON_INSTALL      ON  CACHE BOOL "" FORCE)
set(WITH_QUADRIFLOW          ON  CACHE BOOL "" FORCE)
set(WITH_RUBBERBAND          ON  CACHE BOOL "" FORCE)
set(WITH_SDL                 OFF CACHE BOOL "" FORCE)
set(WITH_TBB                 ON  CACHE BOOL "" FORCE)
set(WITH_USD                 ON  CACHE BOOL "" FORCE)
set(WITH_MATERIALX           ON  CACHE BOOL "" FORCE)
set(WITH_HYDRA               ON  CACHE BOOL "" FORCE)
set(WITH_XR_OPENXR           ON  CACHE BOOL "" FORCE)

# ---- Windows-specific ----

if(WIN32)
  set(WITH_WASAPI              ON  CACHE BOOL "" FORCE)
  set(WITH_TBB_MALLOC_PROXY    ON  CACHE BOOL "" FORCE)
endif()

# ---- GPU compute: runtime compilation only ----
# Cycles GPU rendering still works — kernels compile on first render.
# Disabling pre-compiled binaries avoids needing CUDA/HIP/oneAPI SDKs on CI
# and saves ~2 hours of build time.

set(WITH_CYCLES_DEVICE_OPTIX    ON  CACHE BOOL "" FORCE)
set(WITH_CYCLES_CUDA_BINARIES   OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_HIP_BINARIES    OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_ONEAPI_BINARIES OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_DEVICE_HIPRT    OFF CACHE BOOL "" FORCE)
set(WITH_CYCLES_DEVICE_ONEAPI   OFF CACHE BOOL "" FORCE)

# ---- Portable layout (flat folder, no system directories) ----
# User data goes to %APPDATA% by default (no "portable" marker folder).

set(WITH_INSTALL_PORTABLE       ON  CACHE BOOL "" FORCE)
