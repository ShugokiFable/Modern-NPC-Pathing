#include "pathing.h"
#include "evg_traversal.h"
#include "npc_parkour.h"
#include "parkour_types.h"
#include "raycast.h"
#include "settings.h"

#include "RE/A/ActorState.h"
#include "RE/B/BGSOpenCloseForm.h"
#include "RE/B/bhkCharacterController.h"
#include "RE/B/bhkWorld.h"
#include "RE/E/ExtraTeleport.h"
#include "RE/C/CollisionLayers.h"
#include "RE/D/DialogueMenu.h"
#include "RE/F/FormTypes.h"
#include "RE/H/hkpCharacterState.h"
#include "RE/M/MenuTopicManager.h"
#include "RE/P/PlayerCharacter.h"
#include "RE/P/ProcessLists.h"
#include "RE/T/TESFaction.h"
#include "RE/T/TESForm.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/U/UI.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>

namespace
{
    // Flat, normalized facing direction. Skyrim heading: 0 = +Y (north), clockwise.
    RE::NiPoint3 YawToForward(float a_yaw)
    {
        return { std::sin(a_yaw), std::cos(a_yaw), 0.0f };
    }

    RE::NiPoint3 ForwardVector(RE::Actor* a_actor)
    {
        return YawToForward(a_actor->GetAngleZ());
    }

    // A ray that lands on an actor's body (the player or another NPC) is a
    // dynamic obstacle, not a navmesh failure — never a reason to reposition.
    bool IsActorHit(const Raycast::Result& a_hit)
    {
        switch (a_hit.layer) {
        case RE::COL_LAYER::kCharController:
        case RE::COL_LAYER::kBiped:
        case RE::COL_LAYER::kBipedNoCC:
        case RE::COL_LAYER::kDeadBip:
            return true;
        default:
            break;
        }
        return a_hit.hitRef && a_hit.hitRef->GetFormType() == RE::FormType::ActorCharacter;
    }

    // How close a follower must be to a recorded player parkour point to replay it.
    // 110 with the startPos-gap cap in TryParkour keeps the wrong-ledge/drag
    // problems away while letting followers catch moves they used to miss.
    constexpr float kReplayRadius = 110.0f;
    constexpr float kReplayZTolerance = 100.0f;
    constexpr float kReplayMinPlayerAbove = 40.0f;
    constexpr double kReplayEventLifetime = 120.0;  // seconds

    // Self-heal window after a traversal we initiated: any residual pitch/roll
    // found on the actor inside this window is contamination, not intent.
    constexpr double kPostureGuardWindow = 15.0;  // seconds
    constexpr float kPostureEpsilon = 0.02f;      // radians (~1.1 deg)

    // Per-frame tick: hook PlayerCharacter::Update (vfunc 0xAD). Runs on the main
    // thread once per frame during gameplay, paused in menus.
    struct PlayerUpdateHook
    {
        static void thunk(RE::PlayerCharacter* a_this, float a_delta)
        {
            func(a_this, a_delta);
            PathingManager::GetSingleton()->OnFrame(a_delta);
        }
        static inline REL::Relocation<decltype(thunk)> func;

        static void Install()
        {
            REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_PlayerCharacter[0] };
            func = vtbl.write_vfunc(0xAD, thunk);
            spdlog::info("NPCPathingNG: PlayerCharacter::Update hook installed");
        }
    };
}

void PathingManager::Install()
{
    auto* mgr = GetSingleton();
    mgr->CacheKeywords();
    PlayerUpdateHook::Install();
}

void PathingManager::CacheKeywords()
{
    if (keywordsReady) {
        return;
    }

    kwNPC      = RE::TESForm::LookupByID<RE::BGSKeyword>(0x00013794);  // ActorTypeNPC
    kwCreature = RE::TESForm::LookupByID<RE::BGSKeyword>(0x00013795);  // ActorTypeCreature
    kwDaedra   = RE::TESForm::LookupByID<RE::BGSKeyword>(0x00013797);  // ActorTypeDaedra
    kwDragon   = RE::TESForm::LookupByID<RE::BGSKeyword>(0x00035D59);  // ActorTypeDragon
    kwDwarven  = RE::TESForm::LookupByID<RE::BGSKeyword>(0x0001397A);  // ActorTypeDwarven
    kwGhost    = RE::TESForm::LookupByID<RE::BGSKeyword>(0x000D205E);  // ActorTypeGhost
    kwAnimal   = RE::TESForm::LookupByID<RE::BGSKeyword>(0x00013798);  // ActorTypeAnimal

    keywordsReady = (kwNPC != nullptr);
    if (keywordsReady) {
        spdlog::info("NPCPathingNG: keywords cached");
    } else {
        spdlog::error("NPCPathingNG: failed to cache ActorTypeNPC keyword");
    }
}

// Race/keyword identity: humanoid NPCs only (they share the patched behavior graphs).
bool PathingManager::PassesStaticFilter(RE::Actor* a_actor) const
{
    if (!a_actor || a_actor->IsPlayerRef()) {
        return false;
    }
    if (kwNPC && !a_actor->HasKeyword(kwNPC)) {
        return false;
    }
    if ((kwCreature && a_actor->HasKeyword(kwCreature)) ||
        (kwDaedra && a_actor->HasKeyword(kwDaedra)) ||
        (kwDragon && a_actor->HasKeyword(kwDragon)) ||
        (kwDwarven && a_actor->HasKeyword(kwDwarven)) ||
        (kwGhost && a_actor->HasKeyword(kwGhost)) ||
        (kwAnimal && a_actor->HasKeyword(kwAnimal))) {
        return false;
    }
    return true;
}

// Covers vanilla followers and framework followers (NFF etc. all set the teammate flag).
bool PathingManager::IsTeammate(RE::Actor* a_actor) const
{
    if (a_actor->IsPlayerTeammate()) {
        return true;
    }
    // Type-checked via Is() + static_cast, not LookupByID<TESFaction>: this
    // CommonLib build doesn't instantiate As<TESFaction>, so the templated
    // lookup fails to link (LNK2019).
    auto* form = RE::TESForm::LookupByID(0x0005C84E);  // CurrentFollowerFaction
    auto* followerFaction =
        (form && form->Is(RE::FormType::Faction)) ? static_cast<RE::TESFaction*>(form) : nullptr;
    return followerFaction && a_actor->IsInFaction(followerFaction);
}

// The single gate that separates a real navmesh wedge from every false
// positive (dialogue holds, scene actors, an enemy pressing into the player):
// there must be solid *static* geometry within a stride ahead.
bool PathingManager::IsGenuinelyWallStuck(RE::Actor* a_actor) const
{
    const RE::NiPoint3 pos = a_actor->GetPosition();
    const RE::NiPoint3 fwd = ForwardVector(a_actor);
    for (float h : { 20.0f, 70.0f }) {  // knee and chest
        const auto hit = Raycast::Cast(a_actor, pos + RE::NiPoint3(0.0f, 0.0f, h), fwd, 60.0f);
        if (hit.didHit && !IsActorHit(hit)) {
            return true;
        }
    }
    return false;
}

// A doorway is a pathing chokepoint, not a wall to sidestep. Sliding an NPC
// sideways out of a doorway is always wrong — it pushes them off the only
// route through. Find the door so Unstick can handle it properly instead.
RE::TESObjectREFR* PathingManager::FindBlockingDoor(RE::Actor* a_actor) const
{
    const RE::NiPoint3 pos = a_actor->GetPosition();
    const RE::NiPoint3 fwd = ForwardVector(a_actor);
    for (float h : { 20.0f, 70.0f, 110.0f }) {  // knee, chest, head
        const auto hit = Raycast::Cast(a_actor, pos + RE::NiPoint3(0.0f, 0.0f, h), fwd, 90.0f);
        if (hit.didHit && hit.hitRef && hit.hitRef->GetBaseObject() &&
            hit.hitRef->GetBaseObject()->Is(RE::FormType::Door)) {
            return hit.hitRef;
        }
    }
    return nullptr;
}

// Nudge a stuck NPC's door open. Deliberately narrow: never a load door (that
// would cell-transition the actor), never locked (NPCs must not bypass locks),
// and only when actually closed. SetOpenState is used rather than activation
// so no lock/trap/script side effects ride along.
bool PathingManager::TryOpenBlockingDoor(RE::Actor* a_actor, RE::TESObjectREFR* a_door)
{
    if (!a_door || a_door->IsDisabled() || a_door->IsDeleted()) {
        return false;
    }
    if (a_door->extraList.HasType<RE::ExtraTeleport>()) {
        return false;  // load door — opening it must stay the AI's decision
    }
    if (a_door->IsLocked()) {
        return false;
    }
    if (RE::BGSOpenCloseForm::GetOpenState(a_door) != RE::BGSOpenCloseForm::OPEN_STATE::kClosed) {
        return false;  // already open/opening — the block is something else
    }

    RE::BGSOpenCloseForm::SetOpenState(a_door, true, false);

    if (Settings::GetSingleton()->debugLogging) {
        const char* name = a_actor->GetName();
        spdlog::info("NPCPathingNG: opened blocking door {:08X} for {}",
                     a_door->GetFormID(), name ? name : "?");
    }
    return true;
}

bool PathingManager::InCombatNearPlayer(RE::Actor* a_actor) const
{
    if (!a_actor->IsInCombat()) {
        return false;
    }
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return false;
    }
    // ~melee-to-charge range; inside this an enemy that "stops moving" is
    // pressing the attack into the player, not wedged in the world.
    constexpr float kCombatHoldRange = 320.0f;
    return (a_actor->GetPosition() - player->GetPosition()).Length() < kCombatHoldRange;
}

// Cheap runtime conditions, re-evaluated every sample.
bool PathingManager::PassesDynamicFilter(RE::Actor* a_actor) const
{
    auto* settings = Settings::GetSingleton();

    if (settings->excludeInCombat && a_actor->IsInCombat()) {
        return false;
    }

    if (settings->excludeMounted) {
        RE::NiPointer<RE::Actor> mount;
        if (a_actor->GetMount(mount)) {
            return false;
        }
    }

    auto* state = a_actor->AsActorState();
    if (!state) {
        return false;
    }
    if (state->GetSitSleepState() != RE::SIT_SLEEP_STATE::kNormal) {
        return false;  // in furniture / sleeping
    }
    if (state->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal) {
        return false;  // ragdolled / getting up
    }
    if (state->IsSwimming()) {
        return false;
    }
    if (a_actor->IsInKillMove()) {
        return false;
    }

    auto* ctrl = a_actor->GetCharController();
    if (!ctrl || ctrl->context.currentState != RE::hkpCharacterStateType::kOnGround) {
        return false;  // airborne, jumping, flying
    }

    // Already parkouring (triggered by us or anything else)?
    bool ongoing = false;
    if (a_actor->GetGraphVariableBool(SkyParkourGraph::VarOngoing, ongoing) && ongoing) {
        return false;
    }

    // In dialogue with the player: the dialogue system holds the NPC in place
    // while their movement graph keeps "walking" — a classic false-stuck that
    // made conversation partners teleport around mid-sentence.
    if (auto* ui = RE::UI::GetSingleton(); ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME)) {
        if (auto* topicManager = RE::MenuTopicManager::GetSingleton()) {
            auto speaker = topicManager->speaker.get();
            if (speaker && speaker.get() == a_actor) {
                return false;
            }
            auto lastSpeaker = topicManager->lastSpeaker.get();
            if (lastSpeaker && lastSpeaker.get() == a_actor) {
                return false;
            }
        }
    }

    return true;
}

// Record the player's own SkyParkour moves and EVG marker uses so followers
// can retrace the route.
void PathingManager::TrackPlayerParkour()
{
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        return;
    }

    auto* settings = Settings::GetSingleton();

    // Drop event types as soon as their integration is disabled. Otherwise an
    // old move can wake back up when the toggle is re-enabled within the event
    // lifetime and make a follower replay stale traversal history.
    for (auto it = playerEvents.begin(); it != playerEvents.end();) {
        const bool isEvg = static_cast<bool>(it->furnRef);
        if ((isEvg && !settings->enableEvgTraversal) ||
            (!isEvg && !settings->enableParkour)) {
            it = playerEvents.erase(it);
        } else {
            ++it;
        }
    }

    bool ongoing = false;
    if (settings->enableParkour) {
        player->GetGraphVariableBool(SkyParkourGraph::VarOngoing, ongoing);

        if (ongoing && !playerWasParkouring) {
            PlayerParkourEvent ev;
            ev.pos = player->GetPosition();
            ev.yaw = player->GetAngleZ();
            ev.time = now;
            playerEvents.push_back(std::move(ev));
            while (playerEvents.size() > 16) {
                playerEvents.pop_front();
            }
        }
    }
    playerWasParkouring = ongoing;

    // EVG traversal: fires when the player enters one of the marker furnitures.
    // Only worth recording while followers can actually reuse the marker.
    if (settings->enableEvgTraversal && EvgTraversal::IsNpcUseSupported()) {
        RE::FormID furnBase = 0;
        auto furnHandle = player->GetOccupiedFurniture();
        auto furnPtr = furnHandle.get();
        if (furnPtr && EvgTraversal::IsTraversalFurniture(furnPtr->GetBaseObject())) {
            furnBase = furnPtr->GetBaseObject()->GetFormID();
        }
        if (furnBase && furnBase != playerLastFurnBase) {
            PlayerParkourEvent ev;
            ev.pos = furnPtr->GetPosition();
            ev.yaw = furnPtr->GetAngleZ();
            ev.time = now;
            ev.furnRef = furnPtr->GetHandle();
            playerEvents.push_back(std::move(ev));
            while (playerEvents.size() > 16) {
                playerEvents.pop_front();
            }
        }
        playerLastFurnBase = furnBase;
    } else {
        playerLastFurnBase = 0;
    }
}

// Follower near a spot where the player parkoured, player now above them:
// face the same way and do the same move.
bool PathingManager::TryFollowerReplay(RE::Actor* a_actor, ActorEntry& a_entry)
{
    auto* settings = Settings::GetSingleton();
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player || playerEvents.empty()) {
        return false;
    }

    const RE::NiPoint3 pos = a_actor->GetPosition();
    if (!PassesDynamicFilter(a_actor)) {
        return false;
    }

    const RE::FormID fid = a_actor->GetFormID();
    const float playerAbove = player->GetPositionZ() - pos.z;

    for (auto& ev : playerEvents) {
        if (now - ev.time > kReplayEventLifetime || ev.consumed.contains(fid)) {
            continue;
        }
        const bool isEvg = static_cast<bool>(ev.furnRef);
        if ((isEvg && !settings->enableEvgTraversal) ||
            (!isEvg && !settings->enableParkour)) {
            continue;
        }
        // SkyParkour replays only make sense while the player is above the
        // follower; EVG markers (squeezes, ladders, drops) replay regardless.
        if (!isEvg && playerAbove < kReplayMinPlayerAbove) {
            continue;
        }
        const float dx = pos.x - ev.pos.x;
        const float dy = pos.y - ev.pos.y;
        if (dx * dx + dy * dy > kReplayRadius * kReplayRadius) {
            continue;
        }
        if (std::abs(pos.z - ev.pos.z) > kReplayZTolerance) {
            continue;
        }

        if (isEvg) {
            if (!EvgTraversal::IsNpcUseSupported()) {
                continue;  // engine won't let NPCs enter the furniture; don't stall on it
            }
            auto markerPtr = ev.furnRef.get();
            if (!markerPtr) {
                continue;
            }
            if (EvgTraversal::Use(a_actor, markerPtr.get())) {
                ev.consumed.insert(fid);
                a_entry.stuckCount = 0;
                a_entry.lastEvgTime = now;
                a_entry.cooldownUntil = now + Settings::GetSingleton()->cooldown;
                if (settings->debugLogging) {
                    const char* name = a_actor->GetName();
                    spdlog::info("NPCPathingNG: follower {} using EVG marker after player", name ? name : "?");
                }
                return true;
            }
            continue;
        }

        // Attempt the same move the player made. TryParkour only touches the
        // follower's facing if a valid ledge is actually found — failed probes
        // must never fight the AI's own rotation (caused visible twitching).
        const RE::NiPoint3 fwd = YawToForward(ev.yaw);
        if (TryParkour(a_actor, &fwd)) {
            ev.consumed.insert(fid);
            a_entry.stuckCount = 0;
            a_entry.cooldownUntil = now + Settings::GetSingleton()->cooldown * 0.5f;
            if (settings->debugLogging) {
                const char* name = a_actor->GetName();
                spdlog::info("NPCPathingNG: follower {} replaying player parkour", name ? name : "?");
            }
            return true;
        }
        // No valid ledge from the follower's exact spot — leave the event
        // unconsumed so they can retry as they get closer.
    }
    return false;
}

void PathingManager::ProcessDetection(RE::Actor* a_actor, ActorEntry& a_entry)
{
    auto* settings = Settings::GetSingleton();

    if (a_entry.lastCheckTime >= 0.0 && now - a_entry.lastCheckTime < settings->checkInterval) {
        return;
    }

    const RE::NiPoint3 pos = a_actor->GetPosition();
    const bool firstSample = a_entry.lastCheckTime < 0.0;
    const RE::NiPoint3 prev = a_entry.lastPos;
    a_entry.lastCheckTime = now;
    a_entry.lastPos = pos;

    if (firstSample) {
        return;
    }

    if (!a_entry.filterChecked) {
        a_entry.filterChecked = true;
        a_entry.passesFilter = PassesStaticFilter(a_actor);
    }
    if (!a_entry.passesFilter) {
        return;
    }

    if (a_actor->IsDead()) {
        a_entry.stuckCount = 0;
        return;
    }

    // Posture guard: after a traversal we initiated (SkyParkour job or EVG
    // furniture), any leftover pitch/roll is contamination — the engine rights
    // the player from camera input every frame but never rights NPCs, so a
    // vault that ends tilted leaves them walking diagonally forever. Runs
    // during the post-traversal cooldown on purpose; never in combat (ranged
    // NPCs aim with pitch) and never mid-move.
    const double lastTraversal = std::max(a_entry.lastEvgTime, a_entry.lastTraversalEnd);
    if (now - lastTraversal < kPostureGuardWindow && !a_actor->IsInCombat()) {
        bool ongoing = false;
        a_actor->GetGraphVariableBool(SkyParkourGraph::VarOngoing, ongoing);
        auto* state = a_actor->AsActorState();
        auto* ctrl = a_actor->GetCharController();
        const bool settled = !ongoing && state && ctrl &&
                             state->GetSitSleepState() == RE::SIT_SLEEP_STATE::kNormal &&
                             state->GetKnockState() == RE::KNOCK_STATE_ENUM::kNormal &&
                             ctrl->context.currentState == RE::hkpCharacterStateType::kOnGround;
        if (settled && (std::abs(a_actor->data.angle.x) > kPostureEpsilon ||
                        std::abs(a_actor->data.angle.y) > kPostureEpsilon ||
                        std::abs(ctrl->pitchAngle) > kPostureEpsilon ||
                        std::abs(ctrl->rollAngle) > kPostureEpsilon)) {
            a_actor->data.angle.x = 0.0f;
            a_actor->data.angle.y = 0.0f;
            ctrl->pitchAngle = 0.0f;
            ctrl->rollAngle = 0.0f;
            if (settings->debugLogging) {
                const char* name = a_actor->GetName();
                spdlog::info("NPCPathingNG: posture guard righted {}", name ? name : "?");
            }
        }
    }

    if (now < a_entry.cooldownUntil) {
        a_entry.stuckCount = 0;
        return;
    }

    const bool teammate = IsTeammate(a_actor);
    if (teammate && settings->excludeFollowers) {
        a_entry.stuckCount = 0;
        return;
    }

    // Followers first try to retrace the player's parkour route — this fires
    // even when they're standing still, confused by a failed path.
    if (teammate && settings->followerReplay &&
        (settings->enableParkour || settings->enableEvgTraversal) &&
        TryFollowerReplay(a_actor, a_entry)) {
        return;
    }

    // The core gate: only NPCs whose behavior graph is actually trying to move
    // can be "stuck". Idle guards and sandboxing NPCs never trip this.
    if (!a_actor->IsMoving()) {
        a_entry.stuckCount = 0;
        return;
    }

    if (!PassesDynamicFilter(a_actor)) {
        a_entry.stuckCount = 0;
        return;
    }

    const float moved = (pos - prev).Length();
    if (moved < settings->stuckDistance) {
        a_entry.stuckCount++;
    } else {
        // Moving normally again — fully recovered.
        a_entry.stuckCount = 0;
        a_entry.escalation = 0;
    }

    // Followers react twice as fast and retry sooner — losing the player's
    // trail is the thing this mod exists to fix.
    const int threshold = teammate ? std::max(1, settings->stuckThreshold / 2)
                                   : settings->stuckThreshold;
    const float cooldown = teammate ? settings->cooldown * 0.5f : settings->cooldown;

    if (a_entry.stuckCount >= threshold) {
        a_entry.stuckCount = 0;
        a_entry.cooldownUntil = now + cooldown;
        a_entry.escalation++;  // Unstick resets this to 0 on a successful traversal
        Unstick(a_actor, a_entry, teammate);
    }
}

void PathingManager::Unstick(RE::Actor* a_actor, ActorEntry& a_entry, bool a_teammate)
{
    (void)a_teammate;
    auto* settings = Settings::GetSingleton();

    // 1) Animated traversal is always the goal — try it first, every time.
    //    EVG markers are hand-placed where pathing breaks; parkour self-gates
    //    on real ledge geometry, so it can't fire in open space.
    if (settings->enableEvgTraversal &&
        TryEvgTraversal(a_actor, a_entry, ForwardVector(a_actor))) {
        a_entry.escalation = 0;
        return;
    }
    if (settings->enableParkour && TryParkour(a_actor)) {
        a_entry.escalation = 0;
        return;
    }

    // 1b) Doorways are chokepoints, not walls. Never sidestep an NPC out of a
    //     doorway — that pushes them off the only route through and is a likely
    //     cause of "NPCs stuck at doors". Give the door a nudge if it is simply
    //     shut, then let their own pathing take them through either way.
    if (auto* door = FindBlockingDoor(a_actor)) {
        if (TryOpenBlockingDoor(a_actor, door)) {
            a_entry.escalation = 0;
        }
        return;
    }

    // 2) Teleport is the last resort, and only for an NPC that is genuinely
    //    wedged against static geometry and has stayed that way across several
    //    stuck triggers (so animated traversal really had its chance). An enemy
    //    pressing into the player, or anyone held by AI/dialogue in open space,
    //    never qualifies — this is what made NPCs "teleport more than traverse".
    if (settings->enableTeleportFallback &&
        a_entry.escalation >= settings->teleportEscalation &&
        !InCombatNearPlayer(a_actor) &&
        IsGenuinelyWallStuck(a_actor) &&
        TryTeleportBypass(a_actor)) {
        a_entry.escalation = 0;
    }
}

bool PathingManager::TryEvgTraversal(RE::Actor* a_actor, ActorEntry& a_entry, const RE::NiPoint3& a_fwd)
{
    if (!EvgTraversal::IsNpcUseSupported()) {
        return false;
    }
    // If a recent activation didn't get this actor moving, let the other
    // unstick methods have their turn instead of hammering the same marker.
    if (now - a_entry.lastEvgTime < 20.0) {
        return false;
    }

    auto* marker = EvgTraversal::FindMarkerNear(a_actor, a_fwd, 250.0f);
    if (!marker || !EvgTraversal::Use(a_actor, marker)) {
        return false;
    }

    a_entry.lastEvgTime = now;
    if (Settings::GetSingleton()->debugLogging) {
        const char* name = a_actor->GetName();
        spdlog::info("NPCPathingNG: {} sent through EVG marker {:08X}",
                     name ? name : "?", marker->GetFormID());
    }
    return true;
}

bool PathingManager::TryParkour(RE::Actor* a_actor, const RE::NiPoint3* a_fwdOverride)
{
    auto* settings = Settings::GetSingleton();

    if (!NpcParkour::HasBehaviorPatch(a_actor)) {
        return false;
    }

    auto* cell = a_actor->GetParentCell();
    const bool interior = cell && cell->IsInteriorCell();
    if (interior && settings->parkourIndoorMode == 0) {
        return false;
    }

    const RE::NiPoint3 fwd = a_fwdOverride ? *a_fwdOverride : ForwardVector(a_actor);
    const auto det = NpcParkour::Detect(a_actor, fwd, settings->maxClimbHeight);
    if (det.type == NpcParkourType::NoLedge) {
        return false;
    }
    if (interior && settings->parkourIndoorMode == 1 && IsClimbType(det.type)) {
        return false;  // no wall-climbing inside houses
    }

    // If the anim start position is far away, the alignment lerp visibly
    // drags the actor across the ground. Wait until they're closer.
    if ((det.startPos - a_actor->GetPosition()).Length() > 90.0f) {
        return false;
    }

    // Only turn the actor once the move is definitely happening.
    if (a_fwdOverride) {
        a_actor->data.angle.z = std::atan2(a_fwdOverride->x, a_fwdOverride->y);
    }

    if (!NpcParkour::Trigger(a_actor, det)) {
        return false;
    }

    ParkourJob job;
    job.handle = a_actor->GetHandle();
    job.phase = ParkourJob::Phase::WaitingForGraph;
    job.phaseStart = now;
    job.startPos = det.startPos;
    jobs.push_back(job);

    if (settings->debugLogging) {
        const char* name = a_actor->GetName();
        spdlog::info("NPCPathingNG: parkour type {} triggered on {} (ledge {:.0f},{:.0f},{:.0f})",
                     static_cast<int>(det.type), name ? name : "?",
                     det.ledgePoint.x, det.ledgePoint.y, det.ledgePoint.z);
    }
    return true;
}

bool PathingManager::TryTeleportBypass(RE::Actor* a_actor)
{
    auto* settings = Settings::GetSingleton();
    const RE::NiPoint3 pos = a_actor->GetPosition();
    const float yaw = a_actor->GetAngleZ();

    // Caller (Unstick) has already confirmed this NPC is genuinely wall-stuck,
    // not in combat near the player, and has failed animated traversal
    // repeatedly. Find a validated sidestep around the obstacle.

    // Sideways-biased candidate directions, degrees relative to facing.
    constexpr float candidates[] = { 50.0f, -50.0f, 90.0f, -90.0f, 130.0f, -130.0f };
    constexpr float degToRad = 0.017453292f;

    for (float dist : { settings->snapDistance, settings->snapDistance * 0.6f }) {
        for (float degrees : candidates) {
            const float angle = yaw + degrees * degToRad;
            const RE::NiPoint3 dir(std::sin(angle), std::cos(angle), 0.0f);

            // Sweep a conservative body-width corridor at knee and chest height.
            // A clear center ray alone can still clip the NPC's shoulders through
            // a corner or narrow prop.
            constexpr float kBodyRadius = 28.0f;
            const RE::NiPoint3 side(-dir.y, dir.x, 0.0f);
            bool pathBlocked = false;
            for (float height : { 20.0f, 70.0f }) {
                for (float lateral : { -kBodyRadius, 0.0f, kBodyRadius }) {
                    const RE::NiPoint3 start = pos + RE::NiPoint3(0.0f, 0.0f, height) + side * lateral;
                    if (Raycast::Cast(a_actor, start, dir, dist + 20.0f).didHit) {
                        pathBlocked = true;
                        break;
                    }
                }
                if (pathBlocked) {
                    break;
                }
            }
            if (pathBlocked) {
                continue;
            }

            // Ground-snap the destination.
            const RE::NiPoint3 candidate = pos + dir * dist;
            const RE::NiPoint3 downStart = candidate + RE::NiPoint3(0.0f, 0.0f, 100.0f);
            const RE::NiPoint3 down(0.0f, 0.0f, -1.0f);
            const auto ground = Raycast::Cast(a_actor, downStart, down, 200.0f);
            if (!ground.didHit || ground.normalZ < 0.45f) {
                continue;  // no floor or too steep
            }
            const float groundZ = downStart.z - ground.distance;
            if (std::abs(groundZ - pos.z) > 100.0f) {
                continue;  // big drop or rise — don't yeet NPCs off cliffs
            }

            if (IsActorHit(ground)) {
                continue;  // never use another actor as the destination floor
            }

            // Headroom and capsule-width clearance at the destination. A clear
            // travel ray alone is not enough if the endpoint is inside a narrow
            // corner, prop, or another collision hull.
            const RE::NiPoint3 headroomStart(candidate.x, candidate.y, groundZ + 10.0f);
            const RE::NiPoint3 up(0.0f, 0.0f, 1.0f);
            if (Raycast::Cast(a_actor, headroomStart, up, 110.0f).didHit) {
                continue;
            }

            constexpr float kDiagonal = 0.70710678f;
            constexpr RE::NiPoint3 clearanceDirs[] = {
                { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f },
                { kDiagonal, kDiagonal, 0.0f }, { -kDiagonal, kDiagonal, 0.0f },
                { kDiagonal, -kDiagonal, 0.0f }, { -kDiagonal, -kDiagonal, 0.0f }
            };
            bool destinationBlocked = false;
            for (float height : { 25.0f, 70.0f }) {
                const RE::NiPoint3 center(candidate.x, candidate.y, groundZ + height);
                for (const auto& clearanceDir : clearanceDirs) {
                    const auto clearance = Raycast::Cast(a_actor, center, clearanceDir, kBodyRadius);
                    if (clearance.didHit) {
                        destinationBlocked = true;
                        break;
                    }
                }
                if (destinationBlocked) {
                    break;
                }
            }
            if (destinationBlocked) {
                continue;
            }

            RE::NiPoint3 dest(candidate.x, candidate.y, groundZ + 2.0f);
            a_actor->SetPosition(dest, true);

            if (settings->debugLogging) {
                const char* name = a_actor->GetName();
                spdlog::info("NPCPathingNG: bypass teleport {} {:+.0f}deg -> ({:.0f},{:.0f},{:.0f})",
                             name ? name : "?", degrees, dest.x, dest.y, dest.z);
            }
            return true;
        }
    }

    return false;  // fully boxed in — do nothing rather than clip through walls
}

void PathingManager::UpdateParkourJobs(float a_delta)
{
    for (auto it = jobs.begin(); it != jobs.end();) {
        bool remove = false;

        auto actorPtr = it->handle.get();
        RE::Actor* actor = actorPtr ? actorPtr.get() : nullptr;

        if (!actor || !actor->Is3DLoaded() || actor->IsDead()) {
            if (actor) {
                NpcParkour::OnParkourEnd(actor, false);
            }
            remove = true;
        } else {
            bool ongoing = false;
            actor->GetGraphVariableBool(SkyParkourGraph::VarOngoing, ongoing);

            // Ragdoll interrupts everything; the graph resets itself.
            const bool ragdolled =
                actor->AsActorState()->GetKnockState() != RE::KNOCK_STATE_ENUM::kNormal;

            switch (it->phase) {
            case ParkourJob::Phase::WaitingForGraph:
                if (ragdolled) {
                    NpcParkour::OnParkourEnd(actor, false);
                    remove = true;
                } else if (ongoing) {
                    NpcParkour::OnParkourStart(actor);
                    it->totalMissing = it->startPos - actor->GetPosition();
                    it->missingGap = it->totalMissing.Length();
                    it->phase = ParkourJob::Phase::Ongoing;
                    it->phaseStart = now;
                } else if (now - it->phaseStart > 1.0) {
                    // NotifyAnimationGraph accepted the request but the graph never
                    // entered the state. Explicitly cancel any delayed transition.
                    NpcParkour::OnParkourEnd(actor, true);
                    remove = true;
                }
                break;

            case ParkourJob::Phase::Ongoing:
                if (ragdolled) {
                    NpcParkour::OnParkourEnd(actor, false);
                    remove = true;
                    break;
                }
                const bool timedOut = now - it->phaseStart > 8.0;
                if (!ongoing || timedOut) {
                    // Natural graph completion needs cleanup, not a forced interrupt.
                    NpcParkour::OnParkourEnd(actor, timedOut);
                    remove = true;
                    break;
                }
                // The vanilla follow package can catch-up-teleport the actor
                // mid-animation; never drag them back across the map.
                if ((it->startPos - actor->GetPosition()).Length() > 400.0f) {
                    NpcParkour::OnParkourEnd(actor, true);
                    remove = true;
                    break;
                }
                // Smoothly close the gap to the anim start position (~0.35s),
                // like SkyParkour's Havok channel does for the player.
                if (it->missingGap > 0.1f) {
                    const float frac = std::min(a_delta / 0.35f, 1.0f);
                    RE::NiPoint3 nudge = it->totalMissing * frac;
                    const float nudgeLen = nudge.Length();
                    if (nudgeLen >= it->missingGap) {
                        nudge = it->totalMissing * (it->missingGap / it->totalMissing.Length());
                        it->missingGap = 0.0f;
                    } else {
                        it->missingGap -= nudgeLen;
                    }
                    if (auto* ctrl = actor->GetCharController()) {
                        RE::hkVector4 cur;
                        ctrl->GetPositionImpl(cur, true);
                        const float ws = RE::bhkWorld::GetWorldScale();
                        const RE::hkVector4 hkNudge(nudge.x * ws, nudge.y * ws, nudge.z * ws, 0.0f);
                        ctrl->SetPositionImpl(cur + hkNudge, true, false);
                    }
                }
                break;
            }
        }

        if (remove && actor) {
            // Open the posture-guard window: the next detection samples will
            // right any pitch/roll the traversal left behind.
            entries[actor->GetFormID()].lastTraversalEnd = now;
        }

        it = remove ? jobs.erase(it) : std::next(it);
    }
}

void PathingManager::CancelParkourJobs(bool a_sendInterrupt)
{
    for (auto& job : jobs) {
        if (auto actorPtr = job.handle.get()) {
            NpcParkour::OnParkourEnd(actorPtr.get(), a_sendInterrupt);
        }
    }
    jobs.clear();
}

void PathingManager::CleanupEntries()
{
    constexpr double staleAfter = 30.0;  // seconds unseen
    for (auto it = entries.begin(); it != entries.end();) {
        if (now - it->second.lastSeenTime > staleAfter) {
            it = entries.erase(it);
        } else {
            ++it;
        }
    }
    while (!playerEvents.empty() && now - playerEvents.front().time > kReplayEventLifetime) {
        playerEvents.pop_front();
    }
}

void PathingManager::OnFrame(float a_delta)
{
    auto* settings = Settings::GetSingleton();
    settings->Refresh();  // MCM globals -> members; no-op without the ESP
    now += a_delta;

    if (!settings->enabled) {
        // Disabling during a climb must restore controller simulation and graph state.
        if (wasEnabled) {
            CancelParkourJobs(true);
            entries.clear();
            playerEvents.clear();
            playerWasParkouring = false;
            playerLastFurnBase = 0;
        }
        wasEnabled = false;
        return;
    }
    wasEnabled = true;

    if (!keywordsReady) {
        return;
    }

    // Skip everything during loads, transitions and the main menu — half the
    // world singletons (TES::worldSpace and friends) are in flux then.
    auto* player = RE::PlayerCharacter::GetSingleton();
    if (!player || !player->Is3DLoaded() || !player->GetParentCell()) {
        return;
    }

    if (settings->followerReplay &&
        (settings->enableParkour || settings->enableEvgTraversal)) {
        TrackPlayerParkour();
    } else {
        playerEvents.clear();
        playerWasParkouring = false;
        playerLastFurnBase = 0;
    }

    // Actors mid-parkour need attention every frame.
    if (!jobs.empty()) {
        UpdateParkourJobs(a_delta);
    }

    auto* processLists = RE::ProcessLists::GetSingleton();
    if (!processLists) {
        return;
    }
    auto& handles = processLists->highActorHandles;
    const auto count = static_cast<std::uint32_t>(handles.size());
    if (count == 0) {
        return;
    }

    // Round-robin detection slice. Snapshot the handles first: our unstick
    // actions (teleport, activate, anim events) can make the engine grow the
    // handle array mid-loop, which would leave us iterating freed memory.
    const std::uint32_t slice = std::min<std::uint32_t>(settings->actorsPerFrame, count);
    RE::ActorHandle sliceHandles[100];
    for (std::uint32_t i = 0; i < slice; i++) {
        sliceHandles[i] = handles[rrIndex % count];
        rrIndex++;
    }
    for (std::uint32_t i = 0; i < slice; i++) {
        auto actorPtr = sliceHandles[i].get();
        if (!actorPtr || !actorPtr->Is3DLoaded()) {
            continue;
        }
        RE::Actor* actor = actorPtr.get();

        auto& entry = entries[actor->GetFormID()];
        entry.lastSeenTime = now;
        ProcessDetection(actor, entry);
    }

    if (now - lastCleanup > 10.0) {
        lastCleanup = now;
        CleanupEntries();
    }
}

void PathingManager::Reset()
{
    CancelParkourJobs(false);
    entries.clear();
    playerEvents.clear();
    playerWasParkouring = false;
    playerLastFurnBase = 0;
    wasEnabled = Settings::GetSingleton()->enabled;
    rrIndex = 0;
    spdlog::debug("NPCPathingNG: state reset");
}
