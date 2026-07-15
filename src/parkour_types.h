#pragma once
#include <cstdint>

/// Parkour types — must match SkyParkour V3's behavior graph state machine values
/// (ParkourType.h in the SkyParkour source).
enum class NpcParkourType : std::int32_t {
    NoLedge  = -1, Failed   = 0, Grab     = 1, Vault    = 2,
    StepLow  = 3,  StepHigh = 4, Low      = 5, Medium   = 6,
    High     = 7,  Highest  = 8
};

inline constexpr bool IsClimbType(NpcParkourType a_type)
{
    return a_type == NpcParkourType::Low || a_type == NpcParkourType::Medium ||
           a_type == NpcParkourType::High || a_type == NpcParkourType::Highest;
}

/// Height thresholds from SkyParkour's HardCodedVariables (Skyrim units).
/// Multiplied by actor scale at runtime.
namespace ParkourHeights {
    constexpr float climbMax    = 250.0f;
    constexpr float climbMin    = 30.0f;
    constexpr float vaultMax    = 120.0f;
    constexpr float vaultMin    = 42.0f;
    constexpr float highestLim  = 220.0f;
    constexpr float highLim     = 170.0f;
    constexpr float medLim      = 130.0f;
    constexpr float lowLim      = 80.0f;
    constexpr float highStepLim = 60.0f;

    // Ending elevation of each animation's root motion (SkyParkour HardCodedVariables).
    // The anim start position is placed ledge.z - elevation so root motion ends on the ledge.
    constexpr float highestElev  = 250.0f;
    constexpr float highElev     = 200.0f;
    constexpr float medElev      = 153.0f;
    constexpr float lowElev      = 110.0f;
    constexpr float stepHighElev = 70.0f;
    constexpr float stepLowElev  = 50.0f;
    constexpr float vaultElev    = 60.0f;
}

/// SkyParkour behavior graph variable / anim event names (BehaviorGraph.h in the SkyParkour source).
namespace SkyParkourGraph {
    // Anim events
    constexpr const char* NotifyStart = "SkyParkour";            // send to begin a parkour action
    constexpr const char* EvInterrupt = "SkyParkour_Interrupt";  // safe force-stop (never send SkyParkour_Stop from outside — recursion crash)

    // Graph variables
    constexpr const char* VarLedge        = "SkyParkourLedge";         // int, NpcParkourType
    constexpr const char* VarOngoing      = "SkyParkourOngoing";       // bool, owned by the graph — never write it
    constexpr const char* VarLowerBody    = "SkyParkourLowerBody";     // bool, lower-body-only steps when weapon out
    constexpr const char* VarTPPInstalled = "SkyParkourTPPInstalled";  // bool, third-person behavior patch present
}
