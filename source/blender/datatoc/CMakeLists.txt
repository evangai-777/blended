# SPDX-FileCopyrightText: 2023 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later


# -----------------------------------------------------------------------------
# Build `datatoc` executable.
set(SRC
  datatoc.cc
)

# `SRC_DNA_INC` is defined in the parent directory.
add_executable(datatoc ${SRC})
optimize_debug_target(datatoc)

if(EMSCRIPTEN)
  # Build tools run under Node.js at build time, not in a browser.
  target_compile_options(datatoc PRIVATE -sUSE_PTHREADS=0)
  target_link_options(datatoc PRIVATE
    -sNODERAWFS=1         # Direct filesystem access under Node.js
    -sENVIRONMENT=node    # Target Node.js runtime only
    -sSINGLE_FILE=1       # Embed .wasm in .js for simpler execution
    -sUSE_PTHREADS=0      # No SharedArrayBuffer
    -sSHARED_MEMORY=0     # No shared memory
    -sEXIT_RUNTIME=1      # Flush output on exit
  )
endif()
