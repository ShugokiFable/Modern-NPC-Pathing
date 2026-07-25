# NPC Pathing NG v2.4.6

Runtime navmesh failsafe for Skyrim SE/AE humanoid NPCs, with **optional** SkyParkour vault/climb and follower parkour replay. Stuck NPCs can step around geometry, open simple doors, vault or climb when SkyParkour is installed, and only then fall back to a short validated sidestep teleport.

## What this mod actually does

- Detects NPCs that are **trying to walk** but not moving (idle, sandboxing, sitting, swimming, mounted, etc. are ignored).
- Tries animated **SkyParkour** vault/climb when that integration is installed and enabled.
- Lets **followers** replay *your* recent SkyParkour moves (teammate flag - works with NFF, vanilla followers, and similar frameworks).
- Handles **doorways** without shoving NPCs sideways out of the only route.
- Uses a **validated last-resort teleport** only after repeated stuck cycles against real static geometry - not against the player body, other actors, or dialogue holds.
- Ships with an **MCM** (MCM Helper) and an **INI** fallback.

## What this mod does **not** claim

- **EVG Animated Traversal is not required** and is **not a working NPC feature**.
  - Plugin master list is only `Skyrim.esm`. EVG (and SkyParkour) are runtime lookups.
  - NPC marker activation is rejected by the engine: furniture entry for NPCs is package-driven. Confirmed from SKSE and Papyrus work during 2.4.3-2.4.4.
  - Default: `bEnableEVGTraversal=0`. The FOMOD EVG option is labelled **experimental** and only for testing a future approach.
  - Your own player-side EVG use is unaffected.
- **Not a full AI overhaul.** It unstucks and optionally parkours; it does not rewrite pathfinding or combat AI.
- **Default climb height is 130 units** (steps, vaults, low/chest ledges) - not full mountain climbing. Raise toward 250 in MCM/INI if you want higher climbs.
- **Indoor parkour is off by default.** Teleport fallback can still clear stuck NPCs indoors.
- **Package 2.4.6 ships the 2.4.4 native DLL unchanged** (byte-identical). The DLL's embedded SKSE version string is still `2.4.4`; 2.4.6 is a package/ESP/installer release (double-MCM fix). A future DLL rebuild will bump the embedded version.

## Features (accurate)

**Follower parkour (optional, needs SkyParkour)**  
When you vault or climb with SkyParkour, the mod can record the spot. A follower who reaches that area while you are above them may perform the same move instead of pathing around or warping. Detected via the teammate flag.

**Combat pursuit (default on)**  
NPCs in combat are processed by default so guards/foes can climb after you on short ledges. One MCM toggle turns that off if your load order misbehaves.

**Climbing stays grounded by default**  
Max climb **130** units by default. Indoors parkour off by default.

**Teleport is last resort**  
Only after parkour/other escapes fail against static geometry, with corridor and destination checks. Can be disabled entirely.

**Stuck detection is motion-gated**  
Requires walk intent + no progress. Idle NPCs are not "fixed."

**MCM via MCM Helper (optional)**  
Pages: General, Parkour, Followers & Combat. Globals apply mid-game. Without MCM Helper, edit `Data/SKSE/Plugins/NPCPathingNG.ini` (re-read when you close the journal).

**FOMOD installer**  
Auto-detects SkyParkour / EVG presence and pre-selects a matching INI profile. EVG experimental option is greyed out if EVG is missing.

## Requirements

**Hard**

- Skyrim SE (1.5.97) or AE (1.6.x)
- SKSE64
- Address Library for SKSE Plugins

**Optional**

- [SkyParkour V3](https://www.nexusmods.com/skyrimspecialedition/mods/132292) + its Nemesis/Pandora (or equivalent) behavior patch - for vault/climb and follower replay
- SkyUI + MCM Helper - for the in-game menu (INI works without them)

**Not required**

- EVG Animated Traversal (and NPC EVG marker use does not currently work)

`NPCPathingNG.esp` is ESL-flagged (no full load-order slot).

## Configuration

- MCM: **NPC Pathing NG**
- INI: `Data/SKSE/Plugins/NPCPathingNG.ini`
- Defaults: followers included, combat included, indoor parkour off, climb height **130**, EVG off

## Recent versions

### 2.4.6 (2026-07-23) - current package

- Fixes **double MCM** introduced by yanked **2.4.5** (`NPNG_MCMBridge`).
- Restores **2.4.4 ESP** (`MCM_ConfigBase` only). Does **not** ship the bridge scripts.
- **DLL unchanged** from 2.4.4 (no native rebuild).

If you already loaded 2.4.5: install 2.4.6, remove leftover `Data/Scripts/NPNG_MCMBridge.pex` if present. An empty orphan menu may remain on that save until cleaned or a new game is started.

### 2.4.5 - yanked

Do not use. Dual MCM registration on saves that already had the stock menu.

### 2.4.4

- EVG off by default; documented as non-working for NPCs and not required.
- FOMOD with dependency auto-detection.

### 2.4.3-2.4.1

- Doorway sidestep fix; EVG failures fail loudly; posture/tilt fix after vaults; follower replay tuning; parkour disable cleanup; packaging/CI hardening. See `CHANGELOG.md`.

## Logs

`Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log`  
Enable Debug Logging in the MCM to see detailed events.

## Source / license

- Source: https://github.com/ShugokiFable/Modern-NPC-Pathing  
- License: GPLv3 (see `LICENSE`)  
- Nexus: https://www.nexusmods.com/skyrimspecialedition/mods/185413

## Related: StepUpOnto SKSE compatibility patch (V2)

If you use [StepUpOnto SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/175689) with SkyParkour / this mod,
install the companion patch so StepUp does not fire mid-parkour:

- Source: [patches/StepUpOntoSKSE-V2](patches/StepUpOntoSKSE-V2)
- Binary release: [StepUpOntoSKSE-NPCPathing-V2](https://github.com/ShugokiFable/Modern-NPC-Pathing/releases/tag/StepUpOntoSKSE-NPCPathing-V2)
