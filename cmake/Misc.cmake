# --------------------------------------------------------------------------------
#                            Misc (no change needed).
# --------------------------------------------------------------------------------
# Have CMake parse the config file, generating the config header, with
# correct definitions. Here only used to make version number available to
# the source code. Include "config.h" (no .in suffix) in the source.
configure_file (
  "${PROJECT_SOURCE_DIR}/include/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)
# add the binary tree to the search path for include files
# so that we will find config.h
include_directories("${PROJECT_BINARY_DIR}")

# Ask CMake to output a compile_commands.json file for use with things like Vim YCM.
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
