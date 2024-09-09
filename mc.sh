#!/usr/bin/env bash
SRC_REL=`dirname $0`
SRC=`realpath ${SRC_REL}`
SRC=${SRC}/build/llvm-project/llvm/lib/clang/19/include
echo ${SRC}

echo "mc mb myminio/system-deps"
mc mb myminio/system-deps
mc mb myminio/clang-deps

list=(`cat src/clang/depfile.hh | grep -Eo "\"(.)+\"" | awk '{print substr($0, 2, length($0)-2)}'`)
echo ${list[1]}

prefix=/usr/include
for file in "${list[@]}"
do
  if [[ $file == "$prefix"* ]]
  then
    echo "mc put ${file} myminio/system-deps${file}"
    mc put ${file} myminio/system-deps${file}
    
  else
    basename=`basename $file`
    echo "mc put ${SRC}/$basename myminio/clang-deps${file}"
    mc put ${SRC}/$basename myminio/clang-deps${file}
  fi
done
