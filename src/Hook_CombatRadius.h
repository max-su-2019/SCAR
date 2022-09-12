#pragma once
#include "DKUtil/Hook.hpp"


namespace SCAR
{
	using namespace DKUtil::Alias;

	class CombatRadiusHook
	{
		// 1-6-353-0 @ 0x870CF0
		static inline constexpr std::uint64_t AE_FuncID = 50643;
		// 1-5-97-0 @ 0x844E40
		static inline constexpr std::uint64_t SE_FuncID = 49716;

		static inline constexpr std::ptrdiff_t RadiusL = 0x99;
		static inline constexpr std::ptrdiff_t RadiusH = 0xA0;
		static inline constexpr std::ptrdiff_t MedianL = 0x139;
		static inline constexpr std::ptrdiff_t MedianH = 0x145;

		static inline constexpr Patch RadiusPatch{
			// lahf
			"\x9F\x50"
			// mov rdx, rdi
			"\x48\x89\xFA"
			// movss xmm2, xmm6
			"\xF3\x0F\x10\xD6"
			// mov r9, rbp
			"\x49\x89\xE9"
			// mov [rsp-0x8], rbx
			"\x48\x89\x5C\x24\xF8",
			17
		};

		static inline constexpr Patch MedianPatch{
			// xor cl
			"\x30\xC9",
			19
		};

		static inline constexpr Patch Epilog{
			// sahf
			"\x58\x9E",
			2
		};
		
		// cl, rdx, xmm2, r9, rsp-0x8
		static void RecalculateCombatRadius(bool a_fullRadius, float* a_radius, float a_delta, RE::Character* a_center, RE::Character* a_actor)
		{
			static_assert(sizeof(float) == 0x4);

			if (!a_radius || !a_center || !a_actor) {
				return;
			}

			auto& inner = a_radius[0];
			auto& outer = a_radius[2];

			// recalc
			DEBUG("Set full radius: {}", a_fullRadius);
			DEBUG("Me: {} | {:X} <-> He: {} | {:X}", a_center->GetName(), a_center->GetFormID(), a_actor->GetName(), a_actor->GetFormID());
			DEBUG("InnerR: {} {:X} | OuterR {} {:X}", inner, AsAddress(&a_radius[0]), outer, AsAddress(&a_radius[2]));
			DEBUG("Delta: {}", a_delta);
		}

	public:
		static void Install()
		{
			SKSE::AllocTrampoline(static_cast<size_t>(1) << 7);

			auto Hook_SetRadius = DKUtil::Hook::AddCaveHook(
				DKUtil::Hook::IDToAbs(50643, 49716),
				{ RadiusL, RadiusH },
				FUNC_INFO(RecalculateCombatRadius),
				&RadiusPatch,
				&Epilog,
				DKUtil::Hook::CaveHookFlag::kRestoreBeforeProlog);

			Hook_SetRadius->Enable();

			auto Hook_SetMedian = DKUtil::Hook::AddCaveHook(
				DKUtil::Hook::IDToAbs(50643, 49716),
				{ MedianL, MedianH },
				FUNC_INFO(RecalculateCombatRadius),
				&MedianPatch,
				&Epilog,
				DKUtil::Hook::CaveHookFlag::kRestoreBeforeProlog);

			DKUtil::Hook::WritePatch(Hook_SetMedian->TramEntry + Hook_SetMedian->CaveSize + 0x2, &RadiusPatch);

			Hook_SetMedian->Enable();

			INFO("Hook SetCombatRadius!");
		}
	};
}