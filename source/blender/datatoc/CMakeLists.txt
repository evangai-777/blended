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
  target_link_options(datatoc PRIVATE
    -sNODERAWFS=1 -sMODULARIZE=0 -sENVIRONMENT=node -sSINGLE_FILE=1
    -sUSE_PTHREADS=0 -sSHARED_MEMORY=0
  )
endif()
