# NPC Pathing NG v2.5.0

Runtime navmesh failsafe for Skyrim SE/AE humanoid NPCs, with **optional** SkyParkour vault/climb, EVG Animated Traversal marker routes, and follower parkour replay. Stuck NPCs can step around geometry, open simple doors, vault or climb when SkyParkour is installed, route across EVG markers when EVG is installed, and only then fall back to a short validated sidestep teleport.

## What this mod actually does

- Detects NPCs that are **trying to walk** but not moving (idle, sandboxing, sitting, swimming, mounted, etc. are ignored).
- Tries animated **SkyParkour** vault/climb when that integration is installed and enabled.
- Lets **followers** replay *your* recent SkyParkour moves (teammate flag - works with NFF, vanilla followers, and similar frameworks).
- Handles **doorways** without shoving NPCs sideways out of the only route.
- Uses a **validated last-resort teleport** only after repeated stuck cycles against real static geometry - not against the player body, other actors, or dialogue holds.
- Ships with an **MCM** (MCM Helper) and an **INI** fallback.

## What this mod does **not** claim

- **EVG Animated Traversal is optional, but it *is* a working NPC feature since 2.5.0** — as *routes*, not furniture.
  - Plugin master list is only `Skyrim.esm`. EVG (and SkyParkour) are runtime lookups.
  - NPCs can never *enter* EVG furniture: activation is rejected by the engine (furniture entry for NPCs is package-driven; confirmed from SKSE and Papyrus during 2.4.3-2.4.4). So 2.5.0 treats each marker as a **route**: a stuck NPC approaching a marker from its entry side first tries a SkyParkour move along the marker's heading, then a collision-validated hop across to a landing derived from the marker's bounds and kind (ladder/ledge = up, drop/roll = down, squeeze/vault = across).
  - Default: `bEnableEVGTraversal=1`. With EVG absent, the route layer is inert (no markers resolve) and costs at most a bounded scan in stuck NPCs' cells.
  - Your own player-side EVG use is unaffected.
- **Not a full AI overhaul.** It unstucks and optionally parkours; it does not rewrite pathfinding or combat AI.
- **Default climb height is 130 units** (steps, vaults, low/chest ledges) - not full mountain climbing. Raise toward 250 in MCM/INI if you want higher climbs.
- **Indoor parkour is off by default.** Teleport fallback can still clear stuck NPCs indoors.
- **2.4.8 was the first genuine native rebuild since 2.4.4.** Versions 2.4.5, 2.4.6 and 2.4.7 all shipped the
  identical 2.4.4 DLL (SHA256 `9e5616e0...`), so any behaviour difference reported between those three
  releases was not native code. The 2.4.8 DLL embeds version `2.4.8`.

## What changed in 2.5.0

EVG Animated Traversal markers are now **routes**, and the feature is on by default.

- **Never activates EVG furniture for NPCs** (`ActivateRef`/`Papyrus.Activate` are engine-rejected for NPCs — that path is gone entirely, including the old self-disable latch that existed because of it).
- **Marker-guided parkour:** a stuck NPC near a marker parkours along the marker's heading when the geometry matches. The 2.4.10 player-distance cap still applies, so no distant 2D climb SFX.
- **Bounds-based landing hop:** when the geometry does not match a parkour move (ladders, squeezes, drops), the NPC is hopped across to a landing derived from the marker's kind (Up/Down/Across) and furniture bounds — ground-snapped, headroom- and capsule-cleared, never into water, never through actors, and never while in combat near the player. It fires only after the animated routes have had a chance (second stuck trigger onward).
- **Follower replay** routes followers across the same markers after you use them.
- **Scan latch re-arms on cell change** — a markerless city no longer disables marker use for the whole session, and a markerless area no longer re-scans the cell grid forever.
- **Default turned on** (`bEnableEVGTraversal=1`; MCM "NPCs Use EVG Markers", FOMOD presets rebuilt: SkyParkour + EVG / SkyParkour only / Navmesh failsafe). No new settings otherwise — the ESP globals, record layout and FormIDs are unchanged, so no save cleaning.

## What changed in 2.4.10

Phantom SkyParkour climb sounds. SkyParkour's climb SFX are 2D (`SOMStereo`) —
they play at your ears with no distance fade. 2.4.9 (and earlier) fired parkour
on any stuck humanoid in high process, so you heard a climb next to you with
nobody visible, including while camping "alone" with a hunter or traveler still
in the loaded grid. Parkour now requires the NPC to be within **1600** units of
you (MCM/INI `fParkourMaxPlayerDistance`; **0** restores the old unlimited
range). Animals were already excluded; they were never the source.

## What changed in 2.4.9

Performance release, aimed squarely at frame drops in crowded cities. No behaviour
change and nothing to reconfigure.

- **Crowd jams no longer run the parkour ledge sweep.** One NPC wedged behind another
  is the most common stuck cause in a city, and the ~15-iteration sweep could never
  succeed there (actors are rejected as landing surfaces). Two rays classify the
  blocker first and bail.
- **EVG marker scan no longer walks the whole cell grid.** 250-unit radius, but it was
  calling `ForEachReferenceInRange` on all 25 attached cells; out-of-range cells are
  now skipped.
- **EVG self-disable latch now trips on fruitless scans**, not just rejected
  activations - previously it could never trip anywhere without markers, i.e. every
  city, so the scan repeated all session.
- **Follower-faction lookup cached** instead of a global form-table lookup per actor
  per check.

The EVG items only affected `bEnableEVGTraversal=1` setups. Back then that option
remained off by default and could not work for NPCs; 2.5.0 replaces the activation
approach with marker routes entirely (see above).

## What changed in 2.4.8

- **Fixed:** NPCs vaulted onto barrels, urns and crates and were left stranded on top of them.
  The landing-surface filter only rejected NPCs, doors and activators; `Container`,
  `MovableStatic`, `Flora` and `Tree` are now rejected too.
- **Fixed:** INI edits and the FOMOD preset were silently ignored whenever the ESP was active,
  because settings were copied out of the plugin's globals every frame. The INI now *seeds*
  the MCM at `kDataLoaded` (which runs before a save loads), so a new game or fresh install
  starts from your INI/FOMOD preset, while loading an existing save still restores that
  save's own MCM values. MCM changes still apply instantly.
- **Changed:** `iTeleportEscalation` default raised 3 -> 5; teleports were firing more often
  than the animated traversal they exist to back up.
- **Note:** the `NPCPathingNG.esp` header author field changed, so the plugin hash differs from
  2.4.4-2.4.7. Only the TES4 header changed - no record, FormID or master - so **no save
  cleaning is needed**.

## Features (accurate)

**Follower parkour (optional, needs SkyParkour)**  
When you vault or climb with SkyParkour, the mod can record the spot. A follower who reaches that area while you are above them may perform the same move instead of pathing around or warping. Detected via the teammate flag.

**Combat pursuit (default on)**  
NPCs in combat are processed by default so guards/foes can climb after you on short ledges. One MCM toggle turns that off if your load order misbehaves.

**Climbing stays grounded by default**  
Max climb **130** units by default. Indoors parkour off by default. SkyParkour
animations only fire within **1600** units of the player (their climb SFX are 2D).

**Teleport is last resort**  
Only after parkour/other escapes fail against static geometry, with corridor and destination checks. Can be disabled entirely.

**Stuck detection is motion-gated**  
Requires walk intent + no progress. Idle NPCs are not "fixed."

**MCM via MCM Helper (optional)**  
Pages: General, Parkour, Followers & Combat. Globals apply mid-game. Without MCM Helper, edit `Data/SKSE/Plugins/NPCPathingNG.ini` (re-read when you close the journal).

**FOMOD installer**  
Auto-detects SkyParkour / EVG presence and pre-selects a matching INI profile:
SkyParkour + EVG (recommended), SkyParkour only, or Navmesh failsafe only.

## Requirements

**Hard**

- Skyrim SE (1.5.97) or AE (1.6.x)
- SKSE64
- Address Library for SKSE Plugins

**Optional**

- [SkyParkour V3](https://www.nexusmods.com/skyrimspecialedition/mods/132292) + its Nemesis/Pandora (or equivalent) behavior patch - for vault/climb and follower replay
- SkyUI + MCM Helper - for the in-game menu (INI works without them)

**Not required**

- EVG Animated Traversal — optional. When installed, its markers are used as NPC routes (on by default); without it nothing changes.

`NPCPathingNG.esp` is ESL-flagged (no full load-order slot).

## Configuration

- MCM: **NPC Pathing NG**
- INI: `Data/SKSE/Plugins/NPCPathingNG.ini`
- Defaults: followers included, combat included, indoor parkour off, climb height **130**, parkour max distance **1600**, EVG marker routes **on**

## Recent versions

### 2.5.0 (2026-08-16) - current

- EVG markers as routes (on by default): marker-guided SkyParkour, then a bounds-validated landing hop; no more furniture activation for NPCs. FOMOD presets rebuilt.

### 2.4.10 (2026-08-16)

- Phantom SkyParkour climb sounds: SkyParkour SFX are 2D (`SOMStereo`). Parkour now requires the NPC to be within **1600** units of the player (`fParkourMaxPlayerDistance`; 0 = unlimited).

### 2.4.9

- City performance: crowd-jam early-out, EVG cell skip, fruitless-scan latch, cached follower faction.

### 2.4.8

- First genuine native rebuild since 2.4.4. No more vaults onto barrels/crates. INI/FOMOD preset actually seeds the MCM.

If you already loaded 2.4.5: install 2.4.8 or later, remove leftover `Data/Scripts/NPNG_MCMBridge.pex` if present. An empty orphan menu may remain on that save until cleaned or a new game is started.

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
