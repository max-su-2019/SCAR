#include "Hook_AttackCombo.h"
#include "DataHandler.h"
#include "Function.h"

namespace SCAR
{
	static bool ShouldNextAttack(const std::string a_payload)
	{
		if (a_payload.empty())
			return true;

		return std::atoi(a_payload.c_str()) >= Random::get<float>(0.f, 100.f);
	}

	static inline bool PerformSCARAttack(RE::Actor* a_attacker, RE::Actor* a_targ, RE::hkbClipGenerator* a_clip, RE::CombatBehaviorContextMelee* a_context)
	{
		if (a_clip) {
			if (a_targ && a_attacker->GetActorRuntimeData().currentProcess && !a_attacker->IsPlayerRef() && a_attacker->RequestLOS(a_targ) && AttackRangeCheck::CheckPathing(a_attacker, a_targ)) {
				DEBUG("Find SCAR Action Data in clip \"{}\" of \"{}\"", a_clip->animationName.c_str(), a_attacker->GetName());

				auto dataArr = DataHandler::GetSCARActionData(a_clip);
				std::sort(dataArr.begin(), dataArr.end(), SCARActionData::SortByWeight);
				for (auto data : dataArr) {
					if (data.PerformSCARAction(a_attacker, a_targ, a_context, a_clip))
						return true;
				}
			}
		}

		return false;
	}

	EventResult AnimEventHook::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event || !a_event->holder)
			return _ProcessEvent(a_sink, a_event, a_eventSource);

		auto actor = a_event->holder->As<RE::Actor>();
		auto combatTarg = actor ? actor->GetActorRuntimeData().currentCombatTarget.get() : nullptr;
		if (actor && actor->GetActorRuntimeData().currentProcess && actor->GetActorRuntimeData().currentProcess->high && combatTarg && _strcmpi("SCAR_ComboStart", a_event->tag.c_str()) == 0 &&
			actor->IsAttacking() && ShouldNextAttack(a_event->payload.c_str())) {
			auto scarClip = DataHandler::GetSCARDataClip(actor);
			auto combatbehaviorCtrl = actor->GetActorRuntimeData().combatController ? actor->GetActorRuntimeData().combatController->behaviorController : nullptr;
			if (scarClip && combatbehaviorCtrl) {
				for (const auto thread : combatbehaviorCtrl->active_threads) {
					if (thread && thread->active_node) {
						const auto nodeName = thread->active_node->GetName();
						if (_strcmpi(nodeName, "attack") == 0) {
							auto context = thread->GetCurrentContext<RE::CombatBehaviorContextMelee>();
							if (context) {
								PerformSCARAttack(actor, combatTarg.get(), scarClip, context);
								break;
							}
						}
					}
				}
			}
		}

		return _ProcessEvent(a_sink, a_event, a_eventSource);
	}

}
