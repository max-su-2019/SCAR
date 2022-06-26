#include "Function.h"
#include "SCARActionData.h"

namespace SCAR
{
	void from_json(const json& j, SCARActionData& a_data)
	{
		j.at("IdleAnimation").get_to(a_data.IdleAnimationEditorID);
		j.at("MinDistance").get_to(a_data.minDistance);
		j.at("MaxDistance").get_to(a_data.maxDistance);
		j.at("StartAngle").get_to(a_data.startAngle);
		j.at("EndAngle").get_to(a_data.endAngle);
		j.at("Chance").get_to(a_data.chance);
		j.at("Type").get_to(a_data.actionType);
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
			{ "IDLE", DefaultObject::kActionIdle }
		};

		auto itr = actionMap.find(actionType);
		return itr != actionMap.end() ? itr->second : DefaultObject::kActionRightAttack;
	}

	bool SCARActionData::PerformSCARAction(RE::Actor* a_attacker, RE::Actor* a_target)
	{
		if (a_attacker && a_target && a_attacker->currentProcess &&
			chance >= Random::get<float>(0.f, 100.f) && AttackRangeCheck::WithinAttackRange(a_attacker, a_target, maxDistance + a_attacker->GetReach(), minDistance, GetStartAngle(), GetEndAngle())) {
			auto IdleAnimation = RE::TESForm::LookupByEditorID<RE::TESIdleForm>(IdleAnimationEditorID);
			if (!IdleAnimation) {
				logger::error("Not Vaild Idle Animation Form Get: \"{}\"!", IdleAnimationEditorID);
				return false;
			}

			auto result = a_attacker->currentProcess->PlayIdle(a_attacker, GetActionObject(), IdleAnimation, true, true, a_target);
			if (result) {
				logger::debug("Perform SCAR Action! Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
					IdleAnimationEditorID, minDistance, maxDistance, startAngle, endAngle, chance, actionType, weight);
				AttackRangeCheck::DrawOverlay(a_attacker, a_target, maxDistance + a_attacker->GetReach(), minDistance, GetStartAngle(), GetEndAngle());
			}

			return result;
		}

		return false;
	}

	bool SCARActionData::SortByWeight(SCARActionData a_data1, SCARActionData a_data2)
	{
		return a_data1.weight >= a_data2.weight;
	}
}
