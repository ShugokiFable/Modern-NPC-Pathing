#include "settings.h"

#include "RE/T/TESDataHandler.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <string>
#include <unordered_map>

namespace
{
    constexpr const char* INI_PATH = "Data/SKSE/Plugins/NPCPathingNG.ini";
    constexpr const char* PLUGIN_FILE = "NPCPathingNG.esp";

    std::string Trim(const std::string& a_str)
    {
        const auto begin = a_str.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            return {};
        }
        const auto end = a_str.find_last_not_of(" \t\r\n");
        return a_str.substr(begin, end - begin + 1);
    }

    std::string ToLower(std::string a_str)
    {
        std::transform(a_str.begin(), a_str.end(), a_str.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return a_str;
    }

    // Single-pass INI parse: "section/key" (lowercased) -> value.
    std::unordered_map<std::string, std::string> ParseINI(const char* a_path)
    {
        std::unordered_map<std::string, std::string> values;
        std::ifstream file(a_path);
        if (!file.is_open()) {
            return values;
        }

        std::string line;
        std::string section;
        while (std::getline(file, line)) {
            line = Trim(line);
            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue;
            }
            if (line.front() == '[' && line.back() == ']') {
                section = ToLower(Trim(line.substr(1, line.size() - 2)));
                continue;
            }
            const auto eq = line.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            const auto key = ToLower(Trim(line.substr(0, eq)));
            values[section + "/" + key] = Trim(line.substr(eq + 1));
        }
        return values;
    }

    void WriteDefaultINI()
    {
        std::ofstream f(INI_PATH);
        if (!f.is_open()) {
            spdlog::warn("NPCPathingNG: could not create default INI at {}", INI_PATH);
            return;
        }
        f << "; NPC Pathing NG — fallback settings.\n"
             "; If NPCPathingNG.esp is enabled and you have MCM Helper, use the in-game\n"
             "; MCM instead — MCM values override this file and apply instantly.\n"
             "\n"
             "[General]\n"
             "bEnabled=1\n"
             "; Seconds between position samples per NPC\n"
             "fCheckInterval=0.25\n"
             "; Consecutive stuck samples before acting (interval * threshold = reaction time)\n"
             "iStuckThreshold=4\n"
             "; Moved less than this many units per sample while trying to move = stuck\n"
             "fStuckDistance=4.0\n"
             "; Seconds before the same NPC can be unstuck again\n"
             "fCooldown=3.0\n"
             "; NPCs scanned per frame (round-robin)\n"
             "iActorsPerFrame=10\n"
             "\n"
             "[Parkour]\n"
             "; Vault/climb via SkyParkour behavior patches (needs SkyParkour V3 + Nemesis/Pandora)\n"
             "bEnableParkour=1\n"
             "; Indoors: 0 = no parkour (default), 1 = steps and vaults only, 2 = everything\n"
             "iIndoorMode=0\n"
             "; Max climb height in game units. 130 (default) = steps, vaults and low/chest ledges\n"
             "; only; NPCs won't scale walls, houses or mountainsides. Raise toward 250 (SkyParkour's\n"
             "; own max) for full cliff/mountain climbing.\n"
             "fMaxClimbHeight=130.0\n"
             "; NPCs use EVG Animated Traversal markers (ladders, squeezes, ledges...) if installed\n"
             "bEnableEVGTraversal=1\n"
             "\n"
             "[Followers]\n"
             "; Followers reproduce the player's parkour moves to keep up (Nether's FF compatible)\n"
             "bFollowerReplay=1\n"
             "\n"
             "[Avoidance]\n"
             "; Teleport-around-obstacle fallback, used only as a LAST resort when animated\n"
             "; traversal can't help. Set 0 to disable teleporting entirely.\n"
             "bEnableTeleportFallback=1\n"
             "; How far sideways the bypass teleport goes\n"
             "fSnapDistance=100.0\n"
             "; How many times an NPC must trigger stuck (with no parkour/EVG escape) before a\n"
             "; teleport is allowed. Higher = teleport is rarer and more of a true last resort.\n"
             "iTeleportEscalation=3\n"
             "\n"
             "[Filters]\n"
             "; 0 = NPCs in combat ARE processed (guards/foes climb after you)\n"
             "bExcludeInCombat=0\n"
             "; 0 = followers ARE processed\n"
             "bExcludeFollowers=0\n"
             "bExcludeMounted=1\n"
             "\n"
             "[Debug]\n"
             "; Log every unstuck event to NPCPathingNG.log\n"
             "bDebugLogging=0\n";
        spdlog::info("NPCPathingNG: wrote default INI");
    }
}

void Settings::Load()
{
    auto values = ParseINI(INI_PATH);
    if (values.empty()) {
        WriteDefaultINI();
        return;  // defaults already in members
    }

    auto getBool = [&](const char* a_key, bool a_def) {
        auto it = values.find(a_key);
        return it != values.end() ? (std::atoi(it->second.c_str()) != 0) : a_def;
    };
    auto getInt = [&](const char* a_key, int a_def) {
        auto it = values.find(a_key);
        return it != values.end() ? std::atoi(it->second.c_str()) : a_def;
    };
    auto getFloat = [&](const char* a_key, float a_def) {
        auto it = values.find(a_key);
        return it != values.end() ? static_cast<float>(std::atof(it->second.c_str())) : a_def;
    };

    enabled = getBool("general/benabled", enabled);
    checkInterval = std::max(0.05f, getFloat("general/fcheckinterval", checkInterval));
    stuckThreshold = std::max(1, getInt("general/istuckthreshold", stuckThreshold));
    stuckDistance = std::max(0.5f, getFloat("general/fstuckdistance", stuckDistance));
    cooldown = std::max(0.5f, getFloat("general/fcooldown", cooldown));
    actorsPerFrame = std::clamp(getInt("general/iactorsperframe", actorsPerFrame), 1, 100);

    enableParkour = getBool("parkour/benableparkour", enableParkour);
    parkourIndoorMode = std::clamp(getInt("parkour/iindoormode", parkourIndoorMode), 0, 2);
    maxClimbHeight = std::clamp(getFloat("parkour/fmaxclimbheight", maxClimbHeight), 60.0f, 250.0f);
    enableEvgTraversal = getBool("parkour/benableevgtraversal", enableEvgTraversal);
    teleportEscalation = std::clamp(getInt("avoidance/iteleportescalation", teleportEscalation), 1, 10);

    followerReplay = getBool("followers/bfollowerreplay", followerReplay);

    enableTeleportFallback = getBool("avoidance/benableteleportfallback", enableTeleportFallback);
    snapDistance = std::clamp(getFloat("avoidance/fsnapdistance", snapDistance), 40.0f, 300.0f);

    excludeInCombat = getBool("filters/bexcludeincombat", excludeInCombat);
    excludeFollowers = getBool("filters/bexcludefollowers", excludeFollowers);
    excludeMounted = getBool("filters/bexcludemounted", excludeMounted);

    debugLogging = getBool("debug/bdebuglogging", debugLogging);

    spdlog::info("NPCPathingNG: INI loaded — enabled={}, parkour={}, indoorMode={}, followers={}, combat={}",
                 enabled, enableParkour, parkourIndoorMode, !excludeFollowers, !excludeInCombat);
}

void Settings::BindGlobals()
{
    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler) {
        return;
    }

    auto lookup = [&](RE::FormID a_localID) -> RE::TESGlobal* {
        auto* form = dataHandler->LookupForm(a_localID, PLUGIN_FILE);
        return form ? static_cast<RE::TESGlobal*>(form) : nullptr;
    };

    gEnabled          = lookup(0x800);
    gCheckInterval    = lookup(0x801);
    gStuckThreshold   = lookup(0x802);
    gStuckDistance    = lookup(0x803);
    gCooldown         = lookup(0x804);
    gActorsPerFrame   = lookup(0x805);
    gEnableParkour    = lookup(0x806);
    gIndoorMode       = lookup(0x807);
    gMaxClimbHeight   = lookup(0x808);
    gTeleportFallback = lookup(0x809);
    gSnapDistance     = lookup(0x80A);
    gExcludeInCombat  = lookup(0x80B);
    gExcludeFollowers = lookup(0x80C);
    gExcludeMounted   = lookup(0x80D);
    gFollowerReplay   = lookup(0x80E);
    gDebugLogging     = lookup(0x80F);
    gEvgTraversal     = lookup(0x811);
    gTeleportEscalation = lookup(0x812);

    if (gEnabled) {
        spdlog::info("NPCPathingNG: {} found — settings driven by MCM globals", PLUGIN_FILE);
        Refresh();
    } else {
        spdlog::info("NPCPathingNG: {} not present — using INI settings", PLUGIN_FILE);
    }
}

void Settings::Refresh()
{
    if (!gEnabled) {
        return;  // no ESP — INI values stay
    }

    auto asBool = [](RE::TESGlobal* g, bool cur) { return g ? g->value != 0.0f : cur; };
    auto asInt = [](RE::TESGlobal* g, int cur) { return g ? static_cast<int>(g->value) : cur; };
    auto asFloat = [](RE::TESGlobal* g, float cur) { return g ? g->value : cur; };

    enabled = asBool(gEnabled, enabled);
    checkInterval = std::max(0.05f, asFloat(gCheckInterval, checkInterval));
    stuckThreshold = std::max(1, asInt(gStuckThreshold, stuckThreshold));
    stuckDistance = std::max(0.5f, asFloat(gStuckDistance, stuckDistance));
    cooldown = std::max(0.5f, asFloat(gCooldown, cooldown));
    actorsPerFrame = std::clamp(asInt(gActorsPerFrame, actorsPerFrame), 1, 100);

    enableParkour = asBool(gEnableParkour, enableParkour);
    parkourIndoorMode = std::clamp(asInt(gIndoorMode, parkourIndoorMode), 0, 2);
    maxClimbHeight = std::clamp(asFloat(gMaxClimbHeight, maxClimbHeight), 60.0f, 250.0f);

    enableTeleportFallback = asBool(gTeleportFallback, enableTeleportFallback);
    snapDistance = std::clamp(asFloat(gSnapDistance, snapDistance), 40.0f, 300.0f);

    excludeInCombat = asBool(gExcludeInCombat, excludeInCombat);
    excludeFollowers = asBool(gExcludeFollowers, excludeFollowers);
    excludeMounted = asBool(gExcludeMounted, excludeMounted);

    followerReplay = asBool(gFollowerReplay, followerReplay);
    debugLogging = asBool(gDebugLogging, debugLogging);
    enableEvgTraversal = asBool(gEvgTraversal, enableEvgTraversal);
    teleportEscalation = std::clamp(asInt(gTeleportEscalation, teleportEscalation), 1, 10);
}
