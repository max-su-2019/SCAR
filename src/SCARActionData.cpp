#include "Function.h"
#include "SCARActionData.h"

namespace SCAR
{
	void from_json(const json& j, SCARActionData& a_data)
	{
		j.at("IdleAnimation").get_to(a_data.IdleAnimationEditorID);

		j.at("MinDistance").get_to(a_data.minDistance);
		a_data.minDistance = max(a_data.minDistance, 0.f);

		j.at("MaxDistance").get_to(a_data.maxDistance);
		a_data.maxDistance = max(a_data.maxDistance, 0.f);

		j.at("StartAngle").get_to(a_data.startAngle);
		a_data.startAngle = std::clamp(a_data.startAngle, -180.f, 180.f);

		j.at("EndAngle").get_to(a_data.endAngle);
		a_data.endAngle = std::clamp(a_data.endAngle, -180.f, 180.f);

		j.at("Chance").get_to(a_data.chance);
		a_data.chance = std::clamp(a_data.chance, -180.f, 180.f);

		j.at("Type").get_to(a_data.actionType);
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
			if (IsBashAttack()) {
				auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting("fCombatBashReach");
				if (setting)
					return setting->GetFloat() * a_attacker->GetScale();
			} else {
				auto obj = a_attacker->GetEquippedObject(IsLeftAttack());
				auto weap = obj ? obj->As<RE::TESObjectWEAP>() : nullptr;
				auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting("fCombatDistance");
				if (weap && setting) {
					return weap->GetReach() * setting->GetFloat() * a_attacker->GetScale();
				}
			}
		}

		return a_attacker->GetRace() ? a_attacker->GetRace()->data.unarmedReach : 0.f;
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

	bool SCARActionData::PerformSCARAction(RE::Actor* a_attacker, RE::Actor* a_target)
	{
		if (!a_attacker || !a_target || !a_attacker->currentProcess)
			return false;

		const float weaponReach = GetWeaponReach(a_attacker);
		if (chance >= Random::get<float>(0.f, 100.f) && AttackRangeCheck::WithinAttackRange(a_attacker, a_target, maxDistance + weaponReach, minDistance, GetStartAngle(), GetEndAngle())) {
			auto IdleAnimation = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(IdleAnimationEditorID);
			if (!IdleAnimation) {
				logger::error("Not Vaild Idle Animation Form Get: \"{}\"!", IdleAnimationEditorID);
				return false;
			}

			auto result = a_attacker->currentProcess->PlayIdle(a_attacker, GetActionObject(), IdleAnimation, true, true, a_target);
			if (result) {
				logger::debug("Perform SCAR Action! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
					IdleAnimationEditorID, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);
				AttackRangeCheck::DrawOverlay(a_attacker, a_target, maxDistance + weaponReach, minDistance, GetStartAngle(), GetEndAngle());
			} else
				logger::debug("Play Idle Fail! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
					IdleAnimationEditorID, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);

			return result;
		}

		return false;
	}

	bool SCARActionData::SortByWeight(SCARActionData a_data1, SCARActionData a_data2)
	{
		return a_data1.weight >= a_data2.weight;
	}
}
