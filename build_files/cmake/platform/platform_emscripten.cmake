# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Platform configuration for Emscripten (WebAssembly) builds.
#
# Emscripten provides SDL2, OpenGL ES 3.0 (as WebGL2), and pthreads.
# This file sets linker flags, memory configuration, and filesystem
# options for the WASM target.

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "platform_emscripten.cmake included but EMSCRIPTEN is not defined")
endif()

message(STATUS "Configuring for Emscripten/WebAssembly")

# ── Compiler/linker flags common to all targets ──────────────────

# SDL2 is built into Emscripten — no external library needed.
string(APPEND CMAKE_C_FLAGS   " -s USE_SDL=2")
string(APPEND CMAKE_CXX_FLAGS " -s USE_SDL=2")

# Threading via pthreads (mapped to Web Workers + SharedArrayBuffer).
string(APPEND CMAKE_C_FLAGS   " -pthread")
string(APPEND CMAKE_CXX_FLAGS " -pthread")

# ── Linker flags (Emscripten-specific) ───────────────────────────

set(_EMSCRIPTEN_LINK_FLAGS "")

# WebGL2 (OpenGL ES 3.0).
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s USE_SDL=2")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s USE_WEBGL2=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s FULL_ES3=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s MIN_WEBGL_VERSION=2")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s MAX_WEBGL_VERSION=2")

# Threading.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -pthread")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s PTHREAD_POOL_SIZE=4")

# Memory configuration.
# Start at 512 MB, allow growth up to 4 GB.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s INITIAL_MEMORY=536870912")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s ALLOW_MEMORY_GROWTH=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s MAXIMUM_MEMORY=4294967296")

# Stack size — Blender's deep call stacks need more than the 64 KB default.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s STACK_SIZE=5242880")

# Filesystem: MEMFS by default, IDBFS available for persistence.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s FORCE_FILESYSTEM=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -lidbfs.js")

# Export the main function for the Emscripten runtime.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s EXPORTED_RUNTIME_METHODS=[\"callMain\",\"ccall\",\"cwrap\"]")

# Produce .wasm + .js glue.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s WASM=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s MODULARIZE=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s EXPORT_NAME=\"BlendedModule\"")

# Disable exit-runtime so the module stays alive.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s EXIT_RUNTIME=0")

# Useful error messages in development.
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s ASSERTIONS=1")
string(APPEND _EMSCRIPTEN_LINK_FLAGS " -s GL_ASSERTIONS=1")

# Apply to all link types.
string(APPEND CMAKE_EXE_LINKER_FLAGS    " ${_EMSCRIPTEN_LINK_FLAGS}")
string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${_EMSCRIPTEN_LINK_FLAGS}")
string(APPEND CMAKE_MODULE_LINKER_FLAGS " ${_EMSCRIPTEN_LINK_FLAGS}")

unset(_EMSCRIPTEN_LINK_FLAGS)

# ── Skip precompiled library detection ───────────────────────────
# Emscripten provides its own system libraries (SDL2, zlib, libpng, etc.)
# via emscripten-ports. No LIBDIR needed.

set(WITH_LIBS_PRECOMPILED OFF)
unset(LIBDIR)

# ── Disable features that require OS-level APIs ──────────────────

set(WITH_GHOST_X11      OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_WAYLAND  OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_XDND     OFF CACHE BOOL "" FORCE)
set(WITH_X11_XINPUT     OFF CACHE BOOL "" FORCE)
set(WITH_X11_XFIXES     OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_DEBUG    OFF CACHE BOOL "" FORCE)

# Emscripten's only windowing option is SDL2 — enforce it.
set(WITH_GHOST_SDL      ON  CACHE BOOL "" FORCE)

# ── Disable subsystems that don't cross-compile to Emscripten ────
# These may not be set by the user's config file, so enforce them here
# as a safety net (blended_wasm.cmake also sets these, but this ensures
# correctness even if someone invokes emcmake without the config).

set(WITH_GHOST_CSD      OFF CACHE BOOL "" FORCE)  # No client-side decorations in browser
set(WITH_OPENGL_BACKEND  ON CACHE BOOL "" FORCE)   # WebGL2 is the only option
set(WITH_VULKAN_BACKEND OFF CACHE BOOL "" FORCE)
set(WITH_METAL_BACKEND  OFF CACHE BOOL "" FORCE)

# ── SDL2: Emscripten built-in, skip pkg-config detection ─────────
# The normal FindSDL2 path won't work — Emscripten's -s USE_SDL=2
# flag tells emcc to download and build SDL2 from emscripten-ports.
# We just need to tell CMake the include/link is handled.

set(SDL2_FOUND TRUE)
set(SDL2_INCLUDE_DIR "")
set(SDL2_LIBRARY "")
