#include "raycast.h"

#include "RE/B/bhkPickData.h"
#include "RE/B/bhkWorld.h"
#include "RE/H/hkpCollidable.h"
#include "RE/T/TESHavokUtilities.h"
#include "RE/T/TESObjectCELL.h"

namespace Raycast
{
    Result Cast(RE::Actor* a_actor, const RE::NiPoint3& a_start, const RE::NiPoint3& a_dir, float a_maxDist)
    {
        Result result;
        result.distance = a_maxDist;
        result.hitPos = a_start + a_dir * a_maxDist;

        if (!a_actor) {
            return result;
        }

        auto* cell = a_actor->GetParentCell();
        if (!cell) {
            return result;
        }

        auto* world = cell->GetbhkWorld();
        if (!world) {
            return result;
        }

        const float scale = RE::bhkWorld::GetWorldScale();
        const RE::NiPoint3 end = a_start + a_dir * a_maxDist;

        RE::bhkPickData pickData;
        pickData.rayInput.from = RE::hkVector4(a_start.x * scale, a_start.y * scale, a_start.z * scale, 0.0f);
        pickData.rayInput.to = RE::hkVector4(end.x * scale, end.y * scale, end.z * scale, 0.0f);

        // Actor's filter group with the layer forced to kTransparent — same trick
        // SkyParkour uses for all its parkour rays (skips self, hits world geometry).
        std::uint32_t filter = 0;
        a_actor->GetCollisionFilterInfo(filter);
        filter = (filter & ~static_cast<std::uint32_t>(0x7F)) |
                 static_cast<std::uint32_t>(RE::COL_LAYER::kTransparent);
        pickData.rayInput.filterInfo = filter;

        if (world->PickObject(pickData) && pickData.rayOutput.HasHit()) {
            result.didHit = true;
            result.distance = a_maxDist * pickData.rayOutput.hitFraction;
            result.hitPos = a_start + a_dir * result.distance;
            result.normalZ = pickData.rayOutput.normal.quad.m128_f32[2];
            if (pickData.rayOutput.rootCollidable) {
                result.layer = pickData.rayOutput.rootCollidable->GetCollisionLayer();
                result.hitRef = RE::TESHavokUtilities::FindCollidableRef(*pickData.rayOutput.rootCollidable);
            }
        }

        return result;
    }
}
