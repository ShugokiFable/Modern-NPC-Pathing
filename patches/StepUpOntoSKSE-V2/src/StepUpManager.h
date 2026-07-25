#pragma once
#include <cstdint>
#include <unordered_map>

namespace StepUp
{
	class StepUpManager
	{
	public:
		struct NpcPerfStats
		{
			std::uint32_t scannedActors{ 0 };
			std::uint32_t inRangeActors{ 0 };
			std::uint32_t processedActors{ 0 };
			std::uint32_t steppedActors{ 0 };
			std::uint32_t combatBypassActors{ 0 };
			std::uint32_t activeSessions{ 0 };
			float         scanMs{ 0.f };
			float         stepMs{ 0.f };
			float         totalMs{ 0.f };
		};

		static StepUpManager* GetSingleton()
		{
			static StepUpManager instance;
			return &instance;
		}

		void Update(RE::PlayerCharacter* a_player, float a_deltaTime);
		[[nodiscard]] NpcPerfStats GetNpcPerfStats() const { return m_npcPerf; }

	private:
		StepUpManager() = default;

		[[nodiscard]] bool         CanPlayerAttemptStep(RE::PlayerCharacter* a_player) const;
		[[nodiscard]] static bool  CanNPCAttemptStep(RE::Actor* a_actor);
		[[nodiscard]] static RE::NiPoint3 GetWorldMoveDirection(RE::PlayerCharacter* a_player);
		[[nodiscard]] static float HitNormalUpDot(const RE::hkVector4& a_normal);

		void TryApplyStepUp(
			RE::PlayerCharacter*        a_player,
			RE::bhkCharacterController* a_controller,
			RE::bhkWorld*               a_world,
			float                       a_deltaTime);

		void EndStep()
		{
			m_stepBoostActive  = false;
			m_smoothLiftSpeed  = 0.f;
			m_stepSessionTime  = 0.f;
		}

		void RestoreGravity(RE::bhkCharacterController* a_controller);
		void UpdateNearbyNPCs(RE::PlayerCharacter* a_player, RE::bhkWorld* a_world, float a_deltaTime);

		struct ActorStepState
		{
			RE::NiPoint3 prevPos{ 0.f, 0.f, 0.f };
			bool  hasPosHistory{ false };
			float stuckTimer{ 0.f };
			float cooldownRemaining{ 0.f };
			float lastMovingTime{ 0.f };
			float lastSeenTime{ 0.f };
		};

		void TryApplyStepUpNPC(
			RE::Actor*                  a_actor,
			RE::bhkCharacterController* a_controller,
			RE::bhkWorld*               a_world,
			ActorStepState&             a_state);

		float m_cooldownRemaining{ 0.f };
		bool  m_stepBoostActive{ false };
		float m_stepStartZ{ 0.f };
		float m_smoothLiftSpeed{ 0.f };
		RE::NiPoint3 m_stepMoveDir{ 0.f, 0.f, 0.f };
		float m_lastGroundNormalZ{ 1.f };
		float m_lastRiserNormalZ{ 0.f };

		float m_prevFootZ{ 0.f };
		float m_zVelocity{ 0.f };
		bool  m_hasFootZHistory{ false };

		float m_stepSessionTime{ 0.f };
		float m_gravityDisabledTime{ 0.f };

		float m_savedGravity{ 0.f };
		bool  m_gravityDisabled{ false };
		bool  m_hadNoGravityFlag{ false };
		bool  m_hadOnStairsFlag{ false };

		float m_npcClock{ 0.f };
		std::unordered_map<RE::FormID, ActorStepState> m_npcStates;
		NpcPerfStats m_npcPerf{};
	};
}
