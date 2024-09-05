set(CMAKE_CXX_STANDARD 23)

add_executable(wasm-to-c-minio "wasm2c-driver.cc")
target_link_libraries(wasm-to-c-minio util)
target_link_libraries(wasm-to-c-minio wasm-to-c-lib wabt)
target_link_libraries(wasm-to-c-minio ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(wasm-to-c-minio -static-libgcc -static-libstdc++)

add_executable(c-to-elf-minio "c-to-elf-driver.cc" "mmap.cc" "file_descriptor.cc")
target_link_libraries(c-to-elf-minio util)
target_link_libraries(c-to-elf-minio clang-lib ${CLANG_LIBS} ${LLVM_LIBS})
target_link_libraries(c-to-elf-minio ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(c-to-elf-minio -static-libgcc -static-libstdc++)

add_executable(link-elfs-minio "link-elfs-driver.cc")
target_link_libraries(link-elfs-minio util)
target_link_libraries(link-elfs-minio lld-lib ${LLD_LIBS} ${LLVM_LIBS})
target_link_libraries(link-elfs-minio ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(link-elfs-minio -static-libgcc -static-libstdc++)
#
#add_executable(count-words-minio "count-words-driver.cc")
#target_link_libraries(count-words-minio ${AWSSDK_LINK_LIBRARIES})
#target_link_libraries(count-words-minio -static-libgcc -static-libstdc++)
#
#add_executable(merge-counts-minio "merge-counts-driver.cc")
#target_link_libraries(merge-counts-minio ${AWSSDK_LINK_LIBRARIES})
#target_link_libraries(merge-counts-minio -static-libgcc -static-libstdc++)

add_executable(bptree-get-minio "bptree-get-driver.cc")
target_link_libraries(bptree-get-minio ${CURL_LIBRARIES})
target_link_libraries(bptree-get-minio ${AWSSDK_LINK_LIBRARIES})
target_link_libraries(bptree-get-minio -static-libgcc -static-libstdc++)
