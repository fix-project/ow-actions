#!/bin/bash
mkdir -p actions
cd actions

zip wasm2c.zip ../docker-build/src/driver/wasm-to-c-minio -j 
printf "@ wasm-to-c-minio\n@=exec\n" | zipnote -w wasm2c.zip

zip link-elfs.zip ../docker-build/src/driver/link-elfs-minio -j 
printf "@ link-elfs-minio\n@=exec\n" | zipnote -w link-elfs.zip
