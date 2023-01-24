# SPDX-License-Identifier: Apache-2.0

if(DEFINED TOOLCHAIN_HOME)
  set(find_program_clang_args PATHS ${TOOLCHAIN_HOME} ${ONEAPI_LLVM_BIN_PATH} NO_DEFAULT_PATH)
  set(find_program_binutils_args PATHS ${TOOLCHAIN_HOME} )
endif()

execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE ONEAPI_VERSION)
string(REGEX REPLACE "[^0-9]*([0-9.]+) .*" "\\1" ONEAPI_VERSION ${ONEAPI_VERSION})

if(ONEAPI_VERSION VERSION_LESS "2023.0.0")
  message(FATAL_ERROR "Unsupported version of the toolchain - need to use 2023.0.0 or newer")
endif()

find_program(CMAKE_AR      llvm-ar      ${find_program_clang_args}   )
if(ONEAPI_VERSION VERSION_LESS_EQUAL "2023.0.0")
  find_program(CMAKE_NM      nm           ${find_program_binutils_args}   )
else()
  find_program(CMAKE_NM      llvm-nm      ${find_program_clang_args}   )
endif()
# In OneApi installation directory on Windows, there is no llvm-objdump
# binary, so would better use objdump from system environment both
# on Linux and Windows.
find_program(CMAKE_OBJDUMP objdump      ${find_program_binutils_args})
find_program(CMAKE_RANLIB  llvm-ranlib  ${find_program_clang_args}   )
find_program(CMAKE_OBJCOPY llvm-objcopy ${find_program_binutils_args})
find_program(CMAKE_READELF readelf      ${find_program_binutils_args})
find_program(CMAKE_STRIP   llvm-strip   ${find_program_binutils_args})

message("AR ${CMAKE_AR}")
message("NM ${CMAKE_NM}")
message("OBJDUMP ${CMAKE_OBJDUMP}")
message("RANLIB ${CMAKE_RANLIB}")
message("OBJCOPY ${CMAKE_OBJCOPY}")
message("OBJCOPY ${CMAKE_OBJCOPY}")
message("STRIP ${CMAKE_STRIP}")

find_program(CMAKE_GDB     gdb-oneapi)

# Use the gnu binutil abstraction
include(${ZEPHYR_BASE}/cmake/bintools/llvm/target_bintools.cmake)

