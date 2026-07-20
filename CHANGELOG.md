# Changelog

## 2.4.2 - 2026-07-20

### Fixed

- NPCs no longer get stuck walking diagonally after traversing fences and ledges. Root cause: the engine rights the *player's* pitch/roll from camera input every frame but never rights NPCs, so a vault that ended tilted (root-motion with simulation off, or a slope-rotated EVG furniture marker) left the actor tilted permanently. Every parkour end now zeroes controller pitch/roll and reference pitch/roll, and a 15-second post-traversal posture guard self-heals any residual tilt on the next detection sample (never in combat, so ranged aiming is untouched).

### Changed

- Follower replay is more reliable: trigger radius raised from 80 to 110 units (the anim-start distance cap still prevents wrong-ledge drags) and the player-above threshold lowered from 60 to 40 units, so followers reproduce more of the player's moves instead of falling back to stuck detection.

## 2.4.1 - 2026-07-18

### Fixed

- Restores character-controller simulation and graph state when the mod is disabled during an active NPC parkour animation.
- Separates EVG follower replay from the SkyParkour toggle, prevents replay through a disabled integration, and discards stale events when a toggle changes.
- Clears `SkyParkourLowerBody` before and after every move to prevent state leaking between animations.
- Sends `SkyParkour_Interrupt` only for actual forced stops and timeouts, not normal graph completion.
- Resets teleport escalation after a successful bypass and validates a body-width travel corridor plus ground, headroom, and body-radius clearance at the destination.
- Uses type-safe form casts for MCM globals and the follower faction lookup.
- Stores the ESP HEDR next-object value as the local object ID (`0x813`).
- Corrects the documented default climb height from 250 to 130.
- Synchronizes DLL source, CMake, vcpkg, documentation, and package version metadata.
- Replaces the Visual Studio 2026-only preset and makes CI resolve both standard vcpkg environment variables.

### Distribution

- Adds automated ESP/package validation and a reproducible Nexus ZIP builder.
- Adds a Windows GitHub Actions build that compiles against pinned CommonLibSSE-NG and uploads the release ZIP.
- Excludes compiler output, dependency checkouts, nested repositories, and archive debris from source distributions.
