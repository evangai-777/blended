# SPDX-FileCopyrightText: 2025 Blended Authors
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Warning-report infrastructure for tracking and fixing compiler warnings.
#
# Usage (add to your cmake invocation):
#   cmake -DWITH_WARNINGS_REPORT=ON ...
#
# This captures all compiler warnings to a structured log file that can
# be sorted, deduplicated, and triaged. The log is written to:
#   ${CMAKE_BINARY_DIR}/warnings_report.log
#
# After building, run the companion script to produce a summary:
#   python3 build_files/utils/parse_warnings.py build/warnings_report.log

option(WITH_WARNINGS_REPORT
  "Capture compiler warnings to a structured log for triage"
  OFF
)

if(WITH_WARNINGS_REPORT)
  set(WARNINGS_LOG_FILE "${CMAKE_BINARY_DIR}/warnings_report.log")

  # Ensure we get warnings (don't let -Werror kill the build during triage).
  string(REPLACE "-Werror" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  # Enable common warning categories that Blender's code triggers.
  # These are the ones most relevant to the Emscripten/WebGL2 build.
  set(_WARN_FLAGS
    -Wall
    -Wextra
    -Wno-unused-parameter
    -Wsign-conversion
    -Wshorten-64-to-32
    -Wimplicit-int-conversion
    -Wc++11-narrowing
    -Wunused-variable
    -Wunused-function
    -Wshadow
    -Wformat
    -Wmissing-field-initializers
  )
  string(JOIN " " _WARN_FLAGS_STR ${_WARN_FLAGS})
  string(APPEND CMAKE_C_FLAGS " ${_WARN_FLAGS_STR}")
  string(APPEND CMAKE_CXX_FLAGS " ${_WARN_FLAGS_STR}")

  # Write a header to the log file at configure time.
  file(WRITE "${WARNINGS_LOG_FILE}"
    "# Blended Warnings Report\n"
    "# Generated: ${CMAKE_CURRENT_LIST_FILE}\n"
    "# Date: __BUILD_DATE__\n"
    "# Platform: ${CMAKE_SYSTEM_NAME} / ${CMAKE_C_COMPILER_ID}\n"
    "#\n"
    "# Build with: make 2>&1 | tee -a ${WARNINGS_LOG_FILE}\n"
    "# Then parse with: python3 build_files/utils/parse_warnings.py ${WARNINGS_LOG_FILE}\n"
    "#\n"
  )

  message(STATUS "Warnings report enabled: ${WARNINGS_LOG_FILE}")
  message(STATUS "Build with: make 2>&1 | tee -a ${WARNINGS_LOG_FILE}")
endif()
