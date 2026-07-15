#pragma once

#include "RE/A/Actor.h"
#include "RE/N/NiPoint3.h"
#include "RE/T/TESObjectREFR.h"

/// EVG Animated Traversal integration.
///
/// EVGAT traversal points are FURNITURE markers (EVGAT*Marker in
/// EVGAnimatedTraversal.esl). The traversal animation is the furniture's use
/// animation — OAR swaps the vanilla lever-pull anim per furniture base
/// (CurrentFurniture condition). Activating the furniture ref with an actor
/// as activator makes that actor use it: the engine aligns them to the marker
/// and the root motion carries them across. That works for NPCs exactly like
/// for the player — the original mod just never wired NPCs up (its quest-alias
/// NPC path is commented out in the shipped scripts; NPCs get teleported by
/// patch mods instead).
namespace EvgTraversal
{
    /// Resolve the furniture base forms from EVGAnimatedTraversal.esl.
    /// Call at kDataLoaded. Safe to call when the mod isn't installed.
    void CacheForms();

    /// True if EVGAnimatedTraversal.esl is loaded and forms resolved.
    bool IsAvailable();

    /// True if a_base is one of the usable EVGAT traversal furniture bases.
    bool IsTraversalFurniture(const RE::TESBoundObject* a_base);

    /// Find the best traversal marker near the actor: within a_radius, on the
    /// entry side, heading roughly along the actor's direction of travel.
    RE::TESObjectREFR* FindMarkerNear(RE::Actor* a_actor, const RE::NiPoint3& a_fwd, float a_radius);

    /// Send the NPC through the marker (equivalent of Papyrus marker.Activate(npc)).
    bool Use(RE::Actor* a_actor, RE::TESObjectREFR* a_marker);
}
