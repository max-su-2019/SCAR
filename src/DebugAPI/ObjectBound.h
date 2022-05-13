#pragma once
#include <glm/ext.hpp>
struct ObjectBound
{
	ObjectBound()
	{
		boundMin = glm::vec3();
		boundMax = glm::vec3();
		worldBoundMin = glm::vec3();
		worldBoundMax = glm::vec3();
		rotation = glm::vec3();
	}

	ObjectBound(glm::vec3 pBoundMin, glm::vec3 pBoundMax, glm::vec3 pWorldBoundMin, glm::vec3 pWorldBoundMax, glm::vec3 pRotation)
	{
		boundMin = pBoundMin;
		boundMax = pBoundMax;
		worldBoundMin = pWorldBoundMin;
		worldBoundMax = pWorldBoundMax;
		rotation = pRotation;
	}

	glm::vec3 boundMin;
	glm::vec3 boundMax;
	glm::vec3 worldBoundMin;
	glm::vec3 worldBoundMax;
	glm::vec3 rotation;
};
