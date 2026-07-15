# NPC Pathing NG v2.2

NPCs finally move like they belong in your modded world. A runtime navmesh failsafe **plus full NPC SkyParkour and NPC EVG Animated Traversal**: followers retrace your climbing route instead of teleporting to you, guards and enemies climb after you when you kite them onto a rock, NPCs use EVG traversal points (ladders, squeezes, ledges) with real animations instead of the teleport the original framework falls back to, and any humanoid NPC that walks into broken navmesh gets itself unstuck — by traversing, vaulting, climbing, or (as a last resort) a short validated sidestep teleport.

## Features

**Follower parkour (Nether's Follower Framework compatible)**
When *you* vault or climb with SkyParkour, the mod records the spot and direction. A follower reaching that spot while you're above them performs the **same move** — they physically retrace your route up the mountain instead of pathing around or warp-teleporting behind you. Detected via the teammate flag, so NFF, vanilla, and any other follower framework all work. Followers also react to being stuck twice as fast as regular NPCs.

**Combat pursuit**
NPCs in combat are processed by default. A guard chasing your bounty up a rocky hill will climb the same ledges you did (stuck detection kicks in within ~half a second of them running into the rock face). Same for bandits, draugr-free humanoid foes, etc. If it misbehaves in your setup, one MCM toggle turns it off.

**Everything climbs, everywhere (outdoors)**
Full SkyParkour height range by default — 250 units, enough for city walls and rocky mountainsides. Indoors, parkour is **off by default** (no NPCs scaling your kitchen); the teleport fallback still quietly fixes stuck NPCs inside. MCM lets you allow steps & vaults or everything indoors if you want.

**NPCs use EVG Animated Traversal — with animations, not teleports**
EVG AT's traversal points are furniture markers whose use-animation OAR-swaps into the traversal move — which means NPCs can use them natively; the framework just never wired them up (its NPC handling is literally commented out, so patch mods teleport NPCs instead). This mod does the wiring: a stuck NPC near a marker that points the right way is sent through it — climbing the ladder, squeezing the crack, vaulting the ledge, animation and all. And when *you* use an EVG point, your followers use the **same marker** behind you. Works with every EVG patch collection automatically (detection is by marker furniture base, not by location). Fully optional — auto-disables if EVG AT isn't installed.

**Stuck detection that can't misfire on idle NPCs**
An NPC only counts as stuck when its animation graph is *trying to walk* but its body isn't moving. Idle guards, sandboxing citizens, sitting/sleeping/ragdolled/swimming/mounted NPCs are never touched.

**MCM (via MCM Helper)**
All settings live in an in-game MCM — three pages: General, Parkour, Followers & Combat. Settings are stored in ESP globals, so **every change applies instantly**, mid-game, no reload. Without MCM Helper the mod falls back to the INI (re-read every time you close the journal, so INI edits also apply without restarting).

## Requirements

- Skyrim SE (1.5.97) or AE (1.6.x) — version independent via Address Library
- SKSE64
- **For parkour:** [SkyParkour V3](https://www.nexusmods.com/skyrimspecialedition/mods/132292) + its Nemesis/Pandora patch (the patch also covers NPC behavior graphs — detected per-actor at runtime)
- **For marker traversal:** [EVG Animated Traversal](https://www.nexusmods.com/skyrimspecialedition/mods/63232) + any patch collections that place its markers
- **For MCM:** SkyUI + MCM Helper (optional — INI works without them)
- `NPCPathingNG.esp` is ESL-flagged (no load order slot cost)

## How the parkour works technically

SkyParkour's behavior patch modifies the shared humanoid graphs (`0_master`, `defaultmale/female`), so every humanoid NPC already has the animations — only SkyParkour's own plugin is player-locked. This mod drives the same graph for NPCs: same trigger variables, same char-controller physics handling during the move, same start-position alignment so the root motion lands the NPC exactly on the ledge, and the same safety rules (headroom, ledge flatness, landing space for vaults, never onto actors/doors, never into water). Interrupts, ragdolls, deaths and stuck animations all have failsafe cleanup.

## Configuration

MCM: **NPC Pathing NG** in the mod configuration list. INI fallback: `Data/SKSE/Plugins/NPCPathingNG.ini`.

Notable defaults: followers **included**, combat **included**, indoor parkour **disabled**, climb height **250** (max).

## Logs

`Documents/My Games/Skyrim Special Edition/SKSE/NPCPathingNG.log` (set Debug Logging ON to see every event)
