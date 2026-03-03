# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Minimal Epoxy shim library for Emscripten builds.
# Provides stub implementations of epoxy utility functions
# and redirects <epoxy/gl.h> to Emscripten's GLES3 headers.

add_library(bf_emscripten_epoxy_shim STATIC epoxy_shim.c)
target_include_directories(bf_emscripten_epoxy_shim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
