# NPC Pathing NG 2.4.10 — Nexus release notes

## Short summary

Runtime navmesh failsafe for stuck humanoid NPCs, with optional SkyParkour vault/climb and follower parkour replay. EVG is **not required** and NPC EVG markers **do not work** (engine limit).

**2.4.10** stops phantom SkyParkour climb sounds. Those SFX are 2D (they play at your ears with no distance fade). Parkour now only runs if the NPC is within 1600 units of you.

## Is EVG Animated Traversal required? No.

**EVG has never been required.** The plugin's only master is `Skyrim.esm`. There is no hard dependency on EVG or SkyParkour. Forms are looked up at runtime and skipped when absent.

**EVG marker traversal for NPCs does not work** with the current approach: the engine only lets NPCs enter furniture through AI packages, so marker activation is rejected (confirmed from SKSE and Papyrus). As of 2.4.4 it is **off by default**. The FOMOD EVG option is experimental and greys out if EVG is not installed. Player-side EVG is unaffected.

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
- That animals were playing climb sounds (they were already excluded)

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

Prefer: `NPC Pathing NG 2.4.10 FOMOD.zip`  
Alternate plain layout: `NPC Pathing NG 2.4.10.zip`

Version string on the file: **2.4.10**

## Updating

Replace the previous version. FormIDs are unchanged except one **new** MCM global (`NPNG_ParkourMaxPlayerDistance`). A clean save is not required. Existing saves keep their other MCM values; the new distance cap starts at 1600.

## Changelog (player-facing)

### 2.4.10

- Fixed phantom SkyParkour climb sounds (2D SFX from off-screen / distant NPCs)
- New MCM/INI: `fParkourMaxPlayerDistance` (default 1600, 0 = unlimited)

### 2.4.9

- City performance: crowd-jam early-out, EVG cell skip, fruitless-scan latch, cached follower faction

### 2.4.8

- First genuine native rebuild since 2.4.4
- No more vaults onto barrels/crates
- INI/FOMOD preset actually seeds the MCM

### 2.4.6

- Fixed double MCM caused by 2.4.5 bridge script

### 2.4.5

- Yanked — do not use

## Permissions / credits (page fields)

- Open source under **GPLv3** — see GitHub LICENSE
- Uses CommonLibSSE-NG (build-time); runtime needs SKSE + Address Library
- Optional integrations: SkyParkour, EVG Animated Traversal, MCM Helper / SkyUI — owned by their respective authors; this mod does not re-ship their assets

## Troubleshooting

- Log: `Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log`
- SKSE log for load failures
- When reporting: game version, SKSE version, this mod version, whether SkyParkour is installed, and the NPCPathingNG.log excerpt
