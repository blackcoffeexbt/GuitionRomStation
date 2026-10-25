# Save design for Phase 2 (not implemented)

Phase 1 mounts LittleFS; game state resets at reboot. No save format is active yet. The SD card is not read, formatted or written. There is no real-time offline growth in Phase 1.

Reserved partition: LittleFS, offset `0xc10000`, length `0x3e0000` (3,968 KiB). PSRAM contains runtime state only. Two 6 MiB application partitions leave room for expanded assets and later OTA tooling; OTA is not implemented.

## Proposed version 1 format

Define serialization explicitly as little-endian fields; never dump C++ structs with padding. Header: four-byte magic `LWLD`, u16 format version, u16 header length, u32 payload length, u32 sequence number, u32 CRC32 of payload. Payload: world seed and generation version, terrain edits, stable entity IDs and definitions, player tile/position, inventory, growth state, simulation clock, unlocks and optional trusted UTC timestamp. Validate every count, enum, coordinate, length and checksum before allocating or applying state.

Use alternating complete save slots A/B. Write the older slot to a temporary file; flush/close, read back and validate it, then rename it over that slot. Keep the previous valid slot intact. On load validate both and choose the latest valid sequence. A torn write or corrupted slot must fall back to the other slot; if neither validates, start a new world with a visible recovery notice. A complete save fits comfortably in the reserved partition.

Debounce meaningful mutations for several seconds, rate-limit flash writes and avoid movement-per-frame saves. Explicit Save/Exit should finish any pending save; power-loss recovery uses the last complete slot. Schedule filesystem writes outside critical rendering work and report duration over serial.

Growth uses elapsed simulation seconds. Across reboot, resume saved game time unless an external RTC or otherwise trustworthy timestamp exists. If trusted UTC is available, require monotonic non-negative elapsed time and cap progression (e.g. 72 hours). No Wi-Fi requirement or invented boot-time timestamp.

Host tests for serialization, migration, bounds validation, CRC failures and interrupted writes belong in Phase 2 alongside the actual implementation. They are intentionally not placeholder tests in Phase 1.
