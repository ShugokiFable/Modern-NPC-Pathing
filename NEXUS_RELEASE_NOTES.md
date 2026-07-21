# NPC Pathing NG 2.4.3

## Doorways, and honesty about EVG markers

- **Fixed: NPCs pushed sideways out of doorways.** A doorway is a chokepoint, not a wall. Sidestepping an NPC out of one removes them from the only route through, which was a likely contributor to NPCs milling around doors. The bypass now recognises a door ahead and never repositions there. If the door is simply shut — unlocked, and not a load door — it gets opened instead, and the NPC's own pathing takes it from there.

- **EVG marker traversal for NPCs no longer fails silently.** Activating an EVG furniture marker works for the player but is rejected for NPCs. This is an engine limitation, not a load-order problem: furniture entry for an NPC is driven by the AI package system, which activation cannot force. (EVG's own plugin ships an unused quest, AI package, and `EVGNPCActivateItem` fragment script built for exactly this, which its author abandoned.) After repeated rejections the mod now disables NPC marker traversal for the session, logs one clear explanation, and falls straight through to SkyParkour traversal instead of burning stuck-cycles or stalling follower replay.

  **In practice:** SkyParkour traversal, follower SkyParkour replay, the teleport fallback, and player-side EVG use are all unaffected. Only NPC use of EVG markers is disabled, and only because the engine will not permit it.

## Updating

Replace the previous version with 2.4.3. The ESP keeps the same FormIDs, so a clean save is not required. Existing MCM values remain compatible.

## Requirements

- SKSE64
- Address Library for SKSE Plugins
- SkyParkour V3 plus its NPC behavior patch for SkyParkour traversal
- MCM Helper for the in-game settings menu
