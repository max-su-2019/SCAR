#pragma once
#include "DKUtil/Hook.hpp"
#include "DataHandler.h"
#include "Function.h"
#include "TESActionData.h"

namespace SCAR
{
	using namespace DKUtil::Alias;

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

		static bool RecheckAttackDistance(bool a_originResult, RE::Actor* a_attacker, RE::Actor* a_target, RE::AttackData* a_attackData);

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
			//ChoseAttackData_sub_14080C020+4D7	 call  TESActionData__sub_14075FF10
			static std::uint32_t baseID = 48139, offset = 0x4D7;  //Special Edition
#endif
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> AttackActionBase{ REL::ID(baseID) };
			_PerformAttackAction = trampoline.write_call<5>(AttackActionBase.address() + offset, PerformAttackAction);
			logger::info("Hook PerformAttackAction!");
		}

	private:
		static bool PerformAttackAction(RE::TESActionData* a_actionData);

		static inline REL::Relocation<decltype(PerformAttackAction)> _PerformAttackAction;
	};
}

/*
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

*/
