FROM openwhisk/dockerskeleton

# Copy c-to-elf 
COPY c-to-elf-minio /action/exec

# Copy system headers
COPY usr/include /action/usr/include/

# copy clang headers
COPY  clang-include     /action/usr/lib/llvm-16/lib/clang/16.0.0/include/
