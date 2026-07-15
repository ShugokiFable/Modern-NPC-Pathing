#pragma once

#include "RE/A/Actor.h"
#include "RE/C/CollisionLayers.h"
#include "RE/N/NiPoint3.h"
#include "RE/T/TESObjectREFR.h"

namespace Raycast
{
    struct Result
    {
        bool               didHit = false;
        float              distance = 0.0f;  // game units from start to hit; a_maxDist when no hit
        RE::NiPoint3       hitPos;
        float              normalZ = 1.0f;   // z component of the hit normal
        RE::COL_LAYER      layer = RE::COL_LAYER::kUnidentified;
        RE::TESObjectREFR* hitRef = nullptr;
    };

    /// Havok ray from a_start along a_dir (normalized) for a_maxDist game units.
    /// Uses the actor's collision filter with the layer overridden to kTransparent,
    /// mirroring SkyParkour's raycasts — this skips the caster's own body.
    Result Cast(RE::Actor* a_actor, const RE::NiPoint3& a_start, const RE::NiPoint3& a_dir, float a_maxDist);
}
