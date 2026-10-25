# Phase 2 — gathering and farming

Eight selectable seeds: grass, wildflowers, carrots, pumpkins, berry bushes, small trees, apple trees and oaks. All have four visible stages and grow using elapsed simulation time. Watering grants up to a minute of double-speed growth; plants never wither.

Use Hand to gather seed pouches, wood, stones, berries, mushrooms and flowers. Objects are approached along an obstacle-aware route before the interaction happens. Hoe cultivates clear grass/dirt. Seeds opens a selection panel; select a seed then tap soil. Water helps a planted crop. Hand harvests mature crops and returns seeds; perennial species regrow. Bag displays all supplies.

The top save button shows `Save *` when changed, `Saving...` during a write, and `Saved` when complete. Saves debounce meaningful changes and checkpoint simulation progress every 30 seconds. Visit the cottage for Save & rest before powering off. Rest pauses simulation; tap to resume.

Persistent state includes terrain, objects and regrowth deadlines, plants/growth/water, inventory, last reached player tile, selected seed, mute setting, simulation clock, configurable day length and unlock flags. Position reloads at the last reached tile centre rather than halfway through a movement edge.

LittleFS holds two versioned slots. The new farm replaces only the older slot after a complete temporary write and readback; the current valid slot remains recoverable. Mount failures do not auto-format storage. There is no trusted RTC or Wi-Fi time source in this build, so growth resumes from saved simulation time after power-off. The codec supports capped progression if a future trusted UTC provider is added. The SD card remains untouched.

Host checks: `scripts/test_host.sh`. The farming test covers all species, full-stack failure without consuming a crop, actual approach/interaction logic, growth/watering, save round trips, CRC/header corruption, truncated saves, invalid coordinates/counts and clock safety.

Hardware checks and final measurements are recorded in PERFORMANCE.md.
