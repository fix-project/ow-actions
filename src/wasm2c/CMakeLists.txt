file(GLOB LIB_SOURCES "*.cc")
add_library(wasm-to-c-lib STATIC ${LIB_SOURCES})
