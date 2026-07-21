#pragma once

#include "RE/T/TESGlobal.h"

class Settings
{
public:
    static Settings* GetSingleton()
    {
        static Settings s;
        return &s;
    }

    /// Read the INI (base values / fallback for users without the ESP+MCM).
    void Load();

    /// Bind to NPCPathingNG.esp globals (MCM Helper edits those). Call at kDataLoaded.
    void BindGlobals();

    /// Copy bound global values into members — cheap, called every frame so
    /// MCM changes apply instantly. No-op when the ESP isn't present.
    void Refresh();

    // [General]
    bool  enabled = true;
    float checkInterval = 0.25f;   // seconds between position samples per actor
    int   stuckThreshold = 4;      // consecutive stuck samples before acting
    float stuckDistance = 4.0f;    // moved less than this per sample while trying to move = stuck
    float cooldown = 3.0f;         // seconds before the same actor can be unstuck again
    int   actorsPerFrame = 10;     // round-robin detection slice

    // [Parkour]
    bool  enableParkour = true;
    int   parkourIndoorMode = 0;   // 0 = no parkour indoors (default), 1 = steps+vault, 2 = everything
    float maxClimbHeight = 130.0f; // climb detection cap. 130 = up to low/chest ledges (steps,
                                   // vaults, low ledges) — NPCs don't scale walls/houses/mountains.
                                   // Raise toward 250 (SkyParkour's own max) for full mountain climbs.
    bool  enableEvgTraversal = false;  // NPCs use EVG Animated Traversal markers

    // [Avoidance]
    bool  enableTeleportFallback = true;
    float snapDistance = 100.0f;
    int   teleportEscalation = 3;  // consecutive stuck triggers with no parkour/EVG escape before
                                   // teleport is allowed. Higher = teleport is rarer / more last-resort.

    // [Followers]
    bool followerReplay = true;    // followers reproduce the player's parkour route to keep up

    // [Filters] — combat and followers are INCLUDED by default
    bool excludeInCombat = false;
    bool excludeFollowers = false;
    bool excludeMounted = true;

    // [Debug]
    bool debugLogging = false;

private:
    Settings() = default;

    // Globals from NPCPathingNG.esp (null when the ESP is disabled/missing).
    RE::TESGlobal* gEnabled = nullptr;
    RE::TESGlobal* gCheckInterval = nullptr;
    RE::TESGlobal* gStuckThreshold = nullptr;
    RE::TESGlobal* gStuckDistance = nullptr;
    RE::TESGlobal* gCooldown = nullptr;
    RE::TESGlobal* gActorsPerFrame = nullptr;
    RE::TESGlobal* gEnableParkour = nullptr;
    RE::TESGlobal* gIndoorMode = nullptr;
    RE::TESGlobal* gMaxClimbHeight = nullptr;
    RE::TESGlobal* gTeleportFallback = nullptr;
    RE::TESGlobal* gSnapDistance = nullptr;
    RE::TESGlobal* gExcludeInCombat = nullptr;
    RE::TESGlobal* gExcludeFollowers = nullptr;
    RE::TESGlobal* gExcludeMounted = nullptr;
    RE::TESGlobal* gFollowerReplay = nullptr;
    RE::TESGlobal* gDebugLogging = nullptr;
    RE::TESGlobal* gEvgTraversal = nullptr;
    RE::TESGlobal* gTeleportEscalation = nullptr;
};
