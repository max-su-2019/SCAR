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
}
