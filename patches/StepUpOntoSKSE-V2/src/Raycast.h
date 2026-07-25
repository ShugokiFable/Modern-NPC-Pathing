#pragma once

namespace StepUp::Raycast
{
	/// Cheap forward-ray check: only returns the hit fraction in [0, 1].
	/// Uses a stack-local hkpWorldRayCastOutput internally so callers pay no alloc cost.
	bool CastRayWorldFraction(
		RE::bhkWorld*       a_world,
		std::uint32_t       a_filterInfo,
		const RE::NiPoint3& a_from,
		const RE::NiPoint3& a_to,
		float&              a_outFraction);

	/// Full ray cast: caller receives the complete hkpWorldRayCastOutput (hitFraction + normal etc.).
	bool CastRayWorld(
		RE::bhkWorld*              a_world,
		std::uint32_t              a_filterInfo,
		const RE::NiPoint3&        a_from,
		const RE::NiPoint3&        a_to,
		RE::hkpWorldRayCastOutput& a_outHit);

	/// Returns the player's Havok collision group (upper 16 bits of filterInfo).
	/// Result is cached after the first successful lookup.
	std::uint32_t GetPlayerFilterInfo(RE::Actor* a_actor);
}
