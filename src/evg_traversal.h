#pragma once

#include "RE/A/Actor.h"
#include "RE/N/NiPoint3.h"
#include "RE/T/TESObjectREFR.h"

/// EVG Animated Traversal integration.
///
/// EVGAT traversal points are FURNITURE markers (EVGAT*Marker in
/// EVGAnimatedTraversal.esl). The traversal animation is the furniture's use
/// animation — OAR swaps the vanilla lever-pull anim per furniture base
/// (CurrentFurniture condition).
///
/// NPC limitation (confirmed 2026-07 and again 2026-08, runtime-evidenced):
/// activating the furniture works for the PLAYER but returns false for NPCs,
/// from both ActivateRef and Papyrus Activate(). Furniture entry for an NPC is
/// driven by the AI package/procedure system, not the activation handler, so
/// there is no activation path to force. EVG's own author reached the same
/// conclusion: the plugin ships a quest, an AI package, and a package fragment
/// script named EVGNPCActivateItem for exactly this, and that alias-driven
/// system was abandoned (its cleanup script body is commented out) because a
/// single alias pair can only serve one NPC at a time.
///
/// Since 2.5.0 the markers are used as ROUTES instead of furniture. A stuck
/// NPC approaching a marker from its entry side first tries a SkyParkour move
/// along the marker's heading, then a collision-validated landing hop derived
/// from the marker's bounds (see PathingManager::TryEvgTraversal). The
/// furniture itself is never activated for NPCs.
namespace EvgTraversal
{
    /// What a marker route does to the actor's elevation.
    enum class RouteKind
    {
        Across,  // squeeze / vault / duck — same level, other side
        Up,      // ladder / ledge — land on top
        Down     // drop / roll / slide — land below
    };

    /// Resolve the furniture base forms from EVGAnimatedTraversal.esl.
    /// Call at kDataLoaded. Safe to call when the mod isn't installed.
    void CacheForms();

    /// True if EVGAnimatedTraversal.esl is loaded and forms resolved.
    bool IsAvailable();

    /// True while marker scanning is enabled. Disables for a cell after
    /// repeated scans find no markers (markerless areas), and re-arms when the
    /// player's cell changes or on save load.
    bool IsRouteEnabled();

    /// Track the player's current cell (cheap; call every frame). Re-arms the
    /// fruitless-scan latch when the player travels somewhere new.
    void UpdatePlayerCell(const RE::TESObjectCELL* a_cell);

    /// Record a marker scan that found nothing. Counts toward the latch in
    /// IsRouteEnabled, so markerless areas (most cities) do not re-scan the
    /// cell grid for the whole session.
    void NoteFruitlessScan();

    /// Record a successful route use — clears the fruitless-scan counter.
    void NoteRouteSuccess();

    /// Drop the session's scan latch (called on save load / new game).
    void ResetRouteState();

    /// True if a_base is one of the usable EVGAT traversal furniture bases.
    bool IsTraversalFurniture(const RE::TESBoundObject* a_base);

    /// Route class of a traversal base: Across, Up or Down.
    RouteKind KindFor(const RE::TESBoundObject* a_base);

    /// Find the best traversal marker near the actor: within a_radius, on the
    /// entry side, heading roughly along the actor's direction of travel.
    RE::TESObjectREFR* FindMarkerNear(RE::Actor* a_actor, const RE::NiPoint3& a_fwd, float a_radius);
}
