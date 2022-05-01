#pragma once

namespace SCAR
{
	static constexpr char NEXT_ATTACK_CHANCE[] = "SCAR_nextattackchance";

	bool GetDistanceVariable(RE::Actor* a_actor, std::map<const std::string, float>& a_map);

	bool IsInDistance(float currentDistance, float minDistance, float maxDistance);

	bool ShouldNextAttack(RE::Actor* a_actor);
}
