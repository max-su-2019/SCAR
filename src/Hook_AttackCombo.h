#pragma once
#include "Function.h"

namespace SCAR
{
	static constexpr char NEXT_NORMAL_DISTANCE_MAX[] = "SCAR_nextattackdistancemax",
						  NEXT_NORMAL_DISTANCE_MIN[] = "SCAR_nextattackdistancemin",
						  NEXT_POWER_DISTANCE_MAX[] = "SCAR_nextpowerattackdistancemax",
						  NEXT_POWER_DISTANCE_MIN[] = "SCAR_nextpowerattackdistancemin";

	class AnimEventHook
	{
		using EventResult = RE::BSEventNotifyControl;

	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			//Anniversary Edition
#else
			static std::uint32_t baseID = 261399, offset = 0x1;  //Special Edition
#endif
			REL::Relocation<std::uintptr_t> AnimEventSinkVtbl{ REL::ID(baseID) };
			_ProcessEvent = AnimEventSinkVtbl.write_vfunc(offset, ProcessEvent);
			logger::info("Hook Process Animation Event!");
		}

	private:
		static EventResult ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
		{
			static constexpr char POWER_ATTACK_EVENT[] = "attackPowerStartForward", NORMAL_ATTACK_EVENT[] = "attackStart";

			//logger::debug("Process Animation Event Fire!");

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

				auto heightDiff = std::abs(combatTarg->GetPositionZ() - actor->GetPositionZ());

				if (combatTarg && actor->HasLOS(combatTarg.get()) && heightDiff < actor->GetHeight() && GetDistanceVariable(actor, distMap)) {
					for (auto pair : distMap) {
						logger::debug("{} value is {}", pair.first, pair.second);
					}

					auto normalAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCNormalAttackRoot");  //SCAR Normal Attack Root Idle Form
					auto powerAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCPowerAttackRoot");    //SCAR PowerAttackRoot Idle Form

					auto currentDistance = actor->GetPosition().GetDistance(combatTarg->GetPosition());
					auto InNormalDistance = IsInDistance(currentDistance, distMap.at(NEXT_NORMAL_DISTANCE_MIN), actor->GetReach() + distMap.at(NEXT_NORMAL_DISTANCE_MAX));
					auto InPowerDistance = IsInDistance(currentDistance, distMap.at(NEXT_POWER_DISTANCE_MIN), actor->GetReach() + distMap.at(NEXT_POWER_DISTANCE_MAX));

					actor->SetGraphVariableFloat(NEXT_ATTACK_CHANCE, 100.f);
					float powerAttackChance = 30.f;

					if (powerAttackIdle && InPowerDistance && powerAttackChance > Random::get<float>(0.f, 100.f) &&
						actor->currentProcess->PlayIdle(actor, RE::DEFAULT_OBJECT::kActionRightPowerAttack, powerAttackIdle, true, true, combatTarg.get())) {
						logger::debug("Next Combo is Power Attack!");
						return _ProcessEvent(a_sink, a_event, a_eventSource);
					}

					if (normalAttackIdle && InNormalDistance) {
						logger::debug("Next Combo is Normal Attack!");
						actor->currentProcess->PlayIdle(actor, RE::DEFAULT_OBJECT::kActionRightAttack, normalAttackIdle, true, true, combatTarg.get());
						return _ProcessEvent(a_sink, a_event, a_eventSource);
					}
				}
			}

			return _ProcessEvent(a_sink, a_event, a_eventSource);
		}
		static inline REL::Relocation<decltype(ProcessEvent)> _ProcessEvent;
	};

	class Hook_GetAttackChance2
	{
	public:
		static void install()
		{  //Up	p	sub_14080C020+2AE	call    Character__sub_140845B30
			auto& trampoline = SKSE::GetTrampoline();
			SKSE::AllocTrampoline(1 << 4);

			REL::Relocation<uintptr_t> hook{ REL::ID(48139) };

			_getAttackChance = trampoline.write_call<5>(hook.address() + 0x2AE, getAttackChance2);
			INFO("Get Attackchance2 hook installed.");
		}

	private:
		static float getAttackChance2(RE::Actor* a1, RE::Actor* a2, RE::BGSAttackData* atkData)
		{
			using AttackFlag = RE::AttackData::AttackFlag;
			//logger::debug("{} Hook Trigger!", std::source_location::current().function_name());
			auto result = _getAttackChance(a1, a2, atkData);

			if (atkData && a1 && a2) {
				logger::debug("Attack Data Chance {},Name {}", result, atkData->event.c_str());
			}

			return result;
		}

		static inline REL::Relocation<decltype(getAttackChance2)> _getAttackChance;
	};
}
