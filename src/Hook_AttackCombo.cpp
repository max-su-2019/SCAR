#include "DataHandler.h"
#include "Function.h"
#include "Hook_AttackCombo.h"

namespace SCAR
{
	EventResult AnimEventHook::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event || !a_event->holder)
			return _ProcessEvent(a_sink, a_event, a_eventSource);

		auto actor = a_event->holder->As<RE::Actor>();
		auto combatTarg = actor ? actor->currentCombatTarget.get() : nullptr;

		if (actor && actor->currentProcess && actor->currentProcess->high && ShouldNextAttack(actor) && combatTarg && _strcmpi("MCO_WinOpen", a_event->tag.c_str()) == 0 &&
			combatTarg && actor->RequestLOS(combatTarg.get()) && AttackRangeCheck::CheckPathing(actor, combatTarg.get())) {
			auto scarClip = DataHandler::GetSCARDataClip(actor);
			if (scarClip) {
				logger::debug("Find SCAR Action Data in clip \"{}\" of \"{}\"", scarClip->animationName.c_str(), actor->GetName());
				auto dataArr = DataHandler::GetSCARActionData(scarClip);
				std::sort(dataArr.begin(), dataArr.end(), SCARActionData::SortByWeight);
				for (auto data : dataArr) {
					if (data.PerformSCARAction(actor, combatTarg.get()))
						break;
				}
			}
		}

		return _ProcessEvent(a_sink, a_event, a_eventSource);
	}

}
