# NPC Pathing NG 2.4.4

## Is EVG Animated Traversal required? No.

**EVG has never been required by this mod.** The plugin's only master is `Skyrim.esm` — there is no hard dependency on EVG or on SkyParkour. EVG's forms are looked up at runtime and quietly skipped when the mod is absent.

As of 2.4.4, **EVG marker traversal is disabled by default**, because it cannot work for NPCs: the engine only lets NPCs enter furniture through AI packages, so activating a marker is rejected for them (confirmed from both SKSE and Papyrus). Leaving it enabled bought nothing and only added marker scans and activation calls. Your own use of EVG markers as the player is completely unaffected.

If you are crashing and suspect EVG, please post a crash log — with 2.4.4 this mod does not touch EVG markers at all unless you deliberately opt in.

## New: FOMOD installer with auto-detection

The installer now checks your load order and configures itself:

- **SkyParkour detected** → "SkyParkour traversal" is pre-selected.
- **SkyParkour not found** → "Navmesh failsafe only" is pre-selected instead.
- **EVG not installed or disabled** → the experimental EVG option is greyed out, so it cannot be picked by mistake.

Each choice installs a matching INI, so your settings always agree with what is actually installed.

## Requirements

- SKSE64
- Address Library for SKSE Plugins
- **Optional:** SkyParkour V3 + its NPC behavior patch (for parkour traversal)
- **Optional:** MCM Helper (for the in-game settings menu)
- **Not required:** EVG Animated Traversal

## Updating

Replace the previous version. The ESP keeps the same FormIDs, so a clean save is not required. If a previous version had EVG traversal enabled in your save, switch it off in the MCM.
