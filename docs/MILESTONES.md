# Scope and next milestones

## Phase 1: playable meadow

Complete the movement/rendering acceptance gate before adding farming: verified landscape panel, touch gestures, a scrolling isometric world, animated player, obstacle routing and entities sorted by ground contact. Benchmarked on the user's physical board using USB logs and the MacBook webcam. Quiet I²S sounds and a dismissible guide are included.

There is no combat, penalty, network dependency or countdown. The diagnostic walking tour is opt-in over serial and turns off on a real tap. Source, generated art and original PNGs are tracked separately. `cc2b67d` is the first buildable rollback point; later commits contain hardware performance work.

## Phase 2: farming and persistence

Add Inventory, Plants and SaveManager as simulation modules using stable, data-driven item/plant definitions. Collect resources, cultivate, plant, advance growth using simulation seconds, harvest, debounce saves and validate recovery. Add meaningful host serialization tests with corrupt/truncated files and migration cases. Explicitly expose save status before offering exit. Test save → reboot → reload on device. No trusted RTC exists in Phase 1, so never assume real-world time progressed during power-off.

## Phase 3: editable settlement

Add Building, collision-aware placement previews, reversible removal/repositioning, fences, beds, benches and sheds. Persist modifications in both save slots. SD may become an optional asset/save medium, but standalone operation should remain available using flash.

## Phase 4: polish

Replace placeholders, expand animations, wildlife and ambient sounds; investigate 30 FPS, tighter memory use, and a more extensive worst-case scene benchmark. The Phase 1 measured performance is an initial floor, not a promise of 30 FPS in every future scene.
