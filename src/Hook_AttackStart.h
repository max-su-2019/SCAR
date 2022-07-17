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
#if ANNIVERSARY_EDITION
		// 1-6-353: 0x837410 + 0x404
		static inline constexpr std::uint64_t FuncID = 49170;
		static inline constexpr std::uint64_t AIFuncID = 49171;
		static inline constexpr std::ptrdiff_t OffsetL = 0x404;
		static inline constexpr std::ptrdiff_t OffsetH = 0x409;
#else
		// 1-5-97: 0x80C020 + 0x4A6
		static inline constexpr std::uint64_t FuncID = 48139;
		static inline constexpr std::uint64_t AIFuncID = 48140;
		static inline constexpr std::ptrdiff_t OffsetL = 0x4A6;
		static inline constexpr std::ptrdiff_t OffsetH = 0x4AB;
#endif
		// al
		// r15 / r13
		// rsp+68
		// rsp+20
		static inline constexpr Patch Prolog
		{
			// call AIFunc
			"\xE8\x00\x00\x00\x00"
			// mov rcx, rax
			"\x48\x89\xC1"
			// mov rdx,
			"\x4C\x89"
#if ANNIVERSARY_EDITION
			// r15
			"\xFA"
#else
			// r13
			"\xEA"
#endif
			// mov r8, [rsp+0x68]
			"\x4C\x8B\x44\x24\x68"
			// mov r9, [rsp+0x20]
			"\x4C\x8B\x4C\x24\x20"
#if ANNIVERSARY_EDITION
			// push r13 ~ r15
			"\x41\x55\x41\x57",
#else
			// push r12 ~ r13
			"\x41\x54\x41\x55",
#endif
				25
		};

		static inline constexpr Patch Epilog
		{
#if ANNIVERSARY_EDITION
			// pop r15 ~ r13
			"\x41\x5F\x41\x5D",
#else
			// pop r13 ~ r12
			"\x41\x5D\x41\x5C",
#endif
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

	class AttackAngleHook
	{
	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			static std::uint32_t baseID = 49170, offset = 0x3F1;  //Anniversary Edition
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
		static bool GetAttackAngle(RE::Actor* a1, RE::Actor* a2, const RE::NiPoint3& a3, const RE::NiPoint3& a4, RE::BGSAttackData* a5, float a6, void* a7, bool a8);

		static inline REL::Relocation<decltype(GetAttackAngle)> _GetAttackAngle;
	};

	class AttackActionHook
	{
	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			static std::uint32_t baseID = 49170, offset = 0x435;  //Anniversary Edition
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
