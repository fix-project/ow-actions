include_directories(wasm2c)
add_subdirectory(wasm2c)

include_directories(clang)
add_subdirectory(clang)

include_directories(lld)
add_subdirectory(lld)

# Build corresponding driver
add_subdirectory(driver)
