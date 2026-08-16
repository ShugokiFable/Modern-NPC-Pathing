# NPC Pathing NG 2.5.0 — Nexus release notes

## Short summary

Runtime navmesh failsafe for stuck humanoid NPCs, with optional SkyParkour vault/climb, EVG Animated Traversal marker routes, and follower parkour replay.

**2.5.0** makes EVG markers work for NPCs — as **routes**, not furniture: marker-guided SkyParkour when the geometry matches, else a collision-validated hop across. **On by default** (MCM: "NPCs Use EVG Markers"; toggle off if you prefer).

## Is EVG Animated Traversal required? No.

**EVG has never been required.** The plugin's only master is `Skyrim.esm`. There is no hard dependency on EVG or SkyParkour. Forms are looked up at runtime and skipped when absent.

**How NPCs use EVG since 2.5.0:** the engine only lets NPCs enter furniture through AI packages, so *activating* an EVG marker as furniture is rejected for NPCs (confirmed from SKSE and Papyrus, and by EVG's own abandoned alias quest). 2.5.0 therefore uses the hand-placed markers as the intended route:

- A stuck NPC approaching a marker from its entry side first tries a **SkyParkour move along the marker's heading** when real geometry matches (vaults, ledges).
- Otherwise a **bounds-derived landing hop**: ladders/ledges hop up, drops/rolls/slides hop down, squeezes/vaults hop across. Landings are ground-snapped, headroom- and capsule-cleared, never into water, never through actors, and the hop is refused in combat near the player and until the animated routes have had a chance.
- **Followers replay** the route after you use a marker.
- Marker scanning (the only cost when EVG is absent) pauses per-cell and re-arms when you travel to a new cell.

The old engine-rejected furniture-activation code path is removed entirely, and the default is ON.

## What the mod does (honest)

- Stuck detection only when an NPC is **trying to walk** but not moving
- Optional **SkyParkour** vault/climb for NPCs (needs SkyParkour + behavior patch)
- **EVG marker routes** for stuck NPCs (needs EVG Animated Traversal; on by default)
- **Follower replay** of your SkyParkour moves and marker crossings (teammate flag; NFF-compatible)
- **Doorway** handling without sideways shove out of chokepoints
- **Last-resort** validated sidestep teleport (optional)
- **MCM** via MCM Helper, or INI without it
- **FOMOD** auto-detects SkyParkour / EVG and pre-selects a matching profile

## What it does not claim

- NPCs entering EVG **furniture** (engine limit — routes instead)
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
- EVG Animated Traversal — marker routes for NPCs

## Files to upload

Prefer: `NPC Pathing NG 2.5.0 FOMOD.zip`  
Alternate plain layout: `NPC Pathing NG 2.5.0.zip`

Version string on the file: **2.5.0**

## Updating

Replace the previous version. FormIDs, ESP globals and record layout are unchanged since 2.4.6. A clean save is not required. Existing saves keep their saved MCM values; the ONLY change is the EVG marker-route toggle default, which seeds fresh installs / new games.

## Changelog (player-facing)

### 2.5.0

- EVG markers as routes for stuck NPCs (marker-guided parkour, then bounds-validated landing hop) — **on by default**
- Removed the engine-rejected furniture-activation path; scan latch re-arms on cell change
- FOMOD presets rebuilt: SkyParkour + EVG (recommended) / SkyParkour only / Navmesh failsafe only
- No save cleaning needed

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

- Pulled — do not use

## Permissions / credits (page fields)

- Open source under **GPLv3** — see GitHub LICENSE
- Uses CommonLibSSE-NG (build-time); runtime needs SKSE + Address Library
- Optional integrations: SkyParkour, EVG Animated Traversal, MCM Helper / SkyUI — owned by their respective authors; this mod does not re-ship their assets

## Troubleshooting

- Log: `Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log`
- SKSE log for load failures
- When reporting: game version, SKSE version, this mod version, whether SkyParkour / EVG is installed, and the NPCPathingNG.log excerpt