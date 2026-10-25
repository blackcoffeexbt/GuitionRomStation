#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
c++ -std=c++17 -O2 -Wall -Wextra -Isrc test/input.cpp -o .build/input-tests
.build/input-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc test/collision.cpp -o .build/collision-tests
.build/collision-tests
