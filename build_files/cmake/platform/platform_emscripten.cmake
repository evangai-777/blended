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

# ── FetchContent: download mandatory dependencies ────────────────
# These libraries are required by the core build but are normally
# provided by precompiled lib packages. On Emscripten we fetch them.

include(FetchContent)

# fmt 12.1.0 — matches Blender's pinned version.
FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/12.1.0.tar.gz
  URL_HASH SHA256=ea7de4299689e12b6dddd392f9896f08fb0777ac7168897a244a6d6085043fea
)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(fmt)

# Eigen 3.4.0 — header-only linear algebra library.
FetchContent_Declare(
  eigen
  URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(eigen)

# Zstd 1.5.6 — compression library used for .blend file I/O.
FetchContent_Declare(
  zstd
  URL https://github.com/facebook/zstd/releases/download/v1.5.6/zstd-1.5.6.tar.gz
  SOURCE_SUBDIR build/cmake
)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(zstd)

# Point dependency_targets.cmake at the Zstd headers/library.
FetchContent_GetProperties(zstd SOURCE_DIR ZSTD_SOURCE_DIR)
set(ZSTD_INCLUDE_DIRS "${ZSTD_SOURCE_DIR}/lib" CACHE PATH "" FORCE)
set(ZSTD_LIBRARIES libzstd_static CACHE STRING "" FORCE)

# ── Emscripten Ports ────────────────────────────────────────────
# These libraries are fetched and built automatically by Emscripten
# when the USE_* flags are set. No external find_package needed.

string(APPEND CMAKE_C_FLAGS   " -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_LIBJPEG=1 -s USE_FREETYPE=1")
string(APPEND CMAKE_CXX_FLAGS " -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_LIBJPEG=1 -s USE_FREETYPE=1")

# Set CMake variables so dependency_targets.cmake populates the
# INTERFACE targets. Emscripten handles include paths and linking
# automatically via the USE_* flags — empty values are fine here.
set(ZLIB_INCLUDE_DIRS "" CACHE PATH "" FORCE)
set(ZLIB_LIBRARIES "" CACHE STRING "" FORCE)
set(PNG_INCLUDE_DIRS "" CACHE PATH "" FORCE)
set(PNG_LIBRARIES "" CACHE STRING "" FORCE)
set(JPEG_INCLUDE_DIR "" CACHE PATH "" FORCE)
set(JPEG_LIBRARIES "" CACHE STRING "" FORCE)
set(FREETYPE_INCLUDE_DIRS "" CACHE PATH "" FORCE)
set(FREETYPE_LIBRARIES "" CACHE STRING "" FORCE)
set(BROTLI_LIBRARIES "" CACHE STRING "" FORCE)

# ── Epoxy shim ──────────────────────────────────────────────────
# Blender uses libepoxy for GL function loading (<epoxy/gl.h>).
# On Emscripten, GL functions are provided directly by the runtime.
# We build a minimal shim that provides the epoxy headers and stubs
# for epoxy_gl_version(), epoxy_has_gl_extension(), etc.

add_subdirectory(
  ${CMAKE_SOURCE_DIR}/build_files/cmake/platform/emscripten_compat
  ${CMAKE_BINARY_DIR}/emscripten_compat
)
set(Epoxy_INCLUDE_DIRS
  "${CMAKE_SOURCE_DIR}/build_files/cmake/platform/emscripten_compat"
  CACHE PATH "" FORCE
)
set(Epoxy_LIBRARIES bf_emscripten_epoxy_shim CACHE STRING "" FORCE)

# ── Disable features that require OS-level APIs ──────────────────

set(WITH_GHOST_X11      OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_WAYLAND  OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_XDND     OFF CACHE BOOL "" FORCE)
set(WITH_X11_XINPUT     OFF CACHE BOOL "" FORCE)
set(WITH_X11_XFIXES     OFF CACHE BOOL "" FORCE)
set(WITH_GHOST_DEBUG    OFF CACHE BOOL "" FORCE)

# Emscripten's only windowing option is SDL2 — enforce it.
# WITH_SDL must also be ON (CMakeLists.txt gates GHOST_SDL behind it).
set(WITH_SDL            ON  CACHE BOOL "" FORCE)
set(WITH_GHOST_SDL      ON  CACHE BOOL "" FORCE)

# ── Disable subsystems that don't cross-compile to Emscripten ────
# These may not be set by the user's config file, so enforce them here
# as a safety net (blended_wasm.cmake also sets these, but this ensures
# correctness even if someone invokes emcmake without the config).

set(WITH_IMAGE_OPENJPEG OFF CACHE BOOL "" FORCE)  # OpenJPEG not available on Emscripten
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
