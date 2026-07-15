#pragma once

#include "parkour_types.h"

#include "RE/A/Actor.h"
#include "RE/N/NiPoint3.h"

namespace NpcParkour
{
    struct Detection
    {
        NpcParkourType type = NpcParkourType::NoLedge;
        RE::NiPoint3   ledgePoint;
        RE::NiPoint3   startPos;  // where the anim must start so its root motion ends on the ledge
    };

    /// True if the SkyParkour third-person behavior patch is active on this actor's graph.
    bool HasBehaviorPatch(RE::Actor* a_actor);

    /// Full ledge scan (vault first, then climb) — port of SkyParkour's player-side checks.
    /// a_fwd must be the flat, normalized facing direction. a_maxClimbHeight caps climb scan (game units, unscaled).
    Detection Detect(RE::Actor* a_actor, const RE::NiPoint3& a_fwd, float a_maxClimbHeight);

    /// Set graph variables and fire the SkyParkour start event.
    /// Returns true if the graph accepted the notify.
    bool Trigger(RE::Actor* a_actor, const Detection& a_det);

    /// Graph entered the parkour state: disable char controller simulation
    /// (enables vertical root motion — same as SkyParkour's OnStartStop start path).
    void OnParkourStart(RE::Actor* a_actor);

    /// Parkour finished / must be force-stopped: restore simulation, reset the ledge
    /// graph var and (optionally) send the interrupt event so all graphs resync.
    void OnParkourEnd(RE::Actor* a_actor, bool a_sendInterrupt);
}
