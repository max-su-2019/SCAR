#pragma once
#include "DKUtil/Hook.hpp"

namespace SCAR
{
	using namespace DKUtil::Alias;

	class AttackAngleHook
	{
	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			//Anniversary Edition
#else
			static std::uint32_t baseID = 48139, offset = 0x493;  //Special Edition
#endif
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> AttackDistanceBase{ REL::ID(baseID) };
			_GetAttackAngle = trampoline.write_call<5>(AttackDistanceBase.address() + offset, GetAttackAngle);
			logger::info("Hook GetAttackAngle!");
		}

	private:
		static bool GetAttackAngle(RE::Actor* a1, RE::Actor* a2, const RE::NiPoint3& a3, const RE::NiPoint3& a4, RE::BGSAttackData* a5, float a6, void* a7, bool a8)
		{
			using AttackFlag = RE::AttackData::AttackFlag;

			//logger::debug("{} Hook Trigger!", std::source_location::current().function_name());

			auto result = _GetAttackAngle(a1, a2, a3, a4, a5, a6, a7, a8);
			if (result && a1 && a2 && a5) {
				auto distance = a1->GetPosition().GetDistance(a2->GetPosition());
				auto PowerAttackDistance = a1->GetReach() + 450.f;
				if (distance > PowerAttackDistance)
					return false;
			}

			return result;
		}

		static inline REL::Relocation<decltype(GetAttackAngle)> _GetAttackAngle;
	};

	class AttackDistanceHook2
	{
		static inline constexpr OpCode NOP = 0x90;
		static inline constexpr std::uint64_t FuncID = 48139;
		static inline constexpr std::ptrdiff_t DistanceCheckOffset = 0x4AD;

		static inline OpCode DistanceCheckNop[6]{ NOP, NOP, NOP, NOP, NOP, NOP };

	public:
		static void Install()
		{
			const auto funcAddr = DKUtil::Hook::IDToAbs(FuncID);

			// Distance Check
			DKUtil::Hook::WriteData(funcAddr + DistanceCheckOffset, &DistanceCheckNop, sizeof(DistanceCheckNop));

			INFO("DistanceCheck Hooks installed"sv);
		}
	};

	/*Returns NPC attack chance. Return 0 to deny NPC attack.*/
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
			logger::debug("{} Hook Trigger!", std::source_location::current().function_name());
			auto result = _getAttackChance(a1, a2, atkData);

			if (atkData && a1 && a2 && atkData->data.flags.any(AttackFlag::kPowerAttack)) {
				auto NormalAttackDistance = a1->GetReach() + 110.f;
				auto distance = a1->GetPosition().GetDistance(a2->GetPosition());
				return distance > NormalAttackDistance * 1.5f ? 1.0f : 0.f;
			}

			return result;
		}

		static inline REL::Relocation<decltype(getAttackChance2)> _getAttackChance;
	};

	class AnimEventHook
	{
		using EventResult = RE::BSEventNotifyControl;

	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			//Anniversary Edition
#else
			static std::uint32_t baseID = 261399, offset = 0x1;   //Special Edition
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
				if (actor->GetGraphVariableFloat("SCAR_nextattackdistance", nextAttackDistance) && actor->GetGraphVariableFloat("SCAR_nextpowerattackdistance", nextPowerAttackDistance)) {
					//logger::debug("Event Name is {}, Attack Distance is {}, Power Distance is {}", a_event->tag.c_str(), nextAttackDistance, nextPowerAttackDistance);
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

	class NotifyAnimHook
	{
	public:
		static void install()
		{  //IAnimationGraphManagerHolder__NotifyAnimationGraph_1404F12C0+3B	call    BSAnimationGraphManager__sub_140AE24A0

			REL::Relocation<uintptr_t> Vtbl{ REL::ID(261400) };

			_NotifyAnimationGraph = Vtbl.write_vfunc(0x1, NotifyAnimationGraph);

			INFO("NotifyAnimationGraph hook installed.");
		}

	private:
		static bool NotifyAnimationGraph(RE::IAnimationGraphManagerHolder* a_graphMgr, const RE::BSFixedString& a_eventName)
		{
			static constexpr char POWER_ATTACK_EVENT[] = "attackPowerStartForward", NORMAL_ATTACK_EVENT[] = "attackStart";

			logger::debug("{} Hook Trigger!", std::source_location::current().function_name());

			auto actor = a_graphMgr ? skyrim_cast<RE::Actor*>(a_graphMgr) : nullptr;
			if (actor && !actor->IsPlayerRef() && actor->GetAttackState() == RE::ATTACK_STATE_ENUM::kNone && actor->currentCombatTarget.get()) {
				const std::string eventName = a_eventName.c_str();
				if (eventName == "attackStart" || eventName.find("attackPower") != std::string::npos) {
					auto NormalAttackDistance = actor->GetReach() + 110.f;
					auto PowerAttackDistance = actor->GetReach() + 450.f;
					auto distance = actor->GetPosition().GetDistance(actor->currentCombatTarget.get()->GetPosition());

					std::string resultEvent = "";
					if (distance <= NormalAttackDistance * 1.1f)
						resultEvent = NORMAL_ATTACK_EVENT;
					else
						resultEvent = POWER_ATTACK_EVENT;

					return _NotifyAnimationGraph(a_graphMgr, resultEvent);
				}
			}

			return _NotifyAnimationGraph(a_graphMgr, a_eventName);
		}

		static inline REL::Relocation<decltype(NotifyAnimationGraph)> _NotifyAnimationGraph;
	};
}
