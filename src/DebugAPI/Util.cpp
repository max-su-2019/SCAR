#pragma once
#include "Util.h"
#include "DebugAPI.h"
#include "RE/RTTI.h"

glm::highp_mat4 Util::GetRotationMatrix(glm::vec3 eulerAngles)
{
	return glm::eulerAngleXYZ(-(eulerAngles.x), -(eulerAngles.y), -(eulerAngles.z));
}

glm::vec3 Util::NormalizeVector(glm::vec3 p)
{
	return glm::normalize(p);
}

glm::vec3 Util::RotateVector(glm::quat quatIn, glm::vec3 vecIn)
{
	float num = quatIn.x * 2.0f;
	float num2 = quatIn.y * 2.0f;
	float num3 = quatIn.z * 2.0f;
	float num4 = quatIn.x * num;
	float num5 = quatIn.y * num2;
	float num6 = quatIn.z * num3;
	float num7 = quatIn.x * num2;
	float num8 = quatIn.x * num3;
	float num9 = quatIn.y * num3;
	float num10 = quatIn.w * num;
	float num11 = quatIn.w * num2;
	float num12 = quatIn.w * num3;
	glm::vec3 result;
	result.x = (1.0f - (num5 + num6)) * vecIn.x + (num7 - num12) * vecIn.y + (num8 + num11) * vecIn.z;
	result.y = (num7 + num12) * vecIn.x + (1.0f - (num4 + num6)) * vecIn.y + (num9 - num10) * vecIn.z;
	result.z = (num8 - num11) * vecIn.x + (num9 + num10) * vecIn.y + (1.0f - (num4 + num5)) * vecIn.z;
	return result;
}

glm::vec3 Util::RotateVector(glm::vec3 eulerIn, glm::vec3 vecIn)
{
	glm::vec3 glmVecIn(vecIn.x, vecIn.y, vecIn.z);
	glm::mat3 rotationMatrix = glm::eulerAngleXYZ(eulerIn.x, eulerIn.y, eulerIn.z);

	return rotationMatrix * glmVecIn;
}

glm::vec3 Util::GetForwardVector(glm::quat quatIn)
{
	// rotate Skyrim's base forward vector (positive Y forward) by quaternion
	return RotateVector(quatIn, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Util::GetForwardVector(glm::vec3 eulerIn)
{
	float pitch = eulerIn.x;
	float yaw = eulerIn.z;

	return glm::vec3(
		sin(yaw) * cos(pitch),
		cos(yaw) * cos(pitch),
		sin(pitch));
}

glm::vec3 Util::GetRightVector(glm::quat quatIn)
{
	// rotate Skyrim's base right vector (positive X forward) by quaternion
	return RotateVector(quatIn, glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 Util::GetRightVector(glm::vec3 eulerIn)
{
	float pitch = eulerIn.x;
	float yaw = eulerIn.z + glm::half_pi<float>();

	return glm::vec3(
		sin(yaw) * cos(pitch),
		cos(yaw) * cos(pitch),
		sin(pitch));
}

glm::vec3 Util::ThreeAxisRotation(float r11, float r12, float r21, float r31, float r32)
{
	return glm::vec3(
		asin(r21),
		atan2(r11, r12),
		atan2(-r31, r32));
}

glm::vec3 Util::NiPointToVec(const RE::NiPoint3& a_point3)
{
	return glm::vec3(a_point3.x, a_point3.y, a_point3.z);
}

bool Util::GetBoundingBox(RE::TESObjectREFR* object, ObjectBound& bound)
{
	if (!object)
		return false;

	auto objCharacter = object->GetObjectReference()->As<RE::TESNPC>();
	auto objActor = object->GetObjectReference()->As<RE::Actor>();

	if ((objActor || objCharacter) && GetBBCharacter(object, bound))
		return true;

	if (GetBBRigidBody(object, bound))
		return true;

	return GetBoundingBox_Fallback(object, bound);
}

bool Util::GetBoundingBox(CollisionFocusObject& clObject, ObjectBound& bound)
{
	if (!clObject.CollisionObject)
		return false;

	auto niClObject = clObject.CollisionObject->AsBhkNiCollisionObject();
	if (niClObject && niClObject->body.get()) {
		auto rigidBody = niClObject->body.get()->AsBhkRigidBody();

		if (rigidBody && rigidBody->referencedObject.get()) {
			auto havokRigidBody = static_cast<RE::hkpRigidBody*>(rigidBody->referencedObject.get());
			if (havokRigidBody) {
				uintptr_t hkpBoxShape = (uintptr_t)havokRigidBody + 0x20;
				if (!hkpBoxShape)
					return false;

				RE::hkpShape* hkpShape = *(RE::hkpShape**)hkpBoxShape;
				if (!hkpShape)
					return false;

				// havok uses different coordinate system. Multiply by this to convert havok units to Skyrim units
				float scaleInverse = RE::bhkWorld::GetWorldScaleInverse();

				RE::hkVector4 bhkBodyPosition;
				rigidBody->GetPosition(bhkBodyPosition);
				// just gets the data out of the register Havok uses into a more usable format
				float bodyPosition[4];
				_mm_store_ps(bodyPosition, bhkBodyPosition.quad);

				RE::hkTransform shapeTransform;
				// use identity matrix for the BB of the unrotated object
				shapeTransform.rotation.col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
				shapeTransform.rotation.col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
				shapeTransform.rotation.col2 = { 0.0f, 0.0f, 1.0f, 0.0f };

				shapeTransform.translation.quad = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
				RE::hkAabb boundingBoxLocal;
				hkpShape->GetAabbImpl(shapeTransform, 0.0f, boundingBoxLocal);

				float boundMinLocal[4];
				_mm_store_ps(boundMinLocal, boundingBoxLocal.min.quad);
				float boundMaxLocal[4];
				_mm_store_ps(boundMaxLocal, boundingBoxLocal.max.quad);

				RE::hkVector4 rigidBodyLocalTranslation;
				glm::vec3 rigidBodyLocalRotation(0.0f);

				auto rigidBodyT = skyrim_cast<RE::bhkRigidBodyT*>(rigidBody);
				// some objects have a local translation on their collisionObject. In that
				// case, the node is of type bhkRigidBodyT The BB takes that into
				// account, so I have to as well. Can be seen in nifscope
				if (rigidBodyT) {
					rigidBodyLocalTranslation = rigidBodyT->translation;

					// don't forget that bhkRigidBodyT also has a local rotation!
					auto hkLocalRot = rigidBodyT->rotation;
					float localRotArr[4];
					_mm_store_ps(localRotArr, hkLocalRot.vec.quad);

					rigidBodyLocalRotation = Util::QuatToEuler(glm::quat(
						MakeValid(localRotArr[3], 0.0f),
						MakeValid(localRotArr[0], 0.0f),
						MakeValid(localRotArr[1], 0.0f),
						MakeValid(localRotArr[2], 0.0f)));
				}

				float localTranslation[4];
				_mm_store_ps(localTranslation, rigidBodyLocalTranslation.quad);

				// for some reason still requires adding local rotation offset. Rotation is accurate for most
				// objects, but some are weird: eg. books, wooden handle of mills (somehow on the mill object,
				// local rotation offset shouldn't be added?)
				auto clObjectParentRot = RotMatrixToEuler(clObject.CollisionObject->sceneObject->world.rotate);
				bound.rotation = clObjectParentRot + rigidBodyLocalRotation;

				// put local positions into output
				bound.boundMin = glm::vec3(boundMinLocal[0], boundMinLocal[1], boundMinLocal[2]) * scaleInverse;
				bound.boundMax = glm::vec3(boundMaxLocal[0], boundMaxLocal[1], boundMaxLocal[2]) * scaleInverse;

				auto glmLocalT = MakeValid(glm::vec3(localTranslation[0], localTranslation[1], localTranslation[2]) * scaleInverse, 0.0f);
				// apply the local rotation of bhkRigidBodyT to its translation
				auto glmLocalTRotated = RotateVector(rigidBodyLocalRotation, glmLocalT);

				auto glmBodyPosition = glm::vec3(bodyPosition[0], bodyPosition[1], bodyPosition[2]) * scaleInverse;

				// take what would be the local origin location of the BB and rotate it with the body
				auto boundMinRotated_origin = Util::RotateVector(bound.rotation, bound.boundMin + glmLocalTRotated);
				auto boundMaxRotated_origin = Util::RotateVector(bound.rotation, bound.boundMax + glmLocalTRotated);

				// global origin location
				bound.worldBoundMin = glmBodyPosition + boundMinRotated_origin;
				bound.worldBoundMax = glmBodyPosition + boundMaxRotated_origin;

				return true;
			}
		}
	}

	return false;
}

bool Util::GetBoundingBox(FocusObject* object, ObjectBound& bound)
{
	if (!object)
		return false;

	if (object->CollisionObject.IsValid && GetBoundingBox(object->CollisionObject, bound))
		return true;
	else if (object->ObjectRef.get() && GetBoundingBox(object->ObjectRef.get().get(), bound))
		return true;

	return GetBoundingBox_Fallback(object->ObjectRef.get().get(), bound);
}

bool Util::GetBBRigidBody(RE::TESObjectREFR* object, ObjectBound& bound)
{
	auto mesh = object->GetCurrent3D();
	if (!mesh)
		return false;

	auto clObject = FindCollisionObjectRecursive(mesh->AsNode());
	if (!clObject.IsValid)
		return false;

	return GetBoundingBox(clObject, bound);
}

bool Util::GetBBCharacter(RE::TESObjectREFR* object, ObjectBound& bound)
{
	if (!object)
		return false;

	auto characterObject = object->As<RE::Actor>();
	if (!characterObject)
		return false;

	auto npc = characterObject->GetActorBase();
	if (!npc)
		return false;

	RE::NiAVObject* body = object->Get3D();
	if (!body)
		return false;

	auto scale = static_cast<float>(object->GetReferenceRuntimeData().refScale) / 100.f * npc->GetBaseScale();

	bound.boundMin = NiPointToVec(object->GetBoundMin() * scale);
	bound.boundMax = NiPointToVec(object->GetBoundMax() * scale);

	bound.rotation = RotMatrixToEuler(body->world.rotate);

	auto niPosition = body->world.translate;
	auto boundMinRotated = RotateVector(bound.rotation, bound.boundMin);
	auto boundMaxRotated = RotateVector(bound.rotation, bound.boundMax);

	bound.worldBoundMin = glm::vec3(niPosition.x + boundMinRotated.x,
		niPosition.y + boundMinRotated.y,
		niPosition.z + boundMinRotated.z);

	bound.worldBoundMax = glm::vec3(niPosition.x + boundMaxRotated.x,
		niPosition.y + boundMaxRotated.y,
		niPosition.z + boundMaxRotated.z);

	return true;
}

std::string Util::GetCollisionLayer(CollisionFocusObject clObject)
{
	auto niClObject = clObject.CollisionObject->AsBhkNiCollisionObject();
	if (niClObject && niClObject->body.get()) {
		auto rigidBody = niClObject->body.get()->AsBhkRigidBody();

		if (rigidBody && rigidBody->referencedObject.get()) {
			auto havokRigidBody = static_cast<RE::hkpRigidBody*>(rigidBody->referencedObject.get());
			if (havokRigidBody) {
				auto collidable = havokRigidBody->GetCollidable();
				auto clFilterInfo = collidable->broadPhaseHandle.collisionFilterInfo;

				auto clLayer = clFilterInfo & 0x7f;

				return std::to_string(clLayer);
			}
		}
	}

	return "";
}

bool Util::GetBoundingBox_Fallback(RE::TESObjectREFR* object, ObjectBound& bound)
{
	if (!object)
		return false;

	auto mesh = object->GetCurrent3D();

	RE::NiPoint3 niPos;
	float boundRadius_fallback = 0.0f;

	if (mesh) {
		niPos = mesh->world.translate;
		boundRadius_fallback = mesh->worldBound.radius;
	} else
		niPos = object->GetPosition();

	glm::vec3 objectPos(niPos.x, niPos.y, niPos.z);

	bound.worldBoundMin = objectPos - boundRadius_fallback;
	bound.worldBoundMax = objectPos + boundRadius_fallback;

	bound.rotation = glm::vec3(0.0f, 0.0f, 0.0f);

	bound.boundMin = glm::vec3(-boundRadius_fallback, -boundRadius_fallback, -boundRadius_fallback);
	bound.boundMax = glm::vec3(boundRadius_fallback, boundRadius_fallback, boundRadius_fallback);

	return true;
}

void Util::ToLowerString(std::string& stringIn)
{
	for (char& c : stringIn) {
		c = (char)tolower(c);
	}
}

void Util::StripWhiteSpaces(std::string& stringIn)
{
	stringIn.erase(std::remove_if(stringIn.begin(), stringIn.end(), ::isspace), stringIn.end());
}

bool Util::IsEmpty(const char* stringIn)
{
	if (!stringIn)
		return true;

	return IsEmpty(std::string(stringIn));
}

bool Util::IsEmpty(std::string stringIn)
{
	if (stringIn.empty())
		return true;

	return (stringIn.empty() || stringIn.find_first_not_of(" \x1\t") == std::string::npos);
}

bool Util::IsFirstLineEmpty(const char* stringIn)
{
	if (!stringIn)
		return true;

	std::string stdString(stringIn);

	if (stdString.empty())
		return true;

	std::string::iterator end_pos = std::remove(stdString.begin(), stdString.end(), ' ');
	stdString.erase(end_pos, stdString.end());

	return stdString.empty() || stdString.find_first_of("\r") == 0;
}

int Util::StringFind(const char* stringIn, const char* substring)
{
	auto stdStringIn = std::string(stringIn);
	return (int)stdStringIn.find(substring, 0);
}

glm::vec3 Util::RotMatrixToEuler(RE::NiMatrix3 matrixIn)
{
	auto ent = matrixIn.entry;
	auto rotMat = glm::mat4(
		{ ent[0][0], ent[1][0], ent[2][0],
			ent[0][1], ent[1][1], ent[2][1],
			ent[0][2], ent[1][2], ent[2][2] });

	glm::vec3 rotOut;
	glm::extractEulerAngleXYZ(rotMat, rotOut.x, rotOut.y, rotOut.z);

	return rotOut;
}

constexpr int FIND_COLLISION_MAX_RECURSION = 10;
CollisionFocusObject Util::FindCollisionObjectRecursive(RE::NiNode* nodeIn, int recursionDepth)
{
	CollisionFocusObject objOut;

	if (!nodeIn || recursionDepth > FIND_COLLISION_MAX_RECURSION)
		return objOut;

	auto niCollisionObject = nodeIn->collisionObject;
	if (niCollisionObject) {
		objOut.SetData(niCollisionObject.get(), nodeIn);
		return objOut;
	}

	// sometimes the collision object is on a child
	for (auto child : nodeIn->GetChildren()) {
		if (!child)
			continue;

		auto currNode = child->AsNode();

		if (child->collisionObject) {
			objOut.SetData(child->collisionObject.get(), currNode);
			return objOut;
		} else if (currNode) {
			auto currObj = FindCollisionObjectRecursive(currNode, recursionDepth + 1);

			if (currObj.IsValid)
				return currObj;
		}
	}

	return objOut;
}

float MaxSizeFull = 20.0f;
// 35.0 seems to be the sweet spot. It gives ideal results for armor workbenches and small display cases ->
// on the workbench, it only finds one component, while even for small display cases, both the openeable
// and the static components are found
float MinSizeComponent = 35.0f;

std::vector<CollisionFocusObject> Util::FindCollisionObjectListRecursive(RE::NiNode* nodeIn, float& bestColSize, int recursionDepth)
{
	std::vector<CollisionFocusObject> nodesOut;

	if (!nodeIn || recursionDepth > FIND_COLLISION_MAX_RECURSION)
		return nodesOut;

	auto niCollisionObject = nodeIn->collisionObject;
	if (niCollisionObject) {
		auto objSize = nodeIn->worldBound.radius;

		// if another collision object was already found, only accept further ones of sufficient size, to
		// avoid small components. Eg. armor workbench - there is one big collision object, and one very small one, which
		// is wholly unnecessary, same for barrels with SMIM, where otherwise the lid can be selected separately
		bool betterSize = objSize >= bestColSize;
		if (betterSize || objSize >= MinSizeComponent) {
			auto newObj = CollisionFocusObject(niCollisionObject.get(), nodeIn);
			newObj.UpdateBound();

			nodesOut.push_back(newObj);
			if (betterSize)
				bestColSize = objSize;
		}

		// for small or medium sized objects, don't look at children and return here (unless no object was found yet)
		// large objects, such as city gates should evaluate all children, still, to make sure the best one
		// can be determined - eg. Windhelm city gate, where this would return the topmost part of the gate, which
		// isn't accessible to the player
		if (objSize <= MaxSizeFull)
			return nodesOut;
	}

	for (auto child : nodeIn->GetChildren()) {
		if (!child)
			continue;

		auto currNode = child->AsNode();

		if (child->collisionObject) {
			auto collSize = child->worldBound.radius;
			bool betterSize = collSize >= bestColSize;

			// if collSize is NULL, accept the object anyways because I can't be sure what size it has.
			// Some nodes just don't have worldBound data calculated
			// this is important for example for dessicated corpses, where the biggest node has a worldbound
			// radius of 0.0
			if (collSize == NULL || (betterSize && collSize >= MinSizeComponent)) {
				auto newObj = CollisionFocusObject(child->collisionObject.get(), currNode);
				newObj.UpdateBound();
				nodesOut.push_back(newObj);

				if (betterSize)
					bestColSize = collSize;
			}
		}

		if (currNode) {
			auto subVector = FindCollisionObjectListRecursive(currNode, bestColSize, recursionDepth + 1);
			nodesOut.insert(std::end(nodesOut), std::begin(subVector), std::end(subVector));
		}
	}

	return nodesOut;
}

std::vector<CollisionFocusObject> Util::FindAllCollisionObjectsRecursive(RE::NiNode* nodeIn, int recursionDepth /*= 0*/)
{
	std::vector<CollisionFocusObject> nodesOut;

	if (!nodeIn || recursionDepth > FIND_COLLISION_MAX_RECURSION)
		return nodesOut;

	auto niCollisionObject = nodeIn->collisionObject;
	if (niCollisionObject) {
		auto newObj = CollisionFocusObject(niCollisionObject.get(), nodeIn);
		newObj.UpdateBound();

		nodesOut.push_back(newObj);
	}

	for (auto child : nodeIn->GetChildren()) {
		if (!child)
			continue;

		auto currNode = child->AsNode();
		if (child->collisionObject) {
			auto newObj = CollisionFocusObject(child->collisionObject.get(), currNode);
			newObj.UpdateBound();
			nodesOut.push_back(newObj);
		}

		if (currNode) {
			auto subVector = FindAllCollisionObjectsRecursive(currNode, recursionDepth + 1);
			nodesOut.insert(std::end(nodesOut), std::begin(subVector), std::end(subVector));
		}
	}

	return nodesOut;
}

bool Util::HasCollisionObjectRecursive(RE::NiNode* nodeIn, int recursionDepth /*= 0*/)
{
	if (!nodeIn || recursionDepth > FIND_COLLISION_MAX_RECURSION)
		return false;

	if (nodeIn->collisionObject)
		return true;

	for (auto child : nodeIn->GetChildren()) {
		if (!child)
			continue;

		auto currNode = child->AsNode();
		if (child->collisionObject)
			return true;

		if (currNode && HasCollisionObjectRecursive(currNode, recursionDepth + 1))
			return true;
	}

	return false;
}

std::vector<CollisionFocusObject> Util::FindCollisionObjectListRecursive(RE::NiNode* nodeIn, int recursionDepth)
{
	float colSizeBuffer = -1.0f;
	return FindCollisionObjectListRecursive(nodeIn, colSizeBuffer, recursionDepth);
}

RE::NiNode* Util::GetChildNodeByName(RE::NiNode* nodeIn, const char* targetName)
{
	if (!nodeIn)
		return nullptr;

	for (auto& childObj : nodeIn->GetChildren()) {
		if (!childObj.get())
			continue;

		RE::NiNode* childNode = childObj->AsNode();
		if (!childNode)
			continue;

		if (targetName == childNode->name)
			return childNode;
	}

	return nullptr;
}

glm::vec3 Util::GetBoundingBoxCenter(RE::TESObjectREFR* object)
{
	if (!object)
		return glm::vec3();

	auto objCharacter = object->GetObjectReference()->As<RE::TESNPC>();
	if (objCharacter) {
		RE::NiAVObject* mesh = object->GetCurrent3D();
		if (!mesh)
			return GetObjectAccuratePosition(object);
		RE::NiAVObject* spine = GetCharacterSpine(object);
		if (!spine) {
			auto niBoundCenter = mesh->worldBound.center;
			return glm::vec3(niBoundCenter.x, niBoundCenter.y, niBoundCenter.z);
		}

		auto spineTranslate = spine->world.translate;
		return glm::vec3(spineTranslate.x, spineTranslate.y, spineTranslate.z);
	} else {
		auto mesh = object->GetCurrent3D();
		if (!mesh)
			return GetObjectAccuratePosition(object);

		ObjectBound objectBound;
		if (!GetBoundingBox(object, objectBound)) {
			auto niBoundCenter = mesh->worldBound.center;
			return glm::vec3(niBoundCenter.x, niBoundCenter.y, niBoundCenter.z);
		}

		return GetBoundingBoxCenter(objectBound);
	}
}

glm::vec3 Util::GetBoundingBoxCenter(ObjectBound bb)
{
	auto boundsDiagonal = (bb.boundMax - bb.boundMin) / 2.0f;
	auto boundsDiagonalRotated = Util::RotateVector(bb.rotation, boundsDiagonal);

	return bb.worldBoundMin + boundsDiagonalRotated;
}

glm::vec3 Util::GetBoundingBoxCenter(CollisionFocusObject clObject)
{
	return GetBoundingBoxCenter(clObject.BoundingBox);
}

glm::vec3 Util::GetBoundingBoxCenter(FocusObject* object)
{
	if (!object)
		return glm::vec3();

	if (object->CollisionObject.HasObjectBound)
		return GetBoundingBoxCenter(object->CollisionObject.BoundingBox);
	if (object->CollisionObject.IsValid)
		return GetBoundingBoxCenter(object->CollisionObject);
	else if (object->ObjectRef.get())
		return GetBoundingBoxCenter(object->ObjectRef.get().get());

	return glm::vec3();
}

glm::vec3 Util::GetBoundingBoxTop(RE::TESObjectREFR* object)
{
	if (!object)
		return glm::vec3();

	ObjectBound objectBound;
	if (!GetBoundingBox(object, objectBound))
		return GetObjectAccuratePosition(object);

	return GetBoundingBoxTop(objectBound);
}

glm::vec3 Util::GetBoundingBoxTop(ObjectBound bb)
{
	auto boundsDiagonalHalf = (bb.boundMax - bb.boundMin) / 2.0f;
	auto boundsDiagonalHalfRotated = Util::RotateVector(bb.rotation, boundsDiagonalHalf);

	auto posOut = bb.worldBoundMin + boundsDiagonalHalfRotated;
	posOut.z += boundsDiagonalHalf.z;

	return posOut;
}

glm::vec3 Util::GetBoundingBoxTop(CollisionFocusObject clObject)
{
	return GetBoundingBoxTop(clObject.BoundingBox);
}

glm::vec3 Util::GetBoundingBoxTop(FocusObject* object)
{
	if (!object)
		return glm::vec3();

	if (object->CollisionObject.HasObjectBound)
		return GetBoundingBoxTop(object->CollisionObject.BoundingBox);
	if (object->CollisionObject.IsValid)
		return GetBoundingBoxTop(object->CollisionObject);
	else if (object->ObjectRef.get())
		return GetBoundingBoxTop(object->ObjectRef.get().get());

	return glm::vec3();
}

glm::vec3 Util::GetBoundingBoxBottom(RE::TESObjectREFR* object)
{
	if (!object)
		return glm::vec3();

	ObjectBound objectBound;
	if (!GetBoundingBox(object, objectBound))
		return GetObjectAccuratePosition(object);

	return GetBoundingBoxBottom(objectBound);
}

glm::vec3 Util::GetBoundingBoxBottom(ObjectBound bb)
{
	auto boundsDiagonalHalf = (bb.boundMax - bb.boundMin) / 2.0f;
	auto boundsDiagonalHalfRotated = Util::RotateVector(bb.rotation, boundsDiagonalHalf);

	auto posOut = bb.worldBoundMin + boundsDiagonalHalfRotated;
	posOut.z -= boundsDiagonalHalf.z;

	return posOut;
}

glm::vec3 Util::GetBoundingBoxBottom(CollisionFocusObject clObject)
{
	return GetBoundingBoxBottom(clObject.BoundingBox);
}

glm::vec3 Util::GetBoundingBoxBottom(FocusObject* object)
{
	if (!object)
		return glm::vec3();

	if (object->CollisionObject.HasObjectBound)
		return GetBoundingBoxBottom(object->CollisionObject.BoundingBox);
	if (object->CollisionObject.IsValid)
		return GetBoundingBoxBottom(object->CollisionObject);
	else if (object->ObjectRef.get())
		return GetBoundingBoxBottom(object->ObjectRef.get().get());

	return glm::vec3();
}

RE::NiAVObject* Util::GetCharacterSpine(RE::TESObjectREFR* object)
{
	auto characterObject = object->GetObjectReference()->As<RE::TESNPC>();
	auto mesh = object->GetCurrent3D();

	if (characterObject && mesh) {
		auto spineNode = mesh->GetObjectByName("NPC Spine [Spn0]");
		if (spineNode)
			return spineNode;
	}

	return mesh;
}

RE::NiAVObject* Util::GetCharacterHead(RE::TESObjectREFR* object)
{
	auto characterObject = object->GetObjectReference()->As<RE::TESNPC>();
	auto mesh = object->GetCurrent3D();

	if (characterObject && mesh) {
		auto spineNode = mesh->GetObjectByName("NPC Head [Head]");
		if (spineNode)
			return spineNode;
	}

	return mesh;
}

bool Util::IsRoughlyEqual(float first, float second, float maxDif)
{
	return abs(first - second) <= maxDif;
}

glm::vec3 Util::QuatToEuler(glm::quat q)
{
	auto matrix = glm::toMat4(q);

	glm::vec3 rotOut;
	glm::extractEulerAngleXYZ(matrix, rotOut.x, rotOut.y, rotOut.z);

	return rotOut;
}

glm::quat Util::EulerToQuat(glm::vec3 rotIn)
{
	auto matrix = glm::eulerAngleXYZ(rotIn.x, rotIn.y, rotIn.z);
	return glm::toQuat(matrix);
}

glm::vec3 Util::GetInverseRotation(glm::vec3 rotIn)
{
	auto matrix = glm::eulerAngleXYZ(rotIn.y, rotIn.x, rotIn.z);
	auto inverseMatrix = glm::inverse(matrix);

	glm::vec3 rotOut;
	glm::extractEulerAngleYXZ(inverseMatrix, rotOut.x, rotOut.y, rotOut.z);
	return rotOut;
}

glm::quat Util::GetInverseRotation(glm::quat rotIn)
{
	return glm::inverse(rotIn);
}

glm::vec3 Util::EulerRotationToVector(glm::vec3 rotIn)
{
	return glm::vec3(
		cos(rotIn.y) * cos(rotIn.x),
		sin(rotIn.y) * cos(rotIn.x),
		sin(rotIn.x));
}

glm::vec3 Util::VectorToEulerRotation(glm::vec3 vecIn)
{
	float yaw = atan2(vecIn.x, vecIn.y);
	float pitch = atan2(vecIn.z, sqrt((vecIn.x * vecIn.x) + (vecIn.y * vecIn.y)));

	return glm::vec3(pitch, 0.0f, yaw);
}

glm::vec3 Util::GetCameraPos()
{
	auto playerCam = RE::PlayerCamera::GetSingleton();
	return glm::vec3(playerCam->pos.x, playerCam->pos.y, playerCam->pos.z);
}

glm::quat Util::GetCameraRot()
{
	auto playerCam = RE::PlayerCamera::GetSingleton();

	auto cameraState = playerCam->currentState.get();
	if (!cameraState)
		return glm::quat();

	RE::NiQuaternion niRotation;
	cameraState->GetRotation(niRotation);

	return glm::quat(niRotation.w, niRotation.x, niRotation.y, niRotation.z);
}

bool Util::IsPosBehindPlayerCamera(glm::vec3 pos)
{
	auto cameraPos = GetCameraPos();
	auto cameraRot = GetCameraRot();

	auto toTarget = Util::NormalizeVector(pos - cameraPos);
	auto cameraForward = Util::NormalizeVector(Util::GetForwardVector(cameraRot));

	auto angleDif = abs(glm::length(toTarget - cameraForward));

	// root_two is the diagonal length of a 1x1 square. When comparing normalized forward
	// vectors, this accepts an angle of 90 degrees in all directions
	return angleDif > glm::root_two<float>();
}

glm::vec3 Util::GetBoundRightVectorRotated(ObjectBound objectBound)
{
	glm::vec3 bound(abs(objectBound.boundMin.x - objectBound.boundMax.x), 0.0f, 0.0f);
	auto boundRotated = RotateVector(objectBound.rotation, bound);

	return boundRotated;
}

glm::vec3 Util::GetBoundForwardVectorRotated(ObjectBound objectBound)
{
	glm::vec3 bound(0.0f, abs(objectBound.boundMin.y - objectBound.boundMax.y), 0.0f);
	auto boundRotated = RotateVector(objectBound.rotation, bound);

	return boundRotated;
}

glm::vec3 Util::GetBoundUpVectorRotated(ObjectBound objectBound)
{
	glm::vec3 bound(0.0f, 0.0f, abs(objectBound.boundMin.z - objectBound.boundMax.z));
	auto boundRotated = RotateVector(objectBound.rotation, bound);

	return boundRotated;
}

glm::vec3 Util::GetPointOnRotatedCircle(glm::vec3 origin, float radius, float i, float maxI, glm::vec3 eulerAngles)
{
	float currAngle = (i / maxI) * glm::two_pi<float>();

	glm::vec3 targetPos(
		(radius * cos(currAngle)),
		(radius * sin(currAngle)),
		0.0f);

	auto targetPosRotated = Util::RotateVector(eulerAngles, targetPos);

	return glm::vec3(targetPosRotated.x + origin.x, targetPosRotated.y + origin.y, targetPosRotated.z + origin.z);
}

void Util::ForEachReferenceInRange(RE::NiPoint3 originPos, float a_radius, std::function<bool(RE::TESObjectREFR& a_ref)> a_callback)
{
	auto tes = RE::TES::GetSingleton();

	if (tes->interiorCell) {
		Util::CellForEachReferenceInRange(tes->interiorCell, originPos, a_radius, [&](RE::TESObjectREFR& a_ref) {
			return a_callback(a_ref);
		});
	} else {
		if (const auto gridLength = tes->gridCells ? tes->gridCells->length : 0; gridLength > 0) {
			const float yPlus = originPos.y + a_radius;
			const float yMinus = originPos.y - a_radius;
			const float xPlus = originPos.x + a_radius;
			const float xMinus = originPos.x - a_radius;

			std::uint32_t x = 0;
			do {
				std::uint32_t y = 0;
				do {
					auto cell = tes->gridCells->GetCell(x, y);
					if (cell && cell->IsAttached()) {
						const auto cellCoords = cell->GetCoordinates();
						if (cellCoords) {
							const RE::NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
							if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus && (worldPos.y + 4096.0f) > yMinus) {
								Util::CellForEachReferenceInRange(cell, originPos, a_radius, [&](RE::TESObjectREFR& a_ref) {
									return a_callback(a_ref);
								});
							}
						}
					}
					++y;
				} while (y < gridLength);
				++x;
			} while (x < gridLength);
		}
	}

	if (const auto skyCell = tes->worldSpace ? tes->worldSpace->skyCell : nullptr; skyCell) {
		Util::CellForEachReferenceInRange(skyCell, originPos, a_radius, [&](RE::TESObjectREFR& a_ref) {
			return a_callback(a_ref);
		});
	}
}

void Util::CellForEachReference(RE::TESObjectCELL* cell, std::function<bool(RE::TESObjectREFR&)> a_callback)
{
	if (!cell)
		return;

	RE::BSSpinLockGuard locker(cell->GetRuntimeData().spinLock);
	for (const auto& ref : cell->GetRuntimeData().references) {
		if (ref && !a_callback(*ref)) {
			break;
		}
	}
}

void Util::CellForEachReferenceInRange(RE::TESObjectCELL* cell, const RE::NiPoint3& a_origin, float a_radius, std::function<bool(RE::TESObjectREFR&)> a_callback)
{
	if (!cell)
		return;

	CellForEachReference(cell, [&](RE::TESObjectREFR& ref) {
		// make a simple distance check first, with some added to the acceptance radius, then
		// for all accepted objects, make a more precise distance check toward the center of their
		// bounding box - to avoid calculating bounding box data for every object in the cell(s)

		const auto roughDistance = (a_origin - ref.GetPosition()).Length();
		if (roughDistance <= a_radius + ROUGH_DISTANCE_ADD) {
			const auto preciseDistance = Util::GetDistanceToBounds(glm::vec3(a_origin.x, a_origin.y, a_origin.z), &ref);
			if (preciseDistance <= a_radius) {
				return a_callback(ref);
			}
		}
		return true;
	});
}

bool Util::IsValid(float numIn)
{
	return !(numIn < NEGATIVE_INVALID_THRESHHOLD ||
			 numIn > POSITIVE_INVALID_THRESHHOLD ||
			 std::isinf(numIn) ||
			 std::isinf(-numIn) ||
			 std::isnan(numIn) ||
			 std::isnan(-numIn));
}

bool Util::IsValid(glm::vec3 vecIn)
{
	return IsValid(vecIn.x) && IsValid(vecIn.y) && IsValid(vecIn.z);
}

float Util::MakeValid(float numIn, float setInvalidTo)
{
	if (IsValid(numIn))
		return numIn;
	return setInvalidTo;
}

glm::vec3 Util::MakeValid(glm::vec3 vecIn, float setInvalidTo)
{
	return glm::vec3(
		MakeValid(vecIn.x, setInvalidTo),
		MakeValid(vecIn.y, setInvalidTo),
		MakeValid(vecIn.z, setInvalidTo));
}

glm::vec3 Util::GetObjectAccuratePosition(RE::TESObjectREFR* object)
{
	auto mesh = object->GetCurrent3D();

	// backup, if no mesh is found
	if (!mesh) {
		auto niPos = object->GetPosition();
		return glm::vec3(niPos.x, niPos.y, niPos.z);
	}

	auto niPos = mesh->world.translate;
	return glm::vec3(niPos.x, niPos.y, niPos.z);
}

float Util::GetDistanceToBounds(glm::vec3 start, RE::TESObjectREFR* object)
{
	auto position = GetBoundingBoxCenter(object);
	return glm::length(position - start);
}

EAxis Util::GetBoundsLongestAxis(ObjectBound& bound)
{
	auto boundDiagonal = bound.boundMax - bound.boundMin;

	if (boundDiagonal.x > boundDiagonal.y) {
		if (boundDiagonal.x > boundDiagonal.z)
			return EAxis::AxisX;
		return EAxis::AxisZ;
	}

	if (boundDiagonal.y > boundDiagonal.z)
		return EAxis::AxisY;
	return EAxis::AxisZ;
}

glm::vec3 Util::GetAxisOrigin(ObjectBound& bound, EAxis axis)
{
	auto centerPos = Util::GetBoundingBoxCenter(bound);
	auto boundDiagonal = bound.boundMax - bound.boundMin;

	glm::vec3 originAdd(0.0f, 0.0f, 0.0f);

	switch (axis) {
	case EAxis::AxisX:
		originAdd.x -= boundDiagonal.x / 2.0f;

		break;
	case EAxis::AxisY:
		originAdd.y -= boundDiagonal.y / 2.0f;

		break;
	case EAxis::AxisZ:
		originAdd.z -= boundDiagonal.z / 2.0f;

		break;
	}

	auto originAddRotated = Util::RotateVector(bound.rotation, originAdd);

	return centerPos + originAddRotated;
}

RE::TESObjectREFR* Util::GetPlayerMountRef()
{
	auto playerChar = RE::PlayerCharacter::GetSingleton();

	if (playerChar->IsOnMount()) {
		auto extraData = playerChar->extraList.GetByType<RE::ExtraInteraction>();
		if (extraData) {
			auto mount = extraData->interaction.get()->actor;
			auto mountRef = mount.get()->AsReference();

			return mountRef;
		}
	}

	return nullptr;
}
