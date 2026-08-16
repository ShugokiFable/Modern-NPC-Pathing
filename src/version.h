#pragma once

#include <cstdint>

namespace PluginVersion
{
    inline constexpr std::uint16_t Major = 2;
    inline constexpr std::uint16_t Minor = 5;
    inline constexpr std::uint16_t Patch = 0;
    // Match REL::Version::pack(), used by SKSEPlugin_Version.
    // 2.4.5 / 2.4.6 / 2.4.7 all shipped the unmodified 2.4.4 DLL. 2.4.8 was the
    // first genuine native rebuild since 2.4.4, so Patch tracks String again.
    inline constexpr std::uint32_t Legacy =
        (static_cast<std::uint32_t>(Major) << 24) |
        (static_cast<std::uint32_t>(Minor) << 16) |
        (static_cast<std::uint32_t>(Patch) << 4);
    inline constexpr const char* String = "2.5.0";
}
