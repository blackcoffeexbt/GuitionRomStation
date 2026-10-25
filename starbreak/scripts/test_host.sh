#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p .build
c++ -std=c++17 -O2 -Wall -Wextra -Isrc test/input.cpp -o .build/input-tests
.build/input-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc test/collision.cpp -o .build/collision-tests
.build/collision-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc src/core/Game.cpp test/game.cpp -o .build/game-tests
.build/game-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc src/core/Game.cpp test/timing.cpp -o .build/timing-tests
.build/timing-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc src/core/Game.cpp test/allocation.cpp -o .build/allocation-tests
.build/allocation-tests
c++ -std=c++17 -O2 -Wall -Wextra -Itest/stubs -Isrc -Iassets/generated src/core/Game.cpp src/Renderer.cpp test/render.cpp -o .build/render-tests
.build/render-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc src/core/Game.cpp test/sectors.cpp -o .build/sector-tests
.build/sector-tests
c++ -std=c++17 -O2 -Wall -Wextra -Isrc src/core/Game.cpp test/replay.cpp -o .build/replay-tests
.build/replay-tests
