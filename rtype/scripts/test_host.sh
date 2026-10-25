#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
if [ -f roms/game.sms ]; then
  cc -std=gnu11 -O2 -fno-common -Ilib/smsplus lib/smsplus/*.c test/core.c -lm -o .build/core-test
  .build/core-test roms/game.sms
else
  echo 'SKIP optional real-cartridge test: no local roms/game.sms supplied'
fi
c++ -std=c++17 -O2 -Isrc test/controls.cpp -o .build/controls-test
.build/controls-test
c++ -std=c++17 -O2 -Isrc test/scaling.cpp -o .build/scaling-test
.build/scaling-test
c++ -std=c++17 -O2 -pthread -Isrc test/mailbox.cpp -o .build/mailbox-test
.build/mailbox-test
c++ -std=c++17 -O2 -Isrc test/library.cpp -o .build/library-test
.build/library-test
mkdir -p .build/core-objects
for source in lib/smsplus/*.c; do
  cc -std=gnu11 -O2 -fno-common -Ilib/smsplus -c "$source" -o ".build/core-objects/$(basename "$source" .c).o"
done
c++ -std=c++17 -O2 -Itest/stubs -Isrc -Ilib/smsplus src/Emulator.cpp test/rom_switch.cpp .build/core-objects/*.o -lm -o .build/switch-test
.build/switch-test
c++ -std=c++17 -O2 -Isrc test/rom_reader.cpp -o .build/reader-test
.build/reader-test
