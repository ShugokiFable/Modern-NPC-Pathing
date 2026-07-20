# NPC Pathing NG 2.4.2

## Posture fix and follower-replay tuning

- **Fixed: NPCs stuck walking diagonally after vaulting fences/ledges.** The engine automatically rights the player's tilt every frame but never rights NPCs — a traversal that ended slightly tilted (root-motion vault, or a slope-rotated EVG marker) left them walking at an angle forever. Every traversal now clears pitch/roll on completion, and a 15-second post-traversal posture guard self-heals any residual tilt on the next detection pass. Combat aiming pitch is never touched.
- **Changed: followers reproduce the player's parkour and EVG moves more reliably** (replay trigger radius 80 → 110 units, player-above threshold 60 → 40). The 2.3.0 safeguards against wrong-ledge detection and visible position dragging remain in force.

## Updating

Replace the previous version with 2.4.2. The ESP keeps the same FormIDs, so a clean save should not be required. Existing MCM values remain compatible.

## Requirements

- SKSE64
- Address Library for SKSE Plugins
- SkyParkour V3 plus its NPC behavior patch for SkyParkour traversal
- EVG Animated Traversal for EVG marker traversal
- MCM Helper for the in-game settings menu
