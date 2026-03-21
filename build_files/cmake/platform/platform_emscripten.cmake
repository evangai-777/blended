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
# -Wno-c++11-narrowing: Blender uses brace-initialized lists with implicit
# narrowing conversions (uchar→char, int→float, etc.) throughout the codebase.
# These are safe and intentional but Emscripten's clang treats them as errors.
string(APPEND CMAKE_C_FLAGS   " -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-implicit-int-conversion -Wno-c++11-narrowing")
string(APPEND CMAKE_CXX_FLAGS " -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-implicit-int-conversion -Wno-c++11-narrowing")

# Threading: -pthread is NOT set globally in compiler flags because it
# also defines __EMSCRIPTEN_PTHREADS__ and enables TLS, which causes
# compile/link mismatches for build tools (makesdna, makesrna, etc.)
# that run under Node.js with -sUSE_PTHREADS=0.
#
# CRITICAL: -matomics and -mbulk-memory must ALSO NOT be set globally.
# Build tools link against library targets (bf_dna, bf_dna_blenlib, etc.)
# that are created with blender_add_lib — the same function used for all
# Blender libraries. When ANY object linked into a non-shared-memory
# binary was compiled with -matomics, the WASM data-segment layout is
# corrupted, garbling string constants (e.g. RNA property identifiers
# become "studi3<garbage>ght_r<garbage>ate_z" instead of
# "studiolight_rotate_z"). Per-target overrides (-mno-atomics) do NOT
# help because the library .a objects are already compiled with atomics.
#
# Instead, -pthread (which implies -matomics/-mbulk-memory) is applied
# ONLY to the main blender executable in source/creator/CMakeLists.txt.
# Library objects compiled without atomics are accepted at link time via
# -Wl,--no-check-features on the blender target.

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

# ── Reusable function for build-tool targets ─────────────────────
# Build tools (makesrna, makesdna, datatoc, shader_tool, msgfmt,
# zstd_compress) run under Node.js at build time, not in a browser.
# This function applies the standard Node.js flags consistently.
#
# Usage:
#   emscripten_build_tool_flags(<target>)              — basic flags
#   emscripten_build_tool_flags(<target> STACK_SIZE 8388608) — with stack config
#
function(emscripten_build_tool_flags target)
  cmake_parse_arguments(PARSE_ARGV 1 _EBTF "" "STACK_SIZE" "")

  target_compile_options(${target} PRIVATE -sUSE_PTHREADS=0)
  target_link_options(${target} PRIVATE
    -sNODERAWFS=1
    -sENVIRONMENT=node
    -sSINGLE_FILE=1
    -sUSE_PTHREADS=0
    -sSHARED_MEMORY=0
    -sALLOW_MEMORY_GROWTH=1
    -sEXIT_RUNTIME=1
  )

  if(_EBTF_STACK_SIZE)
    target_link_options(${target} PRIVATE
      -sSTACK_SIZE=${_EBTF_STACK_SIZE}
      -sSTACK_OVERFLOW_CHECK=2
    )
  endif()
endfunction()

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
  "-sEXPORTED_RUNTIME_METHODS=[\"callMain\",\"ccall\",\"cwrap\",\"FS\",\"ENV\"]"
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

# Skip wasm-opt post-link optimization.
# Binaryen's wasm-opt -O2 on Blender's ~50 MB WASM binary requires more
# RAM than GitHub Actions runners (7 GB) or typical dev machines provide.
# The process gets OOM-killed (SIGKILL -9), which prevents em++ from
# emitting the .js glue file — making the build appear to fail even though
# all 5000+ compilation targets succeed.
#
# Emscripten runs wasm-opt when OPT_LEVEL >= 2 (i.e. -O2 and above).
# We force the LINK step to -O1 so that wasm-opt is skipped entirely.
# Object files are still compiled at whatever CMAKE_BUILD_TYPE dictates
# (-O2 for Release), so codegen quality is unaffected. Only the
# post-link Binaryen passes are skipped.
#
# For release builds with sufficient memory (16+ GB), run wasm-opt
# separately:  wasm-opt -O2 bin/blender.wasm -o bin/blender.wasm
list(APPEND EMSCRIPTEN_BROWSER_LINK_FLAGS -O1)

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

set(WITH_OPENIMAGEIO    OFF CACHE BOOL "" FORCE)  # OpenImageIO not available on Emscripten
set(WITH_IMAGE_OPENJPEG OFF CACHE BOOL "" FORCE)  # OpenJPEG not available on Emscripten
set(WITH_IMAGE_CINEON   OFF CACHE BOOL "" FORCE)  # DPX format requires OpenImageIO
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
