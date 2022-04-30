#pragma once

namespace SCAR
{
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
			//logger::debug("Process Animation Event Fire!");
			if (!a_event || !a_event->holder)
				return _ProcessEvent(a_sink, a_event, a_eventSource);

			auto actor = a_event->holder->As<RE::Actor>();
			if (actor && _strcmpi("MCO_WinOpen", a_event->tag.c_str()) == 0) {
				float nextAttackDistance = 0.f, nextPowerAttackDistance = 0.f;
				if (actor->GetGraphVariableFloat("SCAR_nextattackdistancemax", nextAttackDistance) && actor->GetGraphVariableFloat("SCAR_nextpowerattackdistancemax", nextPowerAttackDistance)) {
					auto combatTarg = actor->currentCombatTarget ? actor->currentCombatTarget.get() : nullptr;
					if (!combatTarg || !actor->HasLOS(combatTarg.get()))
						return _ProcessEvent(a_sink, a_event, a_eventSource);

					auto NormalAttackDistance = actor->GetReach() + nextAttackDistance;
					auto PowerAttackDistance = actor->GetReach() + nextPowerAttackDistance;
					auto distance = actor->GetPosition().GetDistance(combatTarg->GetPosition());

					if (NormalAttackDistance * 1.1f >= distance) {
						actor->NotifyAnimationGraph("attackStart");
						logger::debug("Fire Next Normal Attack!, Distance {}", distance);
					} else if (PowerAttackDistance >= distance && PowerAttackDistance >= NormalAttackDistance * 3.0f) {
						actor->NotifyAnimationGraph("attackPowerStartForward");
						logger::debug("Fire Next Power Attack!, Distance {}", distance);
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
