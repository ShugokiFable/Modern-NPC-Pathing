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

    /// Seed the bound globals from the INI values currently held in members.
    ///
    /// Called once at kDataLoaded, which runs BEFORE a save is deserialized.
    /// That ordering is the whole point: on a new game or a fresh install the
    /// INI (and therefore the FOMOD preset the user picked) becomes the starting
    /// MCM state, while loading an existing save still restores that save's own
    /// global values straight over the top. Without this the INI was inert for
    /// anyone running the ESP — Refresh() overwrote it every frame.
    void PushToGlobals();

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
    // SkyParkour climb/vault HKX files fire SoundPlay.SPPF_* descriptors that
    // use vanilla SOMStereo (2D, no attenuation). Without a range cap those
    // SFX play at the listener whenever any high-process NPC parkours — the
    // "ghost climb" reports after 2.4.9. 0 = unlimited (pre-2.4.10 behaviour).
    float parkourMaxPlayerDistance = 1600.0f;
    // NPCs use EVG Animated Traversal markers as ROUTES (2.5.0+): parkour
    // along the marker heading, else a collision-validated hop across. The
    // furniture is never activated for NPCs (engine-rejected). Harmless
    // without EVG installed — the route layer reports unavailable and the
    // only cost is a bounded marker scan in stuck NPCs' cells.
    bool enableEvgTraversal = true;

    // [Avoidance]
    bool  enableTeleportFallback = true;
    float snapDistance = 100.0f;
    int   teleportEscalation = 5;  // consecutive stuck triggers with no parkour/EVG escape before
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
    RE::TESGlobal* gParkourMaxPlayerDistance = nullptr;
};
