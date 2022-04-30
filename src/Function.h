#pragma once

namespace SCAR
{
	bool GetDistanceVariable(RE::Actor* a_actor, std::map<const std::string, float>& a_map);

	bool IsInDistance(float currentDistance, float minDistance, float maxDistance);
}
