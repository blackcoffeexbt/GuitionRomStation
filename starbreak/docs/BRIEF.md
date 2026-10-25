# Starbreak delivery contract

480 × 320 landscape, offline, single player, one difficulty. Original crisp pixel spacecraft, layered stars, readable bright enemy shots, restrained explosions. One complete replayable stage of approximately five minutes.

Gameplay: gentle teaching opening, increasingly hard formations, asteroid field and two-phase boss. Enemies: formation scouts, weaving fighters and armoured gunships, each with distinct silhouette and attacks. Forward cannon has three levels and two guaranteed upgrades; damage retains upgrades. Three shields, brief damage invulnerability and one screen-clearing bomb. Score for enemies/boss plus remaining-shield bonus; results show current/best score with retry. Short firing, impact, pickup and explosion sounds, optional mute.

Controller: Xbox Series X/S stick or D-pad, diagonal-normalised movement; hold A fire, B bomb, Menu pause. A confirms menus; B returns. Touch controls menus and pairing only. Screens: title, controller connection, pause, defeat, victory. HUD: score, shields, bomb and boss health. Remember pairing and reconnect after boot; disconnect pauses immediately and clears motion; reconnect requires explicit confirmation.

Architecture: portable input interface and deterministic fixed 60 Hz simulation. Fixed-capacity pools for enemies, player/enemy bullets, pickups and effects; no gameplay allocations. Swept projectile collision. Full-frame display update, budget Bluetooth memory before choosing framebuffer. High score/settings only, separate versioned namespace; preserve farm saves and leave SD alone.

## Gates and acceptance

1. Hardware proof before stage work: pair user's Xbox controller, verify axes/buttons, latency, disconnect/resume and memory with display + Bluetooth + audio.
2. Playable slice: stars, ship, firing, one enemy, collisions, three shields and restart.
3. Full demo: authored five-minute stage, all enemy types, upgrades, bomb, two-phase boss, audio, all screens and saved best score.
4. Polish and validation: repeated physical-board runs and readability/difficulty tuning.

Required evidence:

- Complete title → victory and defeat → retry on the board.
- Reconnect/pause correctness and no movement while disconnected.
- Host checks: collision sweeps, damage cooldown, upgrades retained through damage, bomb consumption, boss transitions and deterministic patterns.
- High score/settings survive reboot; corrupt shooter settings recover safely.
- At least 30 minutes repeated play, no crashes or declining free memory.
- Target 30 FPS; minimum sustained 25 FPS in busiest scripted encounter with Bluetooth and audio active. Synthetic diagnostic stress alone does not satisfy stage acceptance.

Do not mark the hardware gate or final game accepted using a host simulation, unpaired radio, serial-injected gameplay or a screenshot alone.

## Expansion requested after the initial demo

The user requested additional baddies, level-ups, end-of-level bosses and terrain, retaining the compact sprite scale. The campaign now spans three sectors with different scenery and bosses, seven enemy/hazard types, XP-based pilot ranks and collidable canyon/station boundaries. The existing controller, storage isolation and performance gates still apply.
