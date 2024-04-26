FROM openwhisk/dockerskeleton 

RUN apk add git g++ make cmake zlib-dev openssl-dev curl-dev perl nghttp2-static build-base

RUN cd / && \
    git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp &&  \
    mkdir -p sdkbuild && cd sdkbuild && cmake -DBUILD_ONLY="s3" -DCMAKE_INSTALL_PREFIX=/musl \
    -DBUILD_SHARED_LIBS=OFF -DENABLE_TESTING=OFF -DFORCE_SHARED_CRT=OFF \
    ../aws-sdk-cpp && make install && cd ..

RUN cd / && \
    rm -rf aws-sdk-cpp && \
    rm -rf sdkbuild
