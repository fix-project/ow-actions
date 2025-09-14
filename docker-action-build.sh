#!/bin/bash

pushd dockers/ctoelf-action/
./setup.sh
docker build -f Dockerfile -t yhdengh/fixpoint-benchmark:ctoelf .
popd

pushd dockers/link-elfs/
./setup.sh
docker build -f Dockerfile -t yhdengh/fixpoint-benchmark:linkelfs .
popd
