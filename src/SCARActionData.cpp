#include "SCARActionData.h"
#include "DataHandler.h"
#include "Function.h"

#undef GetObject

namespace SCAR
{
	void from_json(const json& j, SCARActionData& a_data)
	{
		if (j.find("AttackData") != j.end()) {
			j.at("AttackData").get_to(a_data.attackDataName);
		} else {
			j.at("IdleAnimation").get_to(a_data.IdleAnimationEditorID);
		}

		j.at("Type").get_to(a_data.actionType);

		j.at("MinDistance").get_to(a_data.minDistance);
		a_data.minDistance = std::max(a_data.minDistance, 0.f);

		j.at("MaxDistance").get_to(a_data.maxDistance);
		a_data.maxDistance = std::max(a_data.maxDistance, 0.f);

		j.at("StartAngle").get_to(a_data.startAngle);
		a_data.startAngle = std::clamp(a_data.startAngle, -180.f, 180.f);

		j.at("EndAngle").get_to(a_data.endAngle);
		a_data.endAngle = std::clamp(a_data.endAngle, -180.f, 180.f);

		j.at("Chance").get_to(a_data.chance);
		a_data.chance = std::clamp(a_data.chance, -180.f, 180.f);

		if (j.find("TriggerStartTime") != j.end())
			a_data.triggerStartTime = std::max(j.at("TriggerStartTime").get<float>(), 0.f);

		if (j.find("TriggerEndTime") != j.end())
			a_data.triggerEndTime = std::max(j.at("TriggerEndTime").get<float>(), a_data.triggerStartTime);

		if (j.find("WeaponLength") != j.end())
			a_data.weaponLength.emplace(std::max(j.at("WeaponLength").get<float>(), 0.f));

		if (j.find("CoolDownTime") != j.end())
			a_data.coolDownTime.emplace(std::max(j.at("CoolDownTime").get<float>(), 0.f));

		if (j.find("CoolDownAlias") != j.end())
			a_data.coolDownAlias.emplace(j.at("CoolDownAlias").get<std::string>());

		if (j.find("ConditionsAlias") != j.end())
			a_data.conditionsAlias.emplace(j.at("ConditionsAlias").get<std::string>());

		if (j.find("VariantID") != j.end())
			a_data.variantID = std::max(j.at("VariantID").get<std::int32_t>(), 0);
	}

	const bool SCARActionData::IsLeftAttack() const
	{
		return _strcmpi(actionType.c_str(), "LA") == 0 || _strcmpi(actionType.c_str(), "LPA") == 0;
	}

	const bool SCARActionData::IsBashAttack() const
	{
		return _strcmpi(actionType.c_str(), "BA") == 0 || _strcmpi(actionType.c_str(), "BPA") == 0;
	}

	float SCARActionData::GetWeaponReach(RE::Actor* a_attacker) const
	{
		if (a_attacker) {
			if (weaponLength.has_value()) {
				return weaponLength.value() * a_attacker->GetScale();
			}

			if (IsBashAttack()) {
				auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting("fCombatBashReach");
				if (setting)
					return setting->GetFloat() * a_attacker->GetScale();
			} else {
				const bool leftAttack = IsLeftAttack();
				auto dataHandler = DataHandler::GetSingleton();
				if (dataHandler && dataHandler->precisionPtr) {
					auto collisionType = leftAttack ? RequestedAttackCollisionType::LeftWeapon : RequestedAttackCollisionType::RightWeapon;
					return (dataHandler->precisionPtr->GetAttackCollisionCapsuleLength(a_attacker->GetHandle(), collisionType) + a_attacker->GetBoundRadius());
				} else {
					auto obj = a_attacker->GetEquippedObject(leftAttack);
					auto weap = obj ? obj->As<RE::TESObjectWEAP>() : nullptr;
					auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting("fCombatDistance");
					if (weap && setting) {
						return weap->GetReach() * setting->GetFloat() * a_attacker->GetScale();
					}
				}
			}

			return a_attacker->GetRace() ? a_attacker->GetRace()->data.unarmedReach : 0.f;
		}

		return 0.f;
	}

	const bool SCARActionData::IsInAttackCoolDown(RE::CombatBehaviorContextMelee::CombatAttackData* a_coolDownData) const
	{
		if (a_coolDownData && RE::AITimer::QTimer() - a_coolDownData->cooldown_timer.aiTimer <= a_coolDownData->cooldown_timer.timer) {
			return true;
		}

		return false;
	}

	void SCARActionData::UpdateAttackCoolDown(RE::CombatBehaviorContextMelee::CombatAttackData* a_coolDownData)
	{
		if (a_coolDownData && a_coolDownData->attackdata) {
			a_coolDownData->cooldown_timer.aiTimer = RE::AITimer::QTimer();
			a_coolDownData->cooldown_timer.timer = coolDownTime.has_value() ? coolDownTime.value() : a_coolDownData->attackdata->data.recoveryTime;
		}
	}

	const DefaultObject SCARActionData::GetActionObject() const
	{
		static std::map<const std::string, const DefaultObject> actionMap = {
			{ "RA", DefaultObject::kActionRightAttack },
			{ "RPA", DefaultObject::kActionRightPowerAttack },
			{ "LA", DefaultObject::kActionLeftAttack },
			{ "LPA", DefaultObject::kActionLeftPowerAttack },
			{ "DA", DefaultObject::kActionDualAttack },
			{ "DPA", DefaultObject::kActionDualPowerAttack },
			{ "BA", DefaultObject::kActionRightAttack },
			{ "BPA", DefaultObject::kActionRightPowerAttack },
			{ "IDLE", DefaultObject::kActionIdle }
		};

		auto itr = actionMap.find(actionType);
		return itr != actionMap.end() ? itr->second : DefaultObject::kActionRightAttack;
	}

	const ATTACK_TYPE SCARActionData::GetAttackType() const
	{
		static std::map<const std::string, const ATTACK_TYPE> actionMap = {
			{ "RA", ATTACK_TYPE::WeaponRight },
			{ "RPA", ATTACK_TYPE::WeaponRight },
			{ "LA", ATTACK_TYPE::WeaponLeft },
			{ "LPA", ATTACK_TYPE::WeaponLeft },
			{ "DA", ATTACK_TYPE::WeaponRight },
			{ "DPA", ATTACK_TYPE::WeaponRight },
			{ "BA", ATTACK_TYPE::Shield },
			{ "BPA", ATTACK_TYPE::Shield },
			{ "IDLE", ATTACK_TYPE::WeaponRight }
		};

		auto itr = actionMap.find(actionType);
		return itr != actionMap.end() ? itr->second : ATTACK_TYPE::WeaponRight;
	}

	bool SCARActionData::PerformSpecialIdle(RE::Actor* a_attacker, RE::Actor* a_target, RE::BGSAction* a_action, RE::TESIdleForm* a_Idle)
	{
		if (!a_attacker || !a_target || !a_attacker->GetActorRuntimeData().currentProcess)
			return false;

		if (!a_Idle->CheckConditions(a_attacker, a_target, false)) {
			return false;
		}

		auto actionData = RE::TESActionData::Create();
		if (!actionData) {
			return false;
		}

		actionData->source.reset(a_attacker);
		actionData->target.reset(a_target);
		actionData->animObjIdle = a_Idle;
		actionData->animEvent = a_Idle->animEventName;
		actionData->action = a_action;

		a_attacker->SetGraphVariableInt("SCAR_AttackVariants", variantID);
		auto result = actionData->Process();

		actionData->~TESActionData();
		RE::free(actionData);
		actionData = nullptr;

		return result;
	}

	bool SCARActionData::PerformSCARAction(RE::Actor* a_attacker, RE::Actor* a_target, RE::CombatBehaviorContextMelee* a_context, RE::hkbClipGenerator* a_clip)
	{
		if (!a_attacker || !a_target || !a_attacker->GetActorRuntimeData().currentProcess)
			return false;

		if (!a_context || !a_context->combatattackdatas.size()) {
			return false;
		}

		if (a_clip->localTime < triggerStartTime || a_clip->localTime > triggerEndTime) {
			return false;
		}

		auto actionObj = GetActionObject();
		auto attackType = GetAttackType();
		if (actionObj != DefaultObject::kActionIdle && attackType != a_context->attack_type) {
			return false;
		};

		static auto GetCombatData = [this](RE::CombatBehaviorContextMelee* a_context, const std::string a_dataName) -> RE::CombatBehaviorContextMelee::CombatAttackData* {
			for (auto& combatData : a_context->combatattackdatas) {
				if (combatData.attackdata && _strcmpi(combatData.attackdata->event.c_str(), a_dataName.c_str()) == 0) {
					return &combatData;
				}
			}

			return nullptr;
		};

		const float weaponReach = weaponLength.has_value() ? weaponLength.value() * a_attacker->GetScale() : a_context->reach;
		if (chance >= Random::get<float>(0.f, 100.f) && AttackRangeCheck::WithinAttackRange(a_attacker, a_target, maxDistance + weaponReach, minDistance, GetStartRadian(), GetEndRadian())) {
			if (!IdleAnimationEditorID.empty()) {
				auto IdleAnimation = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(IdleAnimationEditorID);
				if (!IdleAnimation) {
					ERROR("Not Vaild Idle Animation Form Get: \"{}\"!", IdleAnimationEditorID);
					return false;
				}

				auto defaultObjMgr = RE::BGSDefaultObjectManager::GetSingleton();
				if (!defaultObjMgr)
					return false;

				auto actionForm = defaultObjMgr->GetObject(actionObj);
				auto action = actionForm ? actionForm->As<RE::BGSAction>() : nullptr;
				if (!action) {
					ERROR("Not Vaild Action Type Get: \"{}\"!", actionType);
					return false;
				}

				auto coolDownData = coolDownAlias.has_value() ? GetCombatData(a_context, coolDownAlias.value()) : nullptr;
				if (IsInAttackCoolDown(coolDownData)) {
					return false;
				}

				auto result = PerformSpecialIdle(a_attacker, a_target, action, IdleAnimation);
				if (result) {
					UpdateAttackCoolDown(coolDownData);

					DEBUG("Perform SCAR Action! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
						IdleAnimationEditorID, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);
					AttackRangeCheck::DrawOverlay(a_attacker, a_target, maxDistance + weaponReach, minDistance, GetStartRadian(), GetEndRadian());
				} else
					DEBUG("Play Idle Fail! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
						IdleAnimationEditorID, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);

				return result;
			} else if (!attackDataName.empty()) {
				auto combatData = GetCombatData(a_context, attackDataName);
				if (!combatData) {
					return false;
				}

				auto coolDownData = coolDownAlias.has_value() ? GetCombatData(a_context, coolDownAlias.value()) : combatData;
				if (IsInAttackCoolDown(coolDownData)) {
					return false;
				}

				RE::CombatAnimation* combatAnim = RE::CombatAnimation::Create(a_attacker, RE::CombatAnimation::ANIM::kActionRightAttack);
				auto attackData = combatData->attackdata;
				if (!combatAnim || !attackData) {
					return false;
				}

				if (conditionsAlias.has_value()) {
					auto IdleAnimation = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(conditionsAlias.value());
					if (IdleAnimation && !IdleAnimation->CheckConditions(a_attacker, a_target, false)) {
						return false;
					}
				}

				a_attacker->SetGraphVariableInt("SCAR_AttackVariants", variantID);
				combatAnim->animEvent = attackData->event;
				auto result = (combatAnim->Execute());
				if (result) {
					UpdateAttackCoolDown(coolDownData);

					DEBUG("Perform SCAR Attack! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
						attackDataName, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);
					AttackRangeCheck::DrawOverlay(a_attacker, a_target, maxDistance + weaponReach, minDistance, GetStartRadian(), GetEndRadian());
				} else {
					DEBUG("Fail to do SCAR Attack! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
						attackDataName, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);
				}

				combatAnim->~CombatAnimation();
				RE::free(combatAnim);
				combatAnim = nullptr;

				return result;
			}
		}

		return false;
	}

	bool SCARActionData::SortByWeight(SCARActionData a_data1, SCARActionData a_data2)
	{
		return a_data1.weight >= a_data2.weight;
	}
}
