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
/// IMPORTANT — NPC limitation (confirmed 2026-07, runtime-evidenced):
/// Activating the furniture works for the PLAYER but returns false for NPCs,
/// from both ActivateRef and Papyrus Activate(). Furniture entry for an NPC is
/// driven by the AI package/procedure system, not the activation handler, so
/// there is no activation path to force. EVG's own author reached the same
/// conclusion: the plugin ships a quest, an AI package, and a package fragment
/// script named EVGNPCActivateItem for exactly this, and that alias-driven
/// system was abandoned (its cleanup script body is commented out) because a
/// single alias pair can only serve one NPC at a time.
///
/// So NPC marker use auto-disables after repeated activation failures rather
/// than burning a stuck-cycle on a call that cannot succeed. Player-side
/// tracking stays useful only while NPC use is supported.
namespace EvgTraversal
{
    /// Resolve the furniture base forms from EVGAnimatedTraversal.esl.
    /// Call at kDataLoaded. Safe to call when the mod isn't installed.
    void CacheForms();

    /// True if EVGAnimatedTraversal.esl is loaded and forms resolved.
    bool IsAvailable();

    /// True while NPC marker use is still believed to work. Flips to false for
    /// the session once activation has failed repeatedly (see header note).
    bool IsNpcUseSupported();

    /// Drop the session's NPC-unsupported latch (called on save load / new game).
    void ResetNpcUseState();

    /// True if a_base is one of the usable EVGAT traversal furniture bases.
    bool IsTraversalFurniture(const RE::TESBoundObject* a_base);

    /// Find the best traversal marker near the actor: within a_radius, on the
    /// entry side, heading roughly along the actor's direction of travel.
    RE::TESObjectREFR* FindMarkerNear(RE::Actor* a_actor, const RE::NiPoint3& a_fwd, float a_radius);

    /// Send the NPC through the marker (equivalent of Papyrus marker.Activate(npc)).
    bool Use(RE::Actor* a_actor, RE::TESObjectREFR* a_marker);
}
