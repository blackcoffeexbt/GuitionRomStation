# Persistent saves — format 1

Phase 2 saves to **LittleFS** in the partition named `littlefs`, offset `0xc10000`, size `0x3e0000`. PSRAM is runtime memory only. SD is not used. Normal application flashing retains these files; erasing flash, uploading a filesystem image or changing the layout does not.

## Recovery protocol

Files: `/wilds-a.sav`, `/wilds-b.sav`, `/wilds.tmp`.

1. Capture a consistent snapshot and encode it in a bounded 64 KiB PSRAM buffer on the simulation thread.
2. A background worker writes `/wilds.tmp` in 512-byte chunks, yielding between chunks.
3. Flush/close; read back and compare every byte, including the checksummed header and payload.
4. Remove only the **older** A/B slot if it exists, then rename the temporary file to that slot. The latest valid slot remains untouched throughout.
5. Publish success to the main thread using an atomic completion flag. Only then show Saved, or pause for Save & rest.

On boot, validate each slot independently and select the latest valid 32-bit sequence using wrap-aware comparison. A bad newer slot falls back to the older valid slot and shows a recovery notice. Incomplete temporary files are ignored. If neither slot validates, start a fresh garden with a visible notice. Failed writes retain the previous slot and retry after a delay. Failed mounts **never auto-format** the filesystem and clearly report that saving is unavailable.

Autosaves debounce meaningful changes for 2.5 seconds; active simulation gets a checkpoint every 30 seconds. Movement marks a change at reached tiles, not every frame. No gameplay state is read from the filesystem worker. Encoding and filesystem transfer durations are distinct diagnostics. Sudden power loss can lose mutations since the last completed save, but should not destroy both completed slots.

## Binary layout

All integer fields are explicitly little-endian, floats are IEEE-754 binary32, doubles binary64. There are no raw C++ struct dumps or padding bytes. Version identifies the schema; unsupported versions are rejected. The full world is stored, so existing farms do not depend on replaying a particular map generator.

Header (24 bytes):

| Offset | Size | Field |
| --- | --- | --- |
| 0 | 4 | ASCII `LWLD` |
| 4 | 2 | Format version, currently 1 |
| 6 | 2 | Header length, 24 |
| 8 | 4 | Exact payload length |
| 12 | 4 | Sequence number |
| 16 | 4 | Payload CRC32, polynomial `0xedb88320` |
| 20 | 4 | CRC32 of header bytes 0–19 |

Payload, in order:

- u16 map side (64), f64 simulation seconds, f32 day length, u32 unlock flags, i64 trusted UTC or zero.
- i16 player tile X/Y, u8 facing, u8 selected seed, u8 mute flag.
- Seventeen u16 inventory quantities in Item enum order.
- 4,096 tiles: u8 terrain, u8 visual variant, i16 object reference (`-1` for none).
- u16 object count; each object: i16 X/Y, u8 kind, u8 variant, u8 active flag, f64 resource-regrowth deadline.
- u16 active plant count; each: i16 X/Y, u8 species, f32 growth seconds, f32 remaining watering seconds.

Current test saves are about 24 KiB. Limits: 700 objects, 128 planted entities, inventory stacks 0–9,999, eight species. Terrain, object and item identifiers are declared in the portable core headers. Adding/changing schema or identifier meanings requires a version/migration decision.

Validation checks exact lengths/CRC, supported map/version, finite bounded times, enum ranges, booleans, player walkability, plant count/age/water, duplicate plant tiles, soil occupancy, every object reference, one-to-one normal object footprints and four-tile cottage footprints. No invalid snapshot is applied to the live game.

## Time and player location

The current board build passes no UTC provider. Loading resumes saved simulation time; a power cycle cannot masquerade as real elapsed growth. The portable codec can apply trusted UTC only when both saved/current timestamps are trustworthy and ordered; progression caps at 72 hours. Negative/missing time does not advance growth. Watering boost is limited to the saved remaining amount.

A save records the last **reached tile centre**. A pending route/action is transient and cancels on reboot. This avoids loading halfway through an obstacle boundary without a valid path. Save & rest also cancels movement/actions only after the save is successful.

## Developer recovery checks

The normal `guition` firmware contains no fault-injection commands. For deliberate development testing only:

```sh
pio run -e guition_validation -t upload --upload-port /dev/cu.usbmodem101
python3 scripts/check_save_recovery.py
pio run -e guition -t upload --upload-port /dev/cu.usbmodem101
```

The validation build's guarded `fault 1` preserves a verified backup before corrupting the latest slot and rebooting; `fault 3` restores that exact backup and reboots. `fault 2` writes an incomplete temporary file then reboots, leaving completed slots untouched. A backup already present is never overwritten. The test script checks recovery logs and inventory equality, then the final normal build must be flashed again. This simulates the storage states of interrupted writes; it is not a claim that power was physically disconnected mid-write.
