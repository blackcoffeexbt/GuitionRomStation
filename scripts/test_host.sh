#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
c++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc -Iassets/generated \
 src/core/*.cpp src/render/*.cpp test/host.cpp -o .build/host-tests
.build/host-tests .build/preview.ppm
