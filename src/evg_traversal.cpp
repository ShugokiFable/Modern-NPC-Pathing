#include "evg_traversal.h"
#include "settings.h"

#include "RE/G/GridCellArray.h"
#include "RE/T/TES.h"
#include "RE/T/TESDataHandler.h"
#include "RE/T/TESObjectCELL.h"

#include <spdlog/spdlog.h>

#include <array>
#include <cmath>
#include <unordered_set>

namespace
{
    constexpr const char* EVG_PLUGIN = "EVGAnimatedTraversal.esl";

    // Usable traversal furniture, local FormIDs verified against the 2.1 ESL.
    // EVGATFailedLedgeMarker (0xF43) is deliberately absent — it's a fail anim.
    constexpr std::array<RE::FormID, 15> kFurnitureIDs = {
        0x80A,  // EVGATLadderShort
        0x812,  // EVGATSqueezeMarker
        0x827,  // EVGATDuckMarker
        0x83E,  // EVGATRaiderDropMarker
        0x891,  // EVGATLedgeMarker
        0x8F4,  // EVGATVaultMarker
        0x920,  // EVGATLadderShortMO
        0x956,  // EVGATTLSlideMO
        0x982,  // EVGATRaiderRollMarker
        0x98A,  // EVGATWallDropFMarker
        0x992,  // EVGATWallDropSMarker
        0x99A,  // EVGATDeepWalkMarker
        0x9BF,  // EVGATMediumLedgeMarker
        0x9C4,  // EVGATLedgeCatchMarker
        0xF1D,  // EVGATTallLadderUpMarker
    };

    std::unordered_set<RE::FormID> g_resolvedIDs;  // runtime FormIDs of the bases
    bool g_available = false;
}

namespace EvgTraversal
{
    void CacheForms()
    {
        g_resolvedIDs.clear();
        g_available = false;

        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            return;
        }

        for (const auto localID : kFurnitureIDs) {
            if (auto* form = dataHandler->LookupForm(localID, EVG_PLUGIN)) {
                g_resolvedIDs.insert(form->GetFormID());
            }
        }

        g_available = !g_resolvedIDs.empty();
        if (g_available) {
            spdlog::info("NPCPathingNG: EVG Animated Traversal found — {} marker types usable by NPCs",
                         g_resolvedIDs.size());
        } else {
            spdlog::info("NPCPathingNG: EVG Animated Traversal not present — marker traversal disabled");
        }
    }

    bool IsAvailable()
    {
        return g_available;
    }

    bool IsTraversalFurniture(const RE::TESBoundObject* a_base)
    {
        return a_base && g_resolvedIDs.contains(a_base->GetFormID());
    }

    RE::TESObjectREFR* FindMarkerNear(RE::Actor* a_actor, const RE::NiPoint3& a_fwd, float a_radius)
    {
        if (!g_available || !a_actor) {
            return nullptr;
        }
        auto* tes = RE::TES::GetSingleton();
        if (!tes) {
            return nullptr;
        }

        const RE::NiPoint3 pos = a_actor->GetPosition();
        RE::TESObjectREFR* best = nullptr;
        float bestScore = -1.0f;

        auto visit =
            [&](RE::TESObjectREFR* a_ref) {
                if (!a_ref || a_ref->IsDisabled() || a_ref->IsDeleted() || !a_ref->Is3DLoaded() ||
                    !IsTraversalFurniture(a_ref->GetBaseObject())) {
                    return RE::BSContainer::ForEachResult::kContinue;
                }

                const RE::NiPoint3 mPos = a_ref->GetPosition();
                if (std::abs(mPos.z - pos.z) > 200.0f) {
                    return RE::BSContainer::ForEachResult::kContinue;
                }

                // Marker heading = traversal direction.
                const float mYaw = a_ref->GetAngleZ();
                const RE::NiPoint3 mFwd(std::sin(mYaw), std::cos(mYaw), 0.0f);

                // Entry-side gate: the actor must not already be past the marker,
                // or the anim would carry them backwards through the obstacle.
                RE::NiPoint3 toActor = pos - mPos;
                toActor.z = 0.0f;
                if (toActor.Dot(mFwd) > 30.0f) {
                    return RE::BSContainer::ForEachResult::kContinue;
                }

                // The actor's travel direction must roughly match the traversal.
                const float align = a_fwd.Dot(mFwd);
                if (align < 0.0f) {
                    return RE::BSContainer::ForEachResult::kContinue;
                }

                // Prefer aligned, close markers.
                const float dist = std::max(1.0f, toActor.Length());
                const float score = align + (a_radius - dist) / a_radius;
                if (score > bestScore) {
                    bestScore = score;
                    best = a_ref;
                }
                return RE::BSContainer::ForEachResult::kContinue;
            };

        // Deliberately NOT TES::ForEachReferenceInRange: that helper ends every
        // scan with worldSpace->GetSkyCell(), and TES::worldSpace can dangle
        // during cell transitions — confirmed crash (AE id 20543, this+0xF8).
        // Markers are never in the sky cell, so we walk the cells ourselves.
        if (tes->interiorCell) {
            tes->interiorCell->ForEachReferenceInRange(pos, a_radius, visit);
        } else if (auto* grid = tes->gridCells; grid && grid->length > 0) {
            const auto length = grid->length;
            for (std::uint32_t x = 0; x < length; x++) {
                for (std::uint32_t y = 0; y < length; y++) {
                    if (auto* cell = grid->GetCell(x, y); cell && cell->IsAttached()) {
                        cell->ForEachReferenceInRange(pos, a_radius, visit);
                    }
                }
            }
        }

        return best;
    }

    bool Use(RE::Actor* a_actor, RE::TESObjectREFR* a_marker)
    {
        if (!a_actor || !a_marker) {
            return false;
        }
        // The marker may have been stored a while ago (follower replay) — never
        // hand the engine an unloaded or detached reference.
        if (a_marker->IsDisabled() || a_marker->IsDeleted() || !a_marker->Is3DLoaded()) {
            return false;
        }
        auto* cell = a_marker->GetParentCell();
        if (!cell || !cell->IsAttached()) {
            return false;
        }
        // Same call Papyrus ObjectReference.Activate(akActor) makes — the engine
        // walks the NPC into the furniture and the OAR-swapped anim plays.
        return a_marker->ActivateRef(a_actor, 0, nullptr, 1, false);
    }
}
