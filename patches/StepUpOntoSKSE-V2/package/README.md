# StepUpOnto SKSE 1.5.1 - SkyParkour / NPC Pathing NG Compatibility Build

A compatibility rebuild of **StepUpOnto SKSE** ([Nexus mod 175689](https://www.nexusmods.com/skyrimspecialedition/mods/175689)). All step-up functionality, settings, and behavior are unchanged from the original 1.5.

## What this build changes

**SkyParkour / NPC Pathing NG awareness.** While a SkyParkour animation is driving an actor (player parkour, or NPC parkour via NPC Pathing NG), the actor's character controller reads as "grounded and not moving" - which is exactly StepUpOnto's step trigger. The original build could fire its step-warp in the middle of a climb or vault, fighting the parkour animation's position alignment and glitching the move. This build checks the `SkyParkourOngoing` animation graph variable in both the player and NPC step gates and stays hands-off until the parkour move finishes.

That's the only behavior change. One internal change with no user impact: the SimpleIni library dependency was replaced with a built-in INI reader/writer (same file, same keys, same format).

## Installation

Install over the original StepUpOnto SKSE, letting the DLL overwrite. Your existing `StepUpOntoSKSE.ini` and all settings keep working - nothing else in the original mod needs to change.

Without SkyParkour installed this build behaves identically to the original.

## Credits

- **TheShinyHaxorus** - original StepUpOnto SKSE: all step-up design and implementation. This is their work with a two-function compatibility guard added, released under the mod page's open modification permissions with credit.
- SkyParkour V3 by Waffuru (the graph variable this build checks).
- Compatibility patch by karlo (NPC Pathing NG).
