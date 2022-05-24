#include "DataHandler.h"
#include "Function.h"
#include "Hook_AttackCombo.h"

namespace SCAR
{
	static constexpr char NEXT_NORMAL_DISTANCE_MAX[] = "SCAR_nextattackdistancemax",
						  NEXT_NORMAL_DISTANCE_MIN[] = "SCAR_nextattackdistancemin",
						  NEXT_POWER_DISTANCE_MAX[] = "SCAR_nextpowerattackdistancemax",
						  NEXT_POWER_DISTANCE_MIN[] = "SCAR_nextpowerattackdistancemin";

	EventResult AnimEventHook::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event || !a_event->holder)
			return _ProcessEvent(a_sink, a_event, a_eventSource);

		auto actor = a_event->holder->As<RE::Actor>();
		auto combatTarg = actor ? actor->currentCombatTarget.get() : nullptr;

		if (actor && actor->currentProcess && actor->currentProcess->high && ShouldNextAttack(actor) && combatTarg && _strcmpi("MCO_WinOpen", a_event->tag.c_str()) == 0) {
			std::map<const std::string, float> distMap = {
				std::make_pair(NEXT_NORMAL_DISTANCE_MAX, 0.f),
				std::make_pair(NEXT_NORMAL_DISTANCE_MIN, 0.f),
				std::make_pair(NEXT_POWER_DISTANCE_MAX, 0.f),
				std::make_pair(NEXT_POWER_DISTANCE_MIN, 0.f),
			};

			if (combatTarg && actor->RequestLOS(combatTarg.get()) && GetDistanceVariable(actor, distMap) && AttackRangeCheck::CheckPathing(actor, combatTarg.get())) {
				for (auto pair : distMap) {
					logger::debug("{} value is {}", pair.first, pair.second);
				}

				auto dataHandler = DataHandler::GetSingleton();
				const float startAngle = dataHandler->settings->GetStartAngle();
				const float endAngle = dataHandler->settings->GetEndAngle();

				const float powerAttackChance = dataHandler->settings->powerAttackChance.get_data();

				auto normalAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCNormalAttack");  //SCAR_NPCNormalAttack Idle Form
				auto powerAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCPowerAttack");    //SCAR_PowerAttack Idle Form

				auto InNormalRange = AttackRangeCheck::WithinAttackRange(actor, combatTarg.get(), actor->GetReach() + distMap.at(NEXT_NORMAL_DISTANCE_MAX), distMap.at(NEXT_NORMAL_DISTANCE_MIN), startAngle, endAngle);
				auto InPowerRange = AttackRangeCheck::WithinAttackRange(actor, combatTarg.get(), actor->GetReach() + distMap.at(NEXT_POWER_DISTANCE_MAX), distMap.at(NEXT_POWER_DISTANCE_MIN), startAngle, endAngle);

				actor->SetGraphVariableFloat(NEXT_ATTACK_CHANCE, 100.f);

				if (powerAttackIdle && InPowerRange && powerAttackChance > Random::get<float>(0.f, 100.f) &&
					actor->currentProcess->PlayIdle(actor, RE::DEFAULT_OBJECT::kActionRightPowerAttack, powerAttackIdle, true, true, combatTarg.get())) {
					logger::debug("Next Combo is Power Attack!");
					AttackRangeCheck::DrawOverlay(actor, combatTarg.get(), actor->GetReach() + distMap.at(NEXT_POWER_DISTANCE_MAX), distMap.at(NEXT_POWER_DISTANCE_MIN), startAngle, endAngle);
					return _ProcessEvent(a_sink, a_event, a_eventSource);
				}

				if (normalAttackIdle && InNormalRange) {
					logger::debug("Next Combo is Normal Attack!");
					AttackRangeCheck::DrawOverlay(actor, combatTarg.get(), actor->GetReach() + distMap.at(NEXT_NORMAL_DISTANCE_MAX), distMap.at(NEXT_NORMAL_DISTANCE_MIN), startAngle, endAngle);
					actor->currentProcess->PlayIdle(actor, RE::DEFAULT_OBJECT::kActionRightAttack, normalAttackIdle, true, true, combatTarg.get());
					return _ProcessEvent(a_sink, a_event, a_eventSource);
				}
			}
		}

		return _ProcessEvent(a_sink, a_event, a_eventSource);
	}
}
