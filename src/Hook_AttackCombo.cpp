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

	EventResult AnimEventHook::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
	{
		if (!a_event || !a_event->holder)
			return _ProcessEvent(a_sink, a_event, a_eventSource);

		auto actor = a_event->holder->As<RE::Actor>();
		auto combatTarg = actor ? actor->currentCombatTarget.get() : nullptr;
		if (actor && actor->currentProcess && actor->currentProcess->high && combatTarg && _strcmpi("SCAR_ComboStart", a_event->tag.c_str()) == 0 &&
			ShouldNextAttack(a_event->payload.c_str()) && actor->RequestLOS(combatTarg.get()) && AttackRangeCheck::CheckPathing(actor, combatTarg.get())) {
			auto scarClip = DataHandler::GetSCARDataClip(actor);
			if (scarClip) {
				DEBUG("Find SCAR Action Data in clip \"{}\" of \"{}\"", scarClip->animationName.c_str(), actor->GetName());
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
