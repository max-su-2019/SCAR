#pragma once
#include "ObjectBound.h"

enum ECollisionObjectSize
{
	None,
	Tiny,	// 1
	Small,	// 2
	Medium,	// 3
	Large,	// 4
	Huge	// 5
};

// currently only uses THRESHHOLD_MEDIUM to determined whether multiple collision objects
// should be considered for selection, if present
const float OBJECT_SIZE_THRESHHOLD_SMALL	= 20.0f;
const float OBJECT_SIZE_THRESHHOLD_MEDIUM	= 40.0f;
const float OBJECT_SIZE_THRESHHOLD_LARGE	= 150.0f;
const float OBJECT_SIZE_THRESHHOLD_HUGE		= 250.0f;

class CollisionFocusObject
{
public:
	CollisionFocusObject(RE::NiCollisionObject* collisionObject, RE::NiNode* parentNode);

	CollisionFocusObject();

	void SetData(RE::NiCollisionObject* collisionObject, RE::NiNode* parentNode);
	void SetObjectBound(ObjectBound& bb);

	void UpdateBound();

	RE::NiCollisionObject* CollisionObject;
	RE::NiNode* ParentNode;

	// cached object bounds to avoid recalculating all over the place
	ObjectBound BoundingBox;
	bool HasObjectBound = false;

	bool IsValid;
};

class FocusObject
{
public:
	FocusObject(float priority, RE::TESObjectREFR* objectRef, CollisionFocusObject collisionObject);

	static ECollisionObjectSize GetCollisionSize(float size);

	/* a product of distance to player and distance between camera angle and angle
	from camera toward object. Lower priority means that this object is preferred as
	focus */
	float Priority;

	RE::ObjectRefHandle ObjectRef;

	// if the object has multiple collisionObjects, this is the one with the best (lowest) priority,
	// determined by camera distance/angle
	CollisionFocusObject CollisionObject;
};
