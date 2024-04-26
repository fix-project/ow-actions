file(GLOB LIB_SOURCES "*.cc")

add_custom_command(
  OUTPUT "wasm-rt-content.h"
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/wasm_rt_content.py
          $ENV{HOME}/wasm-toolchain/wabt/wasm2c/wasm-rt.h
          -o wasm-rt-content.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(clang-lib STATIC ${LIB_SOURCES} wasm-rt-content.h)
add_dependencies(clang-lib intrinsics_gen)
add_dependencies(clang-lib clang-tablegen-targets clang-resource-headers)
