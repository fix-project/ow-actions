#!/bin/bash
mkdir -p actions
cd actions

rm wasm2c.zip
zip wasm2c.zip ../docker-build/src/driver/wasm-to-c-minio -j 
printf "@ wasm-to-c-minio\n@=exec\n" | zipnote -w wasm2c.zip

rm link-elfs.zip
zip link-elfs.zip ../docker-build/src/driver/link-elfs-minio -j 
printf "@ link-elfs-minio\n@=exec\n" | zipnote -w link-elfs.zip

rm count-words.zip
zip count-words.zip ../docker-build/src/driver/count-words-minio -j 
printf "@ count-words-minio\n@=exec\n" | zipnote -w count-words.zip

rm merge-counts.zip
zip merge-counts.zip ../docker-build/src/driver/merge-counts-minio -j 
printf "@ merge-counts-minio\n@=exec\n" | zipnote -w merge-counts.zip
