file(GLOB LIB_SOURCES "*.cc")
add_library(lld-lib STATIC ${LIB_SOURCES})
add_dependencies(lld-lib intrinsics_gen)
add_dependencies(lld-lib ${LLD_LIBS} ${LLVM_LIBS})
