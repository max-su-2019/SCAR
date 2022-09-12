#pragma once

namespace SCAR
{
	class AttackRangeCheck
	{
	public:
		static bool CheckPathing(RE::Actor* a_attacker, RE::Actor* a_target);

		static bool WithinAttackRange(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle);

		static void DrawOverlay(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle);
	};
}
