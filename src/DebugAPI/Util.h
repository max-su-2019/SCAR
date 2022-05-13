#pragma once
#include <vector>

#include "RE/N/NiMath.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include "FocusObject.h"

enum EAxis
{
	AxisX,
	AxisY,
	AxisZ
};

struct RayCastData
{
	RayCastData(glm::vec4 startPos, glm::vec4 endPos)
	{
		StartPos = startPos;
		EndPos = endPos;
	}

	glm::vec4 StartPos;
	glm::vec4 EndPos;
};

namespace Util
{
	/*
	vector translation notes

	Euler Angles:
		x = PITCH
		y = ROLL
		z = YAW

	Position:
		x = RIGHT
		y = FORWARD
		z = UP
	*/

	// inserts value into a vector, keeping the vector sorted
	template <typename T, typename Pred>
	typename std::vector<T>::iterator
		insert_sorted(std::vector<T>& vec, T const& item, Pred pred)
	{
		return vec.insert(
			std::upper_bound(vec.begin(), vec.end(), item, pred),
			item);
	}

	glm::highp_mat4 GetRotationMatrix(glm::vec3 eulerAngles);

	glm::vec3 NormalizeVector(glm::vec3 p);
	glm::vec3 RotateVector(glm::quat quatIn, glm::vec3 vecIn);
	glm::vec3 RotateVector(glm::vec3 eulerIn, glm::vec3 vecIn);
	glm::vec3 GetForwardVector(glm::quat quatIn);
	glm::vec3 GetForwardVector(glm::vec3 eulerIn);
	glm::vec3 GetRightVector(glm::quat quatIn);
	glm::vec3 GetRightVector(glm::vec3 eulerIn);

	glm::vec3 ThreeAxisRotation(float r11, float r12, float r21, float r31, float r32);

	glm::vec3 NiPointToVec(const RE::NiPoint3& a_point3);

	bool GetBoundingBox(RE::TESObjectREFR* object, ObjectBound& bound);
	//bool GetBoundingBox(RE::NiCollisionObject* object, ObjectBound& bound);
	bool GetBoundingBox(CollisionFocusObject& clObject, ObjectBound& bound);
	bool GetBoundingBox(FocusObject* object, ObjectBound& bound);
	bool GetBBRigidBody(RE::TESObjectREFR* object, ObjectBound& bound);
	bool GetBBCharacter(RE::TESObjectREFR* object, ObjectBound& bound);

	std::string GetCollisionLayer(CollisionFocusObject clObject);

	bool GetBoundingBox_Fallback(RE::TESObjectREFR* object, ObjectBound& bound);

	void ToLowerString(std::string& stringIn);
	void StripWhiteSpaces(std::string& stringIn);

	bool IsEmpty(const char* stringIn);
	bool IsEmpty(std::string stringIn);
	bool IsFirstLineEmpty(const char* stringIn);
	int StringFind(const char* stringIn, const char* substring);

	glm::vec3 RotMatrixToEuler(RE::NiMatrix3 matrixIn);

	// recursively iterates a node's children to find the first collisionObject
	CollisionFocusObject FindCollisionObjectRecursive(RE::NiNode* nodeIn, int recursionDepth = 0);
	// recursively iterates a node's children to find all collisionObjects

	std::vector<CollisionFocusObject> FindCollisionObjectListRecursive(RE::NiNode* nodeIn, float& bestColSize, int recursionDepth = 0);
	std::vector<CollisionFocusObject> FindCollisionObjectListRecursive(RE::NiNode* nodeIn, int recursionDepth = 0);

	RE::NiNode* GetChildNodeByName(RE::NiNode* nodeIn, const char* targetName);

	std::vector<CollisionFocusObject> FindAllCollisionObjectsRecursive(RE::NiNode* nodeIn, int recursionDepth = 0);

	bool HasCollisionObjectRecursive(RE::NiNode* nodeIn, int recursionDepth = 0);

	// attempts to find the object's spine node, returns the root node if none is found
	RE::NiAVObject* GetCharacterSpine(RE::TESObjectREFR* object);
	// attempts to find the object's head node, returns the root node if none is found
	RE::NiAVObject* GetCharacterHead(RE::TESObjectREFR* object);

	bool IsRoughlyEqual(float first, float second, float maxDif);

	glm::vec3 QuatToEuler(glm::quat q);

	glm::quat EulerToQuat(glm::vec3 rotIn);

	glm::vec3 GetInverseRotation(glm::vec3 rotIn);
	glm::quat GetInverseRotation(glm::quat rotIn);

	glm::vec3 EulerRotationToVector(glm::vec3);
	glm::vec3 VectorToEulerRotation(glm::vec3);
	glm::vec3 GetCameraPos();
	glm::quat GetCameraRot();

	bool IsPosBehindPlayerCamera(glm::vec3 pos);

	// gets only the x componend of the bounding box, rotated with the object
	glm::vec3 GetBoundRightVectorRotated(ObjectBound objectBound);
	// gets only the y componend of the bounding box, rotated with the object
	glm::vec3 GetBoundForwardVectorRotated(ObjectBound objectBound);
	// gets only the z componend of the bounding box, rotated with the object
	glm::vec3 GetBoundUpVectorRotated(ObjectBound objectBound);

	// returns center of the rotated bounding box
	glm::vec3 GetBoundingBoxCenter(RE::TESObjectREFR* object);
	glm::vec3 GetBoundingBoxCenter(CollisionFocusObject clObject);
	glm::vec3 GetBoundingBoxCenter(FocusObject* object);
	glm::vec3 GetBoundingBoxCenter(ObjectBound bb);

	// returns point above the object for characters, and the rotated top of the
	// bounding box, centered on x and y for other objects
	glm::vec3 GetBoundingBoxTop(RE::TESObjectREFR* object);
	glm::vec3 GetBoundingBoxTop(CollisionFocusObject clObject);
	glm::vec3 GetBoundingBoxTop(FocusObject* object);
	glm::vec3 GetBoundingBoxTop(ObjectBound bb);

	glm::vec3 GetBoundingBoxBottom(RE::TESObjectREFR* object);
	glm::vec3 GetBoundingBoxBottom(CollisionFocusObject clObject);
	glm::vec3 GetBoundingBoxBottom(FocusObject* object);
	glm::vec3 GetBoundingBoxBottom(ObjectBound bb);

	// given a world coordinate and a radius, this returns a point on a rotated circle, with values of i
	// ranging between 0 and maxI to determine the point on the circle
	glm::vec3 GetPointOnRotatedCircle(glm::vec3 origin, float radius, float i, float maxI, glm::vec3 eulerAngles);

	void ForEachReferenceInRange(RE::NiPoint3 originPos, float a_radius, std::function<bool(RE::TESObjectREFR& a_ref)> a_callback);

	// the Skyrim ref's position doesn't always update with its Havok mesh. This is a workaround for this bug
	// it returns the position of the havok mesh, or the vanilla position if no mesh is present
	glm::vec3 GetObjectAccuratePosition(RE::TESObjectREFR* object);

	float GetDistanceToBounds(glm::vec3 start, RE::TESObjectREFR* object);

	// using these instead of the ones on the actual cells, because they use inaccurate position to check against
	// See GetObjectAccuratePosition
	void CellForEachReference(RE::TESObjectCELL* cell, std::function<bool(RE::TESObjectREFR&)> a_callback);
	void CellForEachReferenceInRange(RE::TESObjectCELL* cell, const RE::NiPoint3& a_origin, float a_radius, std::function<bool(RE::TESObjectREFR&)> a_callback);

	// returns true if in param doesn't contain a value that is nan or infinity, or the negative of nan or infinity
	bool IsValid(float numIn);
	bool IsValid(glm::vec3 vecIn);

	// if any value in the inParam is invalid, set this value to second param
	float MakeValid(float numIn, float setInvalidTo);
	glm::vec3 MakeValid(glm::vec3 vecIn, float setInvalidTo);

	// returns which axis of the bounds is largest
	EAxis GetBoundsLongestAxis(ObjectBound& bound);
	// returns the origin, except centered on one specified axis
	glm::vec3 GetAxisOrigin(ObjectBound& bound, EAxis axis);

	RE::TESObjectREFR* GetPlayerMountRef();
	bool IsPlayerInFirstPerson();

	constexpr int FOLLOW_PARENT_MAX_RECURSION = 2;

	// for distance checks toward all nearby objects, first a rough check toward the origin is performed, with an added
	// threshhold of this. If this succeeds, a more precise check will be performed against the BB center of the object
	// this avoids making BB calculations for every object in the cell(s)
	constexpr float ROUGH_DISTANCE_ADD = 250.0f;

	// values smaller / larger than this will not be counted as valid by IsValid
	constexpr float POSITIVE_INVALID_THRESHHOLD = 999999.0f;
	constexpr float NEGATIVE_INVALID_THRESHHOLD = -999999.0f;
};
