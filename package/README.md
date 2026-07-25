# NPC Pathing NG v2.4.6

Runtime navmesh failsafe for Skyrim SE/AE humanoid NPCs, with optional SkyParkour vault/climb and follower parkour replay.

## Does

- Unsticks humanoids that are trying to walk but not moving
- Optional SkyParkour vault/climb + follower replay of your parkour
- Doorway-aware handling (does not shove NPCs out of chokepoints)
- Validated last-resort sidestep teleport (can be disabled)
- MCM (MCM Helper) or INI fallback

## Does not

- **EVG is not required.** NPC EVG marker use does **not** currently work (engine furniture/package limit). Default off; experimental FOMOD option only.
- Not a full AI or pathfinding rewrite
- Default max climb is **130** units (not full cliffs unless you raise it)
- Package **2.4.6** ships the **2.4.4** native DLL unchanged (embedded DLL version remains `2.4.4`)

## Requirements

- SKSE64 + Address Library
- Optional: SkyParkour V3 + behavior patch; SkyUI + MCM Helper
- Not required: EVG Animated Traversal

## Config

- MCM: **NPC Pathing NG**
- INI: `Data/SKSE/Plugins/NPCPathingNG.ini`

## 2.4.6

Restores the 2.4.4 ESP (`MCM_ConfigBase`) after yanked 2.4.5 dual-MCM issue. No `NPNG_MCMBridge` scripts. If you loaded 2.4.5, remove leftover `NPNG_MCMBridge.pex`; orphan empty menu may linger on that save.

## Logs

`Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log`
