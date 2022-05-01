#include "Function.h"

namespace SCAR
{
	bool GetDistanceVariable(RE::Actor* a_actor, std::map<const std::string, float>& a_map)
	{
		for (auto& pair : a_map) {
			if (!a_actor->GetGraphVariableFloat(pair.first, pair.second))
				return false;
		}

		return true;
	};

	bool IsInDistance(float currentDistance, float minDistance, float maxDistance)
	{
		return currentDistance >= minDistance && currentDistance <= maxDistance;
	}

	bool ShouldNextAttack(RE::Actor* a_actor)
	{
		float nextAttackChance = 0.f;
		if (a_actor && a_actor->GetGraphVariableFloat(NEXT_ATTACK_CHANCE, nextAttackChance)) {
			return nextAttackChance > Random::get<float>(0.f, 100.f);
		}

		return false;
	}
}
