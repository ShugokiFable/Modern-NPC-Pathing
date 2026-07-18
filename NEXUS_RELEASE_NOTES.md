# NPC Pathing NG 2.4.1

## Stability update

This release fixes state-management problems that could leave an NPC's character controller or SkyParkour graph variables in a bad state after an interrupted climb. It also makes follower replay respect the SkyParkour and EVG toggles independently, stops normal animation completion from being treated as an interruption, clears lower-body animation state between moves, resets the teleport escalation counter after a successful bypass, and adds body-width travel, ground, headroom, and destination-radius checks.

## Additional corrections

- Corrected the ESP header's next-object metadata.
- Corrected the documented default maximum climb height to 130 units.
- Synchronized the DLL, CMake, package, and source version metadata.
- Added automated validation for the ESP, SEQ, MCM references, DLL architecture/exports/version, and release archive contents.

## Updating

Replace the previous version with 2.4.1. The ESP keeps the same FormIDs, so a clean save should not be required. Existing MCM values remain compatible.

## Requirements

- SKSE64
- Address Library for SKSE Plugins
- SkyParkour V3 plus its NPC behavior patch for SkyParkour traversal
- EVG Animated Traversal for EVG marker traversal
- MCM Helper for the in-game settings menu
