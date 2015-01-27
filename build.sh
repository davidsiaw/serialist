#!/bin/bash

echo "Building coco/R"
pushd coco-r-cpp
make
popd

echo "Building grammar"
pushd src
../coco-r-cpp/Coco serialist.atg
popd

echo "Building serialist"
autoreconf --force --install
./configure
make

