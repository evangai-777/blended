# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Blended WebAssembly build configuration.
#
# Minimal build targeting Emscripten/WebGL2 for browser deployment via
# GitHub Pages.  Uses the SDL2 GHOST backend (Emscripten ships SDL2)
# and OpenGL (mapped to WebGL2 by Emscripten).
#
# Heavy subsystems (Python, Cycles, FFMPEG, audio, physics sims, etc.)
# are disabled to keep the WASM binary size manageable and avoid
# dependencies that don't cross-compile to Emscripten.
#
# Usage:
#   emcmake cmake -C build_files/cmake/config/blended_wasm.cmake ..
#
# See also: REVIEWME.md, build_files/web/WEBASSEMBLY_ROADMAP.md

# ---- Windowing: SDL2 via Emscripten (maps to browser canvas) ----

set(WITH_GHOST_SDL           ON  CACHE BOOL "" FORCE)
set(WITH_HEADLESS            OFF CACHE BOOL "" FORCE)

# ---- GPU: OpenGL only (Emscripten maps GL → WebGL2) ----

set(WITH_OPENGL_BACKEND      ON  CACHE BOOL "" FORCE)
set(WITH_VULKAN_BACKEND      OFF CACHE BOOL "" FORCE)
set(WITH_METAL_BACKEND       OFF CACHE BOOL "" FORCE)

# ---- Disable Python (no CPython in WASM yet) ----

set(WITH_PYTHON              OFF CACHE BOOL "" FORCE)
set(WITH_PYTHON_INSTALL      OFF CACHE BOOL "" FORCE)
set(WITH_PYTHON_MODULE       OFF CACHE BOOL "" FORCE)

# ---- Disable Cycles (needs GPU compute; Eevee works via WebGL) ----

set(WITH_CYCLES              OFF CACHE BOOL "" FORCE)

# ---- Disable audio / video / codecs ----

set(WITH_AUDASPACE           OFF CACHE BOOL "" FORCE)
set(WITH_CODEC_FFMPEG        OFF CACHE BOOL "" FORCE)
set(WITH_CODEC_SNDFILE       OFF CACHE BOOL "" FORCE)
set(WITH_COREAUDIO           OFF CACHE BOOL "" FORCE)
set(WITH_JACK                OFF CACHE BOOL "" FORCE)
set(WITH_OPENAL              OFF CACHE BOOL "" FORCE)
set(WITH_PULSEAUDIO          OFF CACHE BOOL "" FORCE)
set(WITH_SDL                 OFF CACHE BOOL "" FORCE)  # SDL *audio* — not GHOST_SDL
set(WITH_WASAPI              OFF CACHE BOOL "" FORCE)
set(WITH_RUBBERBAND          OFF CACHE BOOL "" FORCE)

# ---- Disable heavy subsystems that don't cross-compile ----

set(WITH_BULLET              OFF CACHE BOOL "" FORCE)
set(WITH_MOD_FLUID           OFF CACHE BOOL "" FORCE)
set(WITH_MOD_OCEANSIM        OFF CACHE BOOL "" FORCE)
set(WITH_LIBMV               OFF CACHE BOOL "" FORCE)
set(WITH_OPENVDB             OFF CACHE BOOL "" FORCE)
set(WITH_OPENVDB_BLOSC       OFF CACHE BOOL "" FORCE)
set(WITH_NANOVDB             OFF CACHE BOOL "" FORCE)
set(WITH_ALEMBIC             OFF CACHE BOOL "" FORCE)
set(WITH_USD                 OFF CACHE BOOL "" FORCE)
set(WITH_MATERIALX           OFF CACHE BOOL "" FORCE)
set(WITH_HYDRA               OFF CACHE BOOL "" FORCE)
set(WITH_XR_OPENXR           OFF CACHE BOOL "" FORCE)
set(WITH_OPENSUBDIV          OFF CACHE BOOL "" FORCE)
set(WITH_OPENCOLORIO         OFF CACHE BOOL "" FORCE)
set(WITH_OPENIMAGEDENOISE    OFF CACHE BOOL "" FORCE)
set(WITH_FFTW3               OFF CACHE BOOL "" FORCE)
set(WITH_GMP                 OFF CACHE BOOL "" FORCE)
set(WITH_HARU                OFF CACHE BOOL "" FORCE)
set(WITH_POTRACE             OFF CACHE BOOL "" FORCE)
set(WITH_QUADRIFLOW          OFF CACHE BOOL "" FORCE)
set(WITH_TBB                 OFF CACHE BOOL "" FORCE)

# ---- Disable features that don't apply in a browser ----

set(WITH_INPUT_NDOF          OFF CACHE BOOL "" FORCE)
set(WITH_INPUT_IME           OFF CACHE BOOL "" FORCE)
set(WITH_INTERNATIONAL       OFF CACHE BOOL "" FORCE)
set(WITH_DRACO               OFF CACHE BOOL "" FORCE)
set(WITH_FREESTYLE           OFF CACHE BOOL "" FORCE)
set(WITH_IK_ITASC            OFF CACHE BOOL "" FORCE)

# ---- Image formats: keep lightweight ones ----

set(WITH_IMAGE_OPENEXR       OFF CACHE BOOL "" FORCE)
set(WITH_IMAGE_OPENJPEG      ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_CINEON        ON  CACHE BOOL "" FORCE)
set(WITH_IMAGE_WEBP          OFF CACHE BOOL "" FORCE)

# ---- I/O: keep basic mesh formats ----

set(WITH_IO_STL              ON  CACHE BOOL "" FORCE)
set(WITH_IO_PLY              ON  CACHE BOOL "" FORCE)
set(WITH_IO_WAVEFRONT_OBJ    ON  CACHE BOOL "" FORCE)
set(WITH_IO_FBX              OFF CACHE BOOL "" FORCE)
set(WITH_IO_GREASE_PENCIL    OFF CACHE BOOL "" FORCE)

# ---- Modifiers: keep basic mesh modifiers ----

set(WITH_MOD_REMESH          ON  CACHE BOOL "" FORCE)
set(WITH_UV_SLIM             ON  CACHE BOOL "" FORCE)
set(WITH_MANIFOLD            OFF CACHE BOOL "" FORCE)
set(WITH_IK_SOLVER           ON  CACHE BOOL "" FORCE)

# ---- Build settings ----

set(WITH_BUILDINFO           ON  CACHE BOOL "" FORCE)
set(WITH_INSTALL_PORTABLE    ON  CACHE BOOL "" FORCE)
set(WITH_COMPILER_SIMD       OFF CACHE BOOL "" FORCE)  # WASM SIMD needs separate flags
set(WITH_ASSERT_ABORT        OFF CACHE BOOL "" FORCE)

# ---- Disable precompiled libraries (Emscripten provides its own) ----

set(WITH_LIBS_PRECOMPILED    OFF CACHE BOOL "" FORCE)
