#!/bin/bash
mkdir docker-build 
cd docker-build 
cmake ../
make -j${nproc}
