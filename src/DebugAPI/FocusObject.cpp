#pragma once
#include "FocusObject.h"
#include "Util.h"

FocusObject::FocusObject(float priority, RE::TESObjectREFR* objectRef, CollisionFocusObject collisionObject)
{
	Priority = priority;
	ObjectRef = objectRef;
	CollisionObject = collisionObject;
}

ECollisionObjectSize FocusObject::GetCollisionSize(float size)
{
	if (size >= OBJECT_SIZE_THRESHHOLD_HUGE)
		return Huge;
	else if (size >= OBJECT_SIZE_THRESHHOLD_LARGE)
		return Large;
	else if (size >= OBJECT_SIZE_THRESHHOLD_MEDIUM)
		return Medium;
	else if (size >= OBJECT_SIZE_THRESHHOLD_SMALL)
		return Small;

	return Tiny;
}

CollisionFocusObject::CollisionFocusObject(RE::NiCollisionObject* collisionObject, RE::NiNode* parentNode)
{
	SetData(collisionObject, parentNode);
}

CollisionFocusObject::CollisionFocusObject()
{
	CollisionObject = nullptr;
	ParentNode = nullptr;

	IsValid = false;
}

void CollisionFocusObject::SetData(RE::NiCollisionObject* collisionObject, RE::NiNode* parentNode)
{
	CollisionObject = collisionObject;
	ParentNode = parentNode;

	IsValid = CollisionObject && ParentNode;
}

void CollisionFocusObject::SetObjectBound(ObjectBound& bb)
{
	BoundingBox = bb;
	HasObjectBound = true;
}

void CollisionFocusObject::UpdateBound()
{
	ObjectBound bound;
	if (Util::GetBoundingBox(*this, bound))
	{
		BoundingBox = bound;
	}
}
