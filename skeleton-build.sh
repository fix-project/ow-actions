#!/bin/bash

pushd dockers/skeleton/
docker build -f Dockerfile -t yhdengh/fixpoint-benchmark:skeleton .
popd
