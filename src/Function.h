#pragma once

namespace SCAR
{
	float GetGameSettingFloat(const std::string a_name, const float a_default);

	class AttackRangeCheck
	{
	public:
		static bool CheckPathing(RE::Actor* a_attacker, RE::Actor* a_target);

		static bool WithinAttackRange(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startRadian, float a_endRadian);

		static void DrawOverlay(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startRadian, float a_endRadian);
	};
}
