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

# ── Python executable (needed by build-time scripts) ──────────────
# platform_unix.cmake normally finds this, but we skip that file on
# Emscripten, so locate it here.
find_program(PYTHON_EXECUTABLE "python3")
if(NOT PYTHON_EXECUTABLE)
  find_program(PYTHON_EXECUTABLE "python")
endif()
if(NOT PYTHON_EXECUTABLE)
  message(FATAL_ERROR "python3 not found — required for build-time code generation")
endif()

# ── Compiler flags common to all targets ──────────────────────────

# SDL2 is built into Emscripten — no external library needed.
string(APPEND CMAKE_C_FLAGS   " -s USE_SDL=2")
string(APPEND CMAKE_CXX_FLAGS " -s USE_SDL=2")

# Suppress warnings that Emscripten's clang treats as errors.
# Blender's codebase has many implicit sign conversions (20,000+ in clang-cl)
# that are intentional. Emscripten's clang enables -Wsign-conversion by default
# and promotes it to an error, which breaks the build.
string(APPEND CMAKE_C_FLAGS   " -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-implicit-int-conversion")
string(APPEND CMAKE_CXX_FLAGS " -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-implicit-int-conversion")

# Threading: -pthread is NOT set globally in compiler flags.
#
# In Emscripten, -pthread at compile time enables shared-memory codegen
# (TLS segments, -matomics, -mbulk-memory, __EMSCRIPTEN_PTHREADS__).
# When this flag is global in CMAKE_C/CXX_FLAGS, it applies to ALL
# translation units — including those linked into build tools that run
# under Node.js with -sUSE_PTHREADS=0. The resulting compile/link
# mismatch (shared-memory object code + non-shared-memory runtime)
# corrupts the WASM data segment, garbling string constants like RNA
# property identifiers.
#
# Instead, -pthread is applied per-target only to the main blender
# executable in source/creator/CMakeLists.txt. Library code compiled
# without -pthread still works correctly: std::atomic operations are
# sequentially consistent by default in single-threaded WASM, and the
# linker resolves the threading ABI at link time.

# ── Linker flags (Emscripten-specific) ───────────────────────────
#
# IMPORTANT: Browser-specific link flags (WebGL, large memory, MODULARIZE,
# filesystem, etc.) are NOT placed in CMAKE_EXE_LINKER_FLAGS because that
# variable applies to ALL executables — including build-time tools
# (makesdna, makesrna, datatoc, shader_tool) that run under Node.js.
# When these tools inherit browser flags (512 MB INITIAL_MEMORY, WebGL2,
# MODULARIZE=1, etc.) it causes data corruption and build failures.
#
# Instead, browser flags are stored in EMSCRIPTEN_BROWSER_LINK_FLAGS and
# applied only to the main blender executable in source/creator/CMakeLists.txt.

# ── Build-tool link flags (applied globally) ────────────────────
# These are safe for ALL executables including build tools.
# Kept intentionally minimal: just WASM output and basic assertions.
string(APPEND CMAKE_EXE_LINKER_FLAGS " -sWASM=1 -sASSERTIONS=1")

# ── Browser-target link flags (applied per-target) ──────────────
# These are collected as a CMake list and applied via target_link_options()
# in source/creator/CMakeLists.txt.
set(EMSCRIPTEN_BROWSER_LINK_FLAGS "")

# WebGL2 (OpenGL ES 3.0).
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS
  -sUSE_SDL=2
  -sUSE_WEBGL2=1
  -sFULL_ES3=1
  -sMIN_WEBGL_VERSION=2
  -sMAX_WEBGL_VERSION=2
)

# NOTE: Threading flags (-pthread, PTHREAD_POOL_SIZE) are intentionally
# NOT in the global linker flags. Emscripten's driver processes -pthread
# AFTER -s settings, so it would override -sUSE_PTHREADS=0 on build tools
# (makesdna, makesrna) that must run single-threaded under Node.js.
# Threading link flags are applied per-target in source/creator/CMakeLists.txt.

# Memory configuration.
# Start at 512 MB, allow growth up to 4 GB.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS
  -sINITIAL_MEMORY=536870912
  -sALLOW_MEMORY_GROWTH=1
  -sMAXIMUM_MEMORY=4294967296
)

# Stack size — Blender's deep call stacks need more than the 64 KB default.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS -sSTACK_SIZE=5242880)

# Filesystem: MEMFS by default, IDBFS available for persistence.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS -sFORCE_FILESYSTEM=1 -lidbfs.js)

# Export the main function for the Emscripten runtime.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS
  "-sEXPORTED_RUNTIME_METHODS=[\"callMain\",\"ccall\",\"cwrap\"]"
)

# Produce modularized output so the web shell can control initialization.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS
  -sMODULARIZE=1
  "-sEXPORT_NAME=\"BlendedModule\""
)

# Disable exit-runtime so the module stays alive in the browser.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS -sEXIT_RUNTIME=0)

# GL debug assertions for development.
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS -sGL_ASSERTIONS=1)

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
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(fmt)

# Eigen 3.4.0 — header-only linear algebra library.
# We only download + create the target manually (DO NOT run Eigen's
# CMakeLists.txt — it pulls in BLAS/LAPACK and a Fortran compiler).
FetchContent_Declare(
  eigen
  URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(eigen)
add_library(Eigen3_HeaderOnly INTERFACE)
target_include_directories(Eigen3_HeaderOnly SYSTEM INTERFACE "${eigen_SOURCE_DIR}")
add_library(Eigen3::Eigen ALIAS Eigen3_HeaderOnly)

# Zstd 1.5.6 — compression library used for .blend file I/O.
FetchContent_Declare(
  zstd
  URL https://github.com/facebook/zstd/releases/download/v1.5.6/zstd-1.5.6.tar.gz
  SOURCE_SUBDIR build/cmake
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
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

# ── Build-time environment ────────────────────────────────────────
# PLATFORM_ENV_BUILD is used by macros.cmake (e.g., msgfmt_simple) to
# set environment variables when running build-time tools. On Emscripten
# we don't need library paths, but the variable must be defined.
set(PLATFORM_ENV_BUILD "_DUMMY_ENV_VAR_=1")

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
