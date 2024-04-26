cmake_minimum_required(VERSION 3.10)

project(boostrap_minio LANGUAGES C CXX)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

include(etc/build_type.cmake)

find_package(ZLIB REQUIRED)
SET(CMAKE_PREFIX_PATH /musl)

# aws cpp sdk
set(SERVICE_COMPONENTS s3)
find_package(AWSSDK REQUIRED COMPONENTS ${SERVICE_COMPONENTS})

# json parsing
include_directories(json/single_include)

# LLVM setups
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE BOOL "" FORCE)
set(LLVM_ENABLE_PROJECTS "clang;lld" CACHE STRING "" FORCE)
set(LLVM_TARGET_ARCH "x86_64" CACHE STRING "" FORCE)
set(LLVM_DEFAULT_TARGET_TRIPLE "x86_64-pc-linux-gnu" CACHE STRING "" FORCE)
set(LLVM_TARGETS_TO_BUILD "X86" CACHE STRING "" FORCE)
set(LLVM_ENABLE_THREADS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_UTILS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LIBCLANG_BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(llvm-project/llvm EXCLUDE_FROM_ALL)

include_directories(llvm-project/llvm/include)
include_directories(${CMAKE_BINARY_DIR}/llvm-project/llvm/include)
include_directories(llvm-project/clang/include)
include_directories(${CMAKE_BINARY_DIR}/llvm-project/llvm/tools/clang/include)
include_directories(llvm-project/lld/include)

set(CLANG_LIBS clangFrontend clangDriver clangSerialization clangParse clangCodeGen clangSema clangAnalysis clangEdit clangAST clangLex clangBasic)
set(LLD_LIBS lldCommon lldELF)
set(LLVM_LIBS LLVMX86AsmParser LLVMX86CodeGen LLVMX86Desc LLVMX86Disassembler LLVMX86Info LLVMX86TargetMCA)

# wabt setup
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_LIBWASM OFF CACHE BOOL "" FORCE)
include_directories(wabt/include)
add_subdirectory(wabt EXCLUDE_FROM_ALL)
include_directories(${CMAKE_BINARY_DIR}/wabt/include)

add_subdirectory(src)
