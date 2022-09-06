#pragma once
#include "DKUtil/Hook.hpp"
#include "DataHandler.h"
#include "Function.h"

namespace SCAR
{
	using namespace DKUtil::Alias;

	class RecheckAttackDistancHook
	{
		// 1-6-353: 0x837410 + 0x404
		static inline constexpr std::uint64_t AE_FuncID = 49170;
		static inline constexpr std::uint64_t AE_AIFuncID = 49171;
		static inline constexpr std::ptrdiff_t AE_OffsetL = 0x404;
		static inline constexpr std::ptrdiff_t AE_OffsetH = 0x409;

		// 1-5-97: 0x80C020 + 0x4A6
		static inline constexpr std::uint64_t SE_FuncID = 48139;
		static inline constexpr std::uint64_t SE_AIFuncID = 48140;
		static inline constexpr std::ptrdiff_t SE_OffsetL = 0x4A6;
		static inline constexpr std::ptrdiff_t SE_OffsetH = 0x4AB;

		// al
		// r15 / r13
		// rsp+68
		// rsp+20
		static inline constexpr Patch AE_Prolog
		{
			// call AIFunc
			"\xE8\x00\x00\x00\x00"
			// mov rcx, rax
			"\x48\x89\xC1"
			// mov rdx,
			"\x4C\x89"
			// r15
			"\xFA"
			// mov r8, [rsp+0x68]
			"\x4C\x8B\x44\x24\x68"
			// mov r9, [rsp+0x20]
			"\x4C\x8B\x4C\x24\x20"
			// push r13 ~ r15
			"\x41\x55\x41\x57",
			25
		};

		static inline constexpr Patch SE_Prolog
		{
			// call AIFunc
			"\xE8\x00\x00\x00\x00"
			// mov rcx, rax
			"\x48\x89\xC1"
			// mov rdx,
			"\x4C\x89"
			// r13
			"\xEA"
			// mov r8, [rsp+0x68]
			"\x4C\x8B\x44\x24\x68"
			// mov r9, [rsp+0x20]
			"\x4C\x8B\x4C\x24\x20"
			// push r12 ~ r13
			"\x41\x54\x41\x55",
			25
		};

		static inline constexpr Patch AE_Epilog
		{
			// pop r15 ~ r13
			"\x41\x5F\x41\x5D",
			4
		};

		static inline constexpr Patch SE_Epilog{
			// pop r13 ~ r12
			"\x41\x5D\x41\x5C",
			4
		};

		static bool RecheckAttackDistance(bool a_originResult, RE::Actor* a_attacker, RE::Actor* a_target, RE::AttackData* a_attackData);

	public:
		static void Install()
		{
			SKSE::AllocTrampoline(static_cast<size_t>(1) << 7);

			auto handle = DKUtil::Hook::AddCaveHook(
				DKUtil::Hook::IDToAbs(AE_FuncID, SE_FuncID),
				DKUtil::Hook::RuntimeOffset(AE_OffsetL, AE_OffsetH, SE_OffsetL, SE_OffsetH),
				FUNC_INFO(RecheckAttackDistance),
				DKUtil::Hook::RuntimePatch(&AE_Prolog, &SE_Prolog),
				DKUtil::Hook::RuntimePatch(&AE_Epilog, &SE_Epilog));

			// recalculate displacement
			DKUtil::Hook::WriteImm(handle->TramEntry + sizeof(OpCode), static_cast<Imm32>(DKUtil::Hook::IDToAbs(AE_AIFuncID, SE_AIFuncID) - handle->TramEntry - 0x5));

			handle->Enable();
		}
	};

	class AttackAngleHook
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> AttackDistanceBase{ RELOCATION_ID(48139, 49170) };
			_GetAttackAngle = trampoline.write_call<5>(AttackDistanceBase.address() + REL::Relocate(0x493, 0x3F1), GetAttackAngle);
			INFO("Hook GetAttackAngle!");
		}

	private:
		static bool GetAttackAngle(RE::Actor* a1, RE::Actor* a2, const RE::NiPoint3& a3, const RE::NiPoint3& a4, RE::BGSAttackData* a5, float a6, void* a7, bool a8);

		static inline REL::Relocation<decltype(GetAttackAngle)> _GetAttackAngle;
	};

	class AttackActionHook
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> AttackActionBase{ RELOCATION_ID(48139, 49170) };
			_PerformAttackAction = trampoline.write_call<5>(AttackActionBase.address() + REL::Relocate(0x4D7, 0x435), PerformAttackAction);
			INFO("Hook PerformAttackAction!");
		}

	private:
		static bool PerformAttackAction(RE::TESActionData* a_actionData);

		static inline REL::Relocation<decltype(PerformAttackAction)> _PerformAttackAction;
	};
}
