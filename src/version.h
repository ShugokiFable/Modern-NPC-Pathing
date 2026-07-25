#pragma once

#include <cstdint>

namespace PluginVersion
{
    inline constexpr std::uint16_t Major = 2;
    inline constexpr std::uint16_t Minor = 4;
    inline constexpr std::uint16_t Patch = 6;
    // Match REL::Version::pack(), used by SKSEPlugin_Version.
    // Package 2.4.6 ships the unmodified 2.4.4 DLL (no native rebuild).
    // Bump Major/Minor/Patch here when the DLL is recompiled next.
    inline constexpr std::uint32_t Legacy =
        (static_cast<std::uint32_t>(Major) << 24) |
        (static_cast<std::uint32_t>(Minor) << 16) |
        (static_cast<std::uint32_t>(Patch) << 4);
    inline constexpr const char* String = "2.4.6";
}
