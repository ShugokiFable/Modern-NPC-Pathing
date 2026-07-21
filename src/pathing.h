#pragma once

#include "RE/A/Actor.h"
#include "RE/B/BGSKeyword.h"
#include "RE/N/NiPoint3.h"

#include <cstdint>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ActorEntry
{
    RE::NiPoint3 lastPos;
    double       lastCheckTime = -1.0;  // -1: no sample yet
    double       cooldownUntil = 0.0;
    double       lastSeenTime = 0.0;
    double       lastEvgTime = -100.0;  // last EVG marker activation attempt
    double       lastTraversalEnd = -100.0;  // parkour job finished — posture-guard window
    int          stuckCount = 0;
    int          escalation = 0;        // consecutive stuck triggers with no parkour/EVG escape
    bool         filterChecked = false;
    bool         passesFilter = false;
};

/// One NPC currently being pushed through a SkyParkour animation.
struct ParkourJob
{
    enum class Phase { WaitingForGraph, Ongoing };

    RE::ActorHandle handle;
    Phase           phase = Phase::WaitingForGraph;
    double          phaseStart = 0.0;
    RE::NiPoint3    startPos;      // anim start position (root motion origin)
    RE::NiPoint3    totalMissing;  // gap between actor and startPos when the anim began
    float           missingGap = 0.0f;
};

/// A traversal the player performed — followers replay these to keep up.
/// Two kinds: a SkyParkour move (pos/yaw only) or an EVG furniture marker
/// (furnRef set — followers activate the same marker).
struct PlayerParkourEvent
{
    RE::NiPoint3                    pos;   // player/marker position when the move started
    float                           yaw;   // heading at that moment
    double                          time;
    RE::ObjectRefHandle             furnRef;   // set for EVG traversal events
    std::unordered_set<RE::FormID>  consumed;  // followers that already replayed this
};

class PathingManager
{
public:
    static PathingManager* GetSingleton()
    {
        static PathingManager instance;
        return &instance;
    }

    /// Install the per-frame tick (PlayerCharacter::Update vfunc hook) and cache keywords.
    /// Call once, at SKSE kDataLoaded.
    static void Install();

    /// Main-thread frame tick.
    void OnFrame(float a_delta);

    /// Drop all state (call on save load / new game).
    void Reset();

private:
    PathingManager() = default;

    void CacheKeywords();
    bool PassesStaticFilter(RE::Actor* a_actor) const;
    bool PassesDynamicFilter(RE::Actor* a_actor) const;
    bool IsTeammate(RE::Actor* a_actor) const;
    /// Solid *static* geometry within reach ahead — a real navmesh wedge, not
    /// the player's body, another actor, or an AI/dialogue hold.
    bool IsGenuinelyWallStuck(RE::Actor* a_actor) const;
    /// In combat and close to the player: pressing the attack, not navmesh-stuck.
    bool InCombatNearPlayer(RE::Actor* a_actor) const;
    /// A door blocking the way ahead, if any (doorways must never be sidestepped).
    RE::TESObjectREFR* FindBlockingDoor(RE::Actor* a_actor) const;
    /// Open a closed, unlocked, non-load door for a stuck NPC.
    bool TryOpenBlockingDoor(RE::Actor* a_actor, RE::TESObjectREFR* a_door);
    void TrackPlayerParkour();
    void ProcessDetection(RE::Actor* a_actor, ActorEntry& a_entry);
    bool TryFollowerReplay(RE::Actor* a_actor, ActorEntry& a_entry);
    void Unstick(RE::Actor* a_actor, ActorEntry& a_entry, bool a_teammate);
    bool TryEvgTraversal(RE::Actor* a_actor, ActorEntry& a_entry, const RE::NiPoint3& a_fwd);
    bool TryParkour(RE::Actor* a_actor, const RE::NiPoint3* a_fwdOverride = nullptr);
    bool TryTeleportBypass(RE::Actor* a_actor);
    void UpdateParkourJobs(float a_delta);
    void CancelParkourJobs(bool a_sendInterrupt);
    void CleanupEntries();

    // Cached keywords
    RE::BGSKeyword* kwNPC = nullptr;
    RE::BGSKeyword* kwCreature = nullptr;
    RE::BGSKeyword* kwDaedra = nullptr;
    RE::BGSKeyword* kwDragon = nullptr;
    RE::BGSKeyword* kwDwarven = nullptr;
    RE::BGSKeyword* kwGhost = nullptr;
    RE::BGSKeyword* kwAnimal = nullptr;
    bool            keywordsReady = false;

    std::unordered_map<RE::FormID, ActorEntry> entries;
    std::vector<ParkourJob>                    jobs;
    std::deque<PlayerParkourEvent>             playerEvents;

    double        now = 0.0;
    double        lastCleanup = 0.0;
    std::uint32_t rrIndex = 0;
    bool          playerWasParkouring = false;
    bool          wasEnabled = true;
    RE::FormID    playerLastFurnBase = 0;  // EVG furniture the player is currently using
};
