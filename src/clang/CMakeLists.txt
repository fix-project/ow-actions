file(GLOB LIB_SOURCES "*.cc")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(clang-lib STATIC ${LIB_SOURCES})
add_dependencies(clang-lib intrinsics_gen)
add_dependencies(clang-lib clang-tablegen-targets clang-resource-headers)
