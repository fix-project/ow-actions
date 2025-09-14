#!/bin/bash
mkdir -p actions
cd actions

rm bptree-get.zip
zip bptree-get.zip ../docker-build/src/driver/bptree-get-minio -j 
printf "@ bptree-get-minio\n@=exec\n" | zipnote -w bptree-get.zip

rm bptree-get-n.zip
zip bptree-get-n.zip ../docker-build/src/driver/bptree-get-n-minio -j
printf "@ bptree-get-n-minio\n@=exec\n" | zipnote -w bptree-get-n.zip

rm bptree-get-string-key.zip
zip bptree-get-string-key.zip ../docker-build/src/driver/bptree-get-string-key-driver -j
printf "@ bptree-get-string-key-driver\n@=exec\n" | zipnote -w bptree-get-string-key-driver.zip

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

rm add.zip
zip add.zip ../docker-build/src/driver/add -j 
printf "@ add\n@=exec\n" | zipnote -w add.zip

pushd compilepoll
npm install
zip -r compilepoll.zip *
popd

pushd lwvirt 
npm install
zip -r lwvirt.zip *
popd

pushd mapreduce
npm install
zip -r mapreduce.zip *
popd
