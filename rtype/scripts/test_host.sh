#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
cc -std=gnu11 -O2 -fno-common -Ilib/smsplus lib/smsplus/*.c test/core.c -lm -o .build/core-test
.build/core-test roms/game.sms
c++ -std=c++17 -O2 -Isrc test/controls.cpp -o .build/controls-test
.build/controls-test
c++ -std=c++17 -O2 -Isrc test/scaling.cpp -o .build/scaling-test
.build/scaling-test
