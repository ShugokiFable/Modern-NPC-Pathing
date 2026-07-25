#pragma once

class Settings
{
public:
	static Settings* GetSingleton()
	{
		static Settings instance;
		return &instance;
	}

	void Load();
	void Save();
	void Clamp();

	bool   enableMod{ true };
	bool   enableDebugLogging{ false };
	bool   enableCompletionNudge{ true };
	bool   enableNPCCompletionNudge{ true };
	bool   enableNearbyNPCStepUp{ false };
	bool   showNPCPerfOverlay{ false };
	bool   npcCombatBypassLimit{ false };
	int    npcMaxActors{ 0 };
	float  npcRange{ 1800.f };

	float maxStepHeight{ 40.f };
	float minStepHeight{ 5.f };
	float forwardDetectionDistance{ 50.f };
	float maxStepVelocity{ 200.f };
	float stepCooldown{ 0.15f };
	float maxSlopeAngle{ 50.f };
	float shinHeight{ 15.f };
	float stepForwardPastRiser{ 8.f };
	float displacementSafetyMargin{ 1.15f };
	float sameWallDistanceEpsilon{ 4.f };
};
