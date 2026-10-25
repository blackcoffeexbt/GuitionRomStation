# Scope and next milestones

## Phase 1: playable meadow

Complete the movement/rendering acceptance gate before adding farming: verified landscape panel, touch gestures, a scrolling isometric world, animated player, obstacle routing and entities sorted by ground contact. Benchmarked on the user's physical board using USB logs and the MacBook webcam. Quiet I²S sounds and a dismissible guide are included.

There is no combat, penalty, network dependency or countdown. The diagnostic walking tour is opt-in over serial and turns off on a real tap. Source, generated art and original PNGs are tracked separately. `cc2b67d` is the first buildable rollback point; later commits contain hardware performance work.

## Phase 2: farming and persistence — implemented

Inventory, Plants and SaveCodec are portable simulation modules; SaveManager handles asynchronous filesystem writes. Gathering, cultivation, eight plant species, watering, growth, harvesting/regrowth and seed returns are present. Full-state versioned saves use checksummed dual slots, with host validation and on-device recovery tests. Save status and Save & rest are visible. No trusted RTC exists, so growth resumes from saved simulation time after power-off. See PHASE2.md and SAVES.md.

## Phase 3: editable settlement

Add Building, collision-aware placement previews, reversible removal/repositioning, fences, beds, benches and sheds. Persist modifications in both save slots. SD may become an optional asset/save medium, but standalone operation should remain available using flash.

## Phase 4: polish

Replace placeholders, expand animations, wildlife and ambient sounds; investigate 30 FPS, tighter memory use, and a more extensive worst-case scene benchmark. The Phase 1 measured performance is an initial floor, not a promise of 30 FPS in every future scene.
