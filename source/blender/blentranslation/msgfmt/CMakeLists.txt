# SPDX-FileCopyrightText: 2017 Blender Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later

# -----------------------------------------------------------------------------
# Build `msgfmt` executable.

set(INC
)

set(SRC
  msgfmt.cc
)

# Manifest required for selecting comctl32 v6 required by the crash popup.
if(WIN32 AND NOT WITH_WINDOWS_EXTERNAL_MANIFEST)
  LIST(APPEND SRC "${CMAKE_BINARY_DIR}/tests.exe.manifest")
endif()

set(LIB
  PRIVATE bf::blenlib
  PRIVATE bf::intern::guardedalloc
  PRIVATE bf::dependencies::zlib
  ${PLATFORM_LINKLIBS})

add_cc_flags_custom_test(msgfmt)

if(WIN32)
  string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " /nodefaultlib:MSVCRT.lib")
  list(APPEND LIB bf_intern_utfconv)
endif()

add_executable(msgfmt ${SRC})
setup_platform_linker_flags(msgfmt)
blender_target_include_dirs(msgfmt ${INC})
target_link_libraries(msgfmt ${LIB})

if(EMSCRIPTEN)
  # Build tool runs under Node.js at build time, not in a browser.
  target_compile_options(msgfmt PRIVATE -sUSE_PTHREADS=0)
  target_link_options(msgfmt PRIVATE
    -sNODERAWFS=1         # Direct filesystem access under Node.js
    -sENVIRONMENT=node    # Target Node.js runtime only
    -sSINGLE_FILE=1       # Embed .wasm in .js for simpler execution
    -sUSE_PTHREADS=0      # No SharedArrayBuffer
    -sSHARED_MEMORY=0     # No shared memory
    -sALLOW_MEMORY_GROWTH=1  # Let Node.js grow heap as needed
    -sEXIT_RUNTIME=1      # Flush output on exit
  )
endif()

# If using an external manifest, copy it to the output directory after building.
if(WITH_WINDOWS_EXTERNAL_MANIFEST)
  add_custom_command(TARGET msgfmt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/tests.exe.manifest  $<TARGET_FILE_DIR:msgfmt>/$<TARGET_FILE_NAME:msgfmt>.manifest
  )
endif()
