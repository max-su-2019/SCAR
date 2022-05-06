#pragma once
#include "DKUtil/Hook.hpp"
#include "Function.h"
#include "TESActionData.h"

namespace SCAR
{
	using namespace DKUtil::Alias;

	static constexpr char FIRST_NORMAL_DISTANCE_MAX[] = "SCAR_firstattackdistancemax",
						  FIRST_NORMAL_DISTANCE_MIN[] = "SCAR_firstattackdistancemin",
						  FIRST_POWER_DISTANCE_MAX[] = "SCAR_firstpowerattackdistancemax",
						  FIRST_POWER_DISTANCE_MIN[] = "SCAR_firstpowerattackdistancemin";

	class RecheckAttackDistancHook
	{
		// 80C020
		static inline constexpr std::uint64_t FuncID = 48139;
		static inline constexpr std::uint64_t AIFuncID = 48140;
		static inline constexpr std::ptrdiff_t OffsetL = 0x4A6;
		static inline constexpr std::ptrdiff_t OffsetH = 0x4AB;

		// al
		// r13
		// rsp+68
		// rsp+20

		static inline constexpr Patch Prolog{
			// call 80C6F0
			"\xE8\x00\x00\x00\x00"
			// mov rcx, rax
			"\x48\x89\xC1"
			// mov rdx, r13
			"\x4C\x89\xEA"
			// mov r8, [rsp+0x68]
			"\x4C\x8B\x44\x24\x68"
			// mov r9, [rsp+0x20]
			"\x4C\x8B\x4C\x24\x20"
			// push r12 ~ r13
			"\x41\x54\x41\x55",
			25
		};

		static inline constexpr Patch Epilog{
			// pop r13 ~ r12
			"\x41\x5D\x41\x5C",
			4
		};

		static bool RecheckAttackDistance(bool a_originResult, RE::Actor* a_attacker, RE::Actor* a_target, RE::AttackData* a_attackData)
		{
			std::map<const std::string, float> distMap = {
				std::make_pair(FIRST_NORMAL_DISTANCE_MAX, 0.f),
				std::make_pair(FIRST_NORMAL_DISTANCE_MIN, 0.f),
				std::make_pair(FIRST_POWER_DISTANCE_MAX, 0.f),
				std::make_pair(FIRST_POWER_DISTANCE_MIN, 0.f),
			};

			if (a_attacker && a_target && a_attacker->GetAttackState() == RE::ATTACK_STATE_ENUM::kNone && GetDistanceVariable(a_attacker, distMap)) {
				auto currentDistance = a_attacker->GetPosition().GetDistance(a_target->GetPosition());

				if (IsInDistance(currentDistance, distMap.at(FIRST_NORMAL_DISTANCE_MIN), a_attacker->GetReach() + distMap.at(FIRST_NORMAL_DISTANCE_MAX)) ||
					IsInDistance(currentDistance, distMap.at(FIRST_POWER_DISTANCE_MIN), a_attacker->GetReach() + distMap.at(FIRST_POWER_DISTANCE_MAX))) {
					for (auto pair : distMap) {
						logger::debug("{} value is {}", pair.first, pair.second);
					}

					logger::debug("Tagre in Attack Distance,Current Distance {}", currentDistance);
					return true;
				}

				return false;
			} else
				return a_originResult;
		}

	public:
		static void Install()
		{
			SKSE::AllocTrampoline(static_cast<size_t>(1) << 7);

			auto handle = DKUtil::Hook::AddCaveHook<OffsetL, OffsetH>(DKUtil::Hook::IDToAbs(FuncID), FUNC_INFO(RecheckAttackDistance), &Prolog, &Epilog);

			// recalculate displacement
			DKUtil::Hook::WriteImm(handle->TramEntry + sizeof(OpCode), static_cast<Imm32>(DKUtil::Hook::IDToAbs(AIFuncID) - handle->TramEntry - 0x5));

			handle->Enable();
		}
	};

	class AttackActionHook
	{
	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			//Anniversary Edition
#else
			static std::uint32_t baseID = 48139, offset = 0x4D7;  //Special Edition
#endif
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> AttackActionBase{ REL::ID(baseID) };
			_PerformAttackAction = trampoline.write_call<5>(AttackActionBase.address() + offset, PerformAttackAction);
			logger::info("Hook PerformAttackAction!");
		}

	private:
		static bool PerformAttackAction(RE::TESActionData* a_actionData)
		{
			static constexpr char POWER_ATTACK_EVENT[] = "attackPowerStartForward", NORMAL_ATTACK_EVENT[] = "attackStart";

			//logger::debug("{} Hook Trigger!", std::source_location::current().function_name());

			if (a_actionData) {
				std::map<const std::string, float> distMap = {
					std::make_pair(FIRST_NORMAL_DISTANCE_MAX, 0.f),
					std::make_pair(FIRST_NORMAL_DISTANCE_MIN, 0.f),
					std::make_pair(FIRST_POWER_DISTANCE_MAX, 0.f),
					std::make_pair(FIRST_POWER_DISTANCE_MIN, 0.f),
				};

				auto actor = a_actionData->Subject_8 ? a_actionData->Subject_8->As<RE::Actor>() : nullptr;
				auto targ = actor ? actor->currentCombatTarget.get() : nullptr;
				if (actor && actor->currentProcess && !actor->IsPlayerRef() && targ && GetDistanceVariable(actor, distMap)) {
					actor->SetGraphVariableFloat(NEXT_ATTACK_CHANCE, 100.f);
					actor->SetGraphVariableInt("MCO_nextattack", 1);
					actor->SetGraphVariableInt("MCO_nextpowerattack", 1);

					auto currentDistance = actor->GetPosition().GetDistance(targ->GetPosition());
					auto InNormalDistance = IsInDistance(currentDistance, distMap.at(FIRST_NORMAL_DISTANCE_MIN), actor->GetReach() + distMap.at(FIRST_NORMAL_DISTANCE_MAX));
					auto InPowerDistance = IsInDistance(currentDistance, distMap.at(FIRST_POWER_DISTANCE_MIN), actor->GetReach() + distMap.at(FIRST_POWER_DISTANCE_MAX));

					float powerAttackChance = 30.f;

					auto normalAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCNormalAttackRoot");  //SCAR Normal Attack Root Idle Form
					auto powerAttackIdle = RE::TESForm::LookupByEditorID<RE::TESIdleForm>("SCAR_NPCPowerAttackRoot");    //SCAR_PowerAttackRoot Idle Form

					if (powerAttackIdle && InPowerDistance && powerAttackChance >= Random::get<float>(0.f, 100.f) && !actor->IsPowerAttackCoolingDown()) {
						logger::debug("Power Atatck Start! Subject: \"{}\"", actor->GetName());
						actor->StartPowerAttackCoolDown();
						return actor->currentProcess->PlayIdle(actor, RE::DEFAULT_OBJECT::kActionRightPowerAttack, powerAttackIdle, true, true, targ.get());
					} else if (normalAttackIdle && InNormalDistance) {
						logger::debug("Normal Atatck Start! Subject: \"{}\"", actor->GetName());
						return actor->currentProcess->PlayIdle(actor, RE::DEFAULT_OBJECT::kActionRightAttack, normalAttackIdle, true, true, targ.get());
					} else
						return false;
				}
			}

			return _PerformAttackAction(a_actionData);
		}

		static inline REL::Relocation<decltype(PerformAttackAction)> _PerformAttackAction;
	};

	/*
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

			//logger::debug("{} Hook Trigger!", std::source_location::current().function_name());

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
	*/
}
