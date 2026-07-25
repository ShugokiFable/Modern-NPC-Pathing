#include "Settings.h"

// NPCPathingNG-compat build: SimpleIni dependency replaced with a small
// self-contained parser/writer. Same file, same keys, same behavior.

namespace
{
	constexpr const char* kPath = "Data/SKSE/Plugins/StepUpOntoSKSE.ini";

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

	// "section/key" (lowercased) -> value
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
			values[section + "/" + ToLower(Trim(line.substr(0, eq)))] = Trim(line.substr(eq + 1));
		}
		return values;
	}

	bool ParseBool(const std::string& a_val, bool a_def)
	{
		const auto lower = ToLower(a_val);
		if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
			return true;
		}
		if (lower == "false" || lower == "0" || lower == "no" || lower == "off") {
			return false;
		}
		return a_def;
	}
}

void Settings::Clamp()
{
	maxStepHeight = std::clamp(maxStepHeight, 10.f, 120.f);
	minStepHeight = std::clamp(minStepHeight, 0.f, maxStepHeight - 1.f);
	forwardDetectionDistance = std::clamp(forwardDetectionDistance, 20.f, 150.f);
	maxStepVelocity = std::clamp(maxStepVelocity, 50.f, 800.f);
	stepCooldown = std::clamp(stepCooldown, 0.05f, 1.0f);
	maxSlopeAngle = std::clamp(maxSlopeAngle, 15.f, 75.f);
	shinHeight = std::clamp(shinHeight, 5.f, 40.f);
	stepForwardPastRiser = std::clamp(stepForwardPastRiser, 2.f, 24.f);
	displacementSafetyMargin = std::clamp(displacementSafetyMargin, 1.0f, 2.0f);
	sameWallDistanceEpsilon = std::clamp(sameWallDistanceEpsilon, 1.f, 20.f);
	npcMaxActors = std::clamp(npcMaxActors, 0, 128);
	npcRange = std::clamp(npcRange, 300.f, 10000.f);
}

void Settings::Load()
{
	auto values = ParseINI(kPath);
	if (values.empty()) {
		logger::warn("StepUpOntoSKSE.ini not found; creating with defaults");
		Clamp();
		Save();
		return;
	}

	auto getBool = [&](const char* a_key, bool a_def) {
		auto it = values.find(a_key);
		return it != values.end() ? ParseBool(it->second, a_def) : a_def;
	};
	auto getInt = [&](const char* a_key, int a_def) {
		auto it = values.find(a_key);
		return it != values.end() ? std::atoi(it->second.c_str()) : a_def;
	};
	auto getFloat = [&](const char* a_key, float a_def) {
		auto it = values.find(a_key);
		return it != values.end() ? static_cast<float>(std::atof(it->second.c_str())) : a_def;
	};

	enableMod = getBool("general/benablemod", true);
	enableDebugLogging = getBool("general/benabledebuglogging", false);
	enableCompletionNudge = getBool("general/benablecompletionnudge", true);
	enableNPCCompletionNudge = getBool("general/benablenpccompletionnudge", true);
	enableNearbyNPCStepUp = getBool("general/benablenearbynpcstepup", false);
	showNPCPerfOverlay = getBool("general/bshownpcperfoverlay", false);
	npcCombatBypassLimit = getBool("general/bnpccombatbypasslimit", false);
	npcMaxActors = getInt("general/inpcmaxactors", 0);
	npcRange = getFloat("general/fnpcrange", 1800.f);

	maxStepHeight = getFloat("stepup/fmaxstepheight", 40.f);
	minStepHeight = getFloat("stepup/fminstepheight", 5.f);
	forwardDetectionDistance = getFloat("stepup/fforwarddetectiondistance", 50.f);
	maxStepVelocity = getFloat("stepup/fmaxstepvelocity", 200.f);
	stepCooldown = getFloat("stepup/fstepcooldown", 0.15f);
	maxSlopeAngle = getFloat("stepup/fmaxslopeangle", 50.f);
	shinHeight = getFloat("stepup/fshinheight", 15.f);
	stepForwardPastRiser = getFloat("stepup/fstepforwardpastriser", 8.f);
	displacementSafetyMargin = getFloat("stepup/fdisplacementsafetymargin", 1.15f);
	sameWallDistanceEpsilon = getFloat("stepup/fsamewalldistanceepsilon", 4.f);

	Clamp();
}

void Settings::Save()
{
	std::ofstream f(kPath);
	if (!f.is_open()) {
		logger::error("Failed to save StepUpOntoSKSE.ini");
		return;
	}
	auto b = [](bool v) { return v ? "true" : "false"; };
	f << "[General]\n"
	  << "bEnableMod = " << b(enableMod) << "\n"
	  << "bEnableDebugLogging = " << b(enableDebugLogging) << "\n"
	  << "bEnableCompletionNudge = " << b(enableCompletionNudge) << "\n"
	  << "bEnableNPCCompletionNudge = " << b(enableNPCCompletionNudge) << "\n"
	  << "bEnableNearbyNPCStepUp = " << b(enableNearbyNPCStepUp) << "\n"
	  << "bShowNPCPerfOverlay = " << b(showNPCPerfOverlay) << "\n"
	  << "bNPCCombatBypassLimit = " << b(npcCombatBypassLimit) << "\n"
	  << "iNPCMaxActors = " << npcMaxActors << "\n"
	  << "fNPCRange = " << std::fixed << std::setprecision(6) << npcRange << "\n"
	  << "\n[StepUp]\n"
	  << "fMaxStepHeight = " << maxStepHeight << "\n"
	  << "fMinStepHeight = " << minStepHeight << "\n"
	  << "fForwardDetectionDistance = " << forwardDetectionDistance << "\n"
	  << "fMaxStepVelocity = " << maxStepVelocity << "\n"
	  << "fStepCooldown = " << stepCooldown << "\n"
	  << "fMaxSlopeAngle = " << maxSlopeAngle << "\n"
	  << "fShinHeight = " << shinHeight << "\n"
	  << "fStepForwardPastRiser = " << stepForwardPastRiser << "\n"
	  << "fDisplacementSafetyMargin = " << displacementSafetyMargin << "\n"
	  << "fSameWallDistanceEpsilon = " << sameWallDistanceEpsilon << "\n";
}
