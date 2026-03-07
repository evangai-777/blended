# SPDX-FileCopyrightText: 2023 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later


# -----------------------------------------------------------------------------
# Build `shader_tool` executable.
set(SRC
  attribute.cc
  enum.cc
  flow_control.cc
  function.cc
  intermediate.cc
  metadata.cc
  namespace.cc
  processor.cc
  resource_table.cc
  shader_tool.cc
  string.cc
  struct.cc
  template.cc
  union.cc

  intermediate.hh
  processor.hh
  scope.hh
  token_stream.hh
  token.hh

  lexit/lexit.cc
)

if(WITH_GPU_SHADER_ASSERT)
  add_definitions(-DWITH_GPU_SHADER_ASSERT)
endif()

# `SRC_DNA_INC` is defined in the parent directory.
add_executable(shader_tool ${SRC})
optimize_debug_target(shader_tool)

if(EMSCRIPTEN)
  # Build tools run under Node.js at build time, not in a browser.
  target_compile_options(shader_tool PRIVATE -sUSE_PTHREADS=0)
  target_link_options(shader_tool PRIVATE
    -sNODERAWFS=1         # Direct filesystem access under Node.js
    -sENVIRONMENT=node    # Target Node.js runtime only
    -sSINGLE_FILE=1       # Embed .wasm in .js for simpler execution
    -sUSE_PTHREADS=0      # No SharedArrayBuffer
    -sSHARED_MEMORY=0     # No shared memory
    -sSTACK_SIZE=8388608  # 8 MB stack
    -sSTACK_OVERFLOW_CHECK=2
    -sEXIT_RUNTIME=1      # Flush output on exit
  )
endif()
