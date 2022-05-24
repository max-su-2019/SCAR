#include "Hook_AttackStart.h"

namespace SCAR
{
	static constexpr char FIRST_NORMAL_DISTANCE_MAX[] = "SCAR_firstattackdistancemax",
						  FIRST_NORMAL_DISTANCE_MIN[] = "SCAR_firstattackdistancemin",
						  FIRST_POWER_DISTANCE_MAX[] = "SCAR_firstpowerattackdistancemax",
						  FIRST_POWER_DISTANCE_MIN[] = "SCAR_firstpowerattackdistancemin";

	bool RecheckAttackDistancHook::RecheckAttackDistance(bool a_originResult, RE::Actor* a_attacker, RE::Actor* a_target, RE::AttackData* a_attackData)
	{
		std::map<const std::string, float> distMap = {
			std::make_pair(FIRST_NORMAL_DISTANCE_MAX, 0.f),
			std::make_pair(FIRST_NORMAL_DISTANCE_MIN, 0.f),
			std::make_pair(FIRST_POWER_DISTANCE_MAX, 0.f),
			std::make_pair(FIRST_POWER_DISTANCE_MIN, 0.f),
		};

		if (a_attacker && !a_attacker->IsBlocking() && a_target && GetDistanceVariable(a_attacker, distMap)) {
			auto dataHandler = DataHandler::GetSingleton();
			const float startAngle = dataHandler->settings->GetStartAngle();
			const float endAngle = dataHandler->settings->GetEndAngle();

			if (a_attacker->RequestLOS(a_target) && AttackRangeCheck::CheckPathing(a_attacker, a_target) &&
				(AttackRangeCheck::WithinAttackRange(a_attacker, a_target, a_attacker->GetReach() + distMap.at(FIRST_NORMAL_DISTANCE_MAX), distMap.at(FIRST_NORMAL_DISTANCE_MIN), startAngle, endAngle) ||
					AttackRangeCheck::WithinAttackRange(a_attacker, a_target, a_attacker->GetReach() + distMap.at(FIRST_POWER_DISTANCE_MAX), distMap.at(FIRST_POWER_DISTANCE_MIN), startAngle, endAngle))) {
				for (auto pair : distMap) {
					logger::debug("{} value is {}", pair.first, pair.second);
				}
				logger::debug("Tagre in Attack Distance");
				return true;
			}

			return false;
		} else
			return a_originResult;
	}

	bool AttackActionHook::PerformAttackAction(RE::TESActionData* a_actionData)
	{
		if (a_actionData) {
			bool result = false;

			std::map<const std::string, float> distMap = {
				std::make_pair(FIRST_NORMAL_DISTANCE_MAX, 0.f),
				std::make_pair(FIRST_NORMAL_DISTANCE_MIN, 0.f),
				std::make_pair(FIRST_POWER_DISTANCE_MAX, 0.f),
				std::make_pair(FIRST_POWER_DISTANCE_MIN, 0.f),
			};

			auto attacker = a_actionData->Subject_8 ? a_actionData->Subject_8->As<RE::Actor>() : nullptr;
			auto targ = attacker ? attacker->currentCombatTarget.get() : nullptr;
			if (attacker && !attacker->IsBlocking() && attacker->currentProcess && !attacker->IsPlayerRef() && targ &&
				GetDistanceVariable(attacker, distMap) && attacker->RequestLOS(targ.get()) && AttackRangeCheck::CheckPathing(attacker, targ.get())) {
				attacker->SetGraphVariableFloat(NEXT_ATTACK_CHANCE, 100.f);
				attacker->SetGraphVariableInt("MCO_nextattack", 1);
				attacker->SetGraphVariableInt("MCO_nextpowerattack", 1);

				auto dataHandler = DataHandler::GetSingleton();
				const float startAngle = dataHandler->settings->GetStartAngle();
				const float endAngle = dataHandler->settings->GetEndAngle();

				const float powerAttackChance = dataHandler->settings->powerAttackChance.get_data();

				auto InNormalRange = AttackRangeCheck::WithinAttackRange(attacker, targ.get(), attacker->GetReach() + distMap.at(FIRST_NORMAL_DISTANCE_MAX), distMap.at(FIRST_NORMAL_DISTANCE_MIN), startAngle, endAngle);
				auto InPowerRange = AttackRangeCheck::WithinAttackRange(attacker, targ.get(), attacker->GetReach() + distMap.at(FIRST_POWER_DISTANCE_MAX), distMap.at(FIRST_POWER_DISTANCE_MIN), startAngle, endAngle);

				auto normalAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCNormalAttack");  //SCAR_NPCNormalAttack Idle Form
				auto powerAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCPowerAttack");    //SCAR_PowerAttack Idle Form

				if (powerAttackIdle && InPowerRange && powerAttackChance >= Random::get<float>(0.f, 100.f)) {
					logger::debug("Power Atatck Start! Subject: \"{}\"", attacker->GetName());
					result = attacker->currentProcess->PlayIdle(attacker, RE::DEFAULT_OBJECT::kActionRightPowerAttack, powerAttackIdle, true, true, targ.get());
					if (result)
						AttackRangeCheck::DrawOverlay(attacker, targ.get(), attacker->GetReach() + distMap.at(FIRST_POWER_DISTANCE_MAX), distMap.at(FIRST_POWER_DISTANCE_MIN), startAngle, endAngle);
				} else if (normalAttackIdle && InNormalRange) {
					logger::debug("Normal Atatck Start! Subject: \"{}\"", attacker->GetName());
					result = attacker->currentProcess->PlayIdle(attacker, RE::DEFAULT_OBJECT::kActionRightAttack, normalAttackIdle, true, true, targ.get());
					if (result)
						AttackRangeCheck::DrawOverlay(attacker, targ.get(), attacker->GetReach() + distMap.at(FIRST_NORMAL_DISTANCE_MAX), distMap.at(FIRST_NORMAL_DISTANCE_MIN), startAngle, endAngle);
				}

				return result;
			}
		}

		return _PerformAttackAction(a_actionData);
	}

}
