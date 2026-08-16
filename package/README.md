# NPC Pathing NG v2.5.0

Runtime navmesh failsafe for Skyrim SE/AE humanoid NPCs, with optional SkyParkour vault/climb, EVG Animated Traversal marker routes, and follower parkour replay.

## Does

- Unsticks humanoids that are trying to walk but not moving
- Optional SkyParkour vault/climb + follower replay of your parkour
- EVG Animated Traversal markers used as **routes** (on by default): marker-guided parkour, then a collision-validated hop across. Never activates the furniture (engine limit)
- Doorway-aware handling (does not shove NPCs out of chokepoints)
- Validated last-resort sidestep teleport (can be disabled)
- MCM (MCM Helper) or INI fallback

## Does not

- **EVG is not required.** With EVG Animated Traversal installed, its markers are used as NPC routes; without it, nothing changes
- Not a full AI or pathfinding rewrite
- Default max climb is **130** units (not full cliffs unless you raise it)
- Parkour is capped to **1600** units from the player by default (SkyParkour climb SFX are 2D). Set `fParkourMaxPlayerDistance=0` for the old unlimited range.

## Requirements

- SKSE64 + Address Library
- Optional: SkyParkour V3 + behavior patch; SkyUI + MCM Helper; EVG Animated Traversal (for marker routes)

## Config

- MCM: **NPC Pathing NG**
- INI: `Data/SKSE/Plugins/NPCPathingNG.ini`
- Defaults: EVG marker routes **on** (`bEnableEVGTraversal=1`), followers on, combat on, indoor parkour off, climb 130, parkour max distance 1600

## 2.5.0

EVG markers as routes: a stuck NPC near a marker approaches it from the entry side, parkours along its heading when the geometry matches, otherwise hops across to a bounds-validated landing (ladders/ledges up, drops/rolls down, squeezes/vaults across). The furniture-activation path is removed (it was engine-rejected for NPCs), the scan latch re-arms on cell change, and the feature is on by default. No save cleaning needed.

## 2.4.8

Restores the 2.4.4 ESP (`MCM_ConfigBase`) after yanked 2.4.5 dual-MCM issue. No `NPNG_MCMBridge` scripts. If you loaded 2.4.5, remove leftover `NPNG_MCMBridge.pex`; orphan empty menu may linger on that save.

## Logs

`Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log`
