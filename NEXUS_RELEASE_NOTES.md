# NPC Pathing NG 2.4.6 — Nexus release notes

## Short summary

Runtime navmesh failsafe for stuck humanoid NPCs, with optional SkyParkour vault/climb and follower parkour replay. EVG is **not required** and NPC EVG markers **do not work** (engine limit). 2.4.6 fixes the double MCM from yanked 2.4.5; native DLL is still the 2.4.4 binary.

## Is EVG Animated Traversal required? No.

**EVG has never been required.** The plugin’s only master is `Skyrim.esm`. There is no hard dependency on EVG or SkyParkour. Forms are looked up at runtime and skipped when absent.

**EVG marker traversal for NPCs does not work** with the current approach: the engine only lets NPCs enter furniture through AI packages, so marker activation is rejected (confirmed from SKSE and Papyrus). As of 2.4.4 it is **off by default**. The FOMOD EVG option is experimental and greys out if EVG is not installed. Player-side EVG is unaffected.

If you crash and suspect EVG, post a crash log — with default settings this mod does not touch EVG markers.

## What the mod does (honest)

- Stuck detection only when an NPC is **trying to walk** but not moving
- Optional **SkyParkour** vault/climb for NPCs (needs SkyParkour + behavior patch)
- **Follower replay** of your SkyParkour moves (teammate flag; NFF-compatible)
- **Doorway** handling without sideways shove out of chokepoints
- **Last-resort** validated sidestep teleport (optional)
- **MCM** via MCM Helper, or INI without it
- **FOMOD** auto-detects SkyParkour and pre-selects a matching profile

## What it does not claim

- Full EVG Animated Traversal for NPCs
- Full AI / navmesh rebuild
- Default mountain-scale climbing (default max climb **130** units)
- That package 2.4.6 rebuilt the DLL (it did not; DLL remains 2.4.4 byte-identical)

## Requirements

**Required**

- SKSE64
- Address Library for SKSE Plugins
- Skyrim SE 1.5.97 or AE 1.6.x

**Optional**

- SkyParkour V3 + Nemesis/Pandora (or equivalent) NPC/behavior patch — parkour + follower replay
- SkyUI + MCM Helper — in-game menu

**Not required**

- EVG Animated Traversal

## Files to upload

Prefer: `NPC Pathing NG 2.4.6 FOMOD.zip`  
Alternate plain layout: `NPC Pathing NG 2.4.6.zip` (if provided)

Version string on the file: **2.4.6**

## Updating

Replace the previous version. FormIDs are unchanged; a clean save is not required for normal updates.

**If you installed 2.4.5 (yanked):**

1. Install 2.4.6
2. Delete leftover `Data/Scripts/NPNG_MCMBridge.pex` if a manager left it
3. Empty second MCM may remain on **saves that already loaded 2.4.5** until cleaned or a new game

If an older save had EVG traversal toggled on, turn it off in the MCM (default is off).

## Changelog (player-facing)

### 2.4.6

- Fixed double MCM caused by 2.4.5 bridge script
- Restored stock MCM Helper quest script setup from 2.4.4
- Removed `NPNG_MCMBridge` from the package
- Native DLL unchanged from 2.4.4

### 2.4.5

- Yanked — do not use

### 2.4.4

- EVG off by default; documented as non-working for NPCs
- FOMOD auto-detection

### 2.4.3

- No more sideways shove out of doorways; open simple doors instead
- EVG NPC attempts fail loudly and stop retry loops

### 2.4.2

- Post-vault posture/tilt fix
- Follower replay radius/threshold tuning

### 2.4.1

- Safer parkour disable mid-animation; independent SkyParkour/EVG toggles; teleport clearance fixes; packaging/CI

## Permissions / credits (page fields)

- Open source under **GPLv3** — see GitHub LICENSE
- Uses CommonLibSSE-NG (build-time); runtime needs SKSE + Address Library
- Optional integrations: SkyParkour, EVG Animated Traversal, MCM Helper / SkyUI — owned by their respective authors; this mod does not re-ship their assets

## Troubleshooting

- Log: `Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log`
- SKSE log for load failures
- When reporting: game version, SKSE version, this mod version, whether SkyParkour is installed, and the NPCPathingNG.log excerpt
