#pragma once

namespace SCAR
{
	class AIAttackStartHook
	{
	public:
		static void InstallHook()
		{
			SKSE::AllocTrampoline(1 << 4);
			auto& trampoline = SKSE::GetTrampoline();

			REL::Relocation<std::uintptr_t> CombatBehaviorAttackBase{ REL::ID(48147) };  //1.5.97 14080D8C0
			_StartAttack = trampoline.write_call<5>(CombatBehaviorAttackBase.address() + 0x164, StartAttack);
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static bool StartAttack(RE::CombatBehaviorContextMelee* a_context);

		static inline REL::Relocation<decltype(StartAttack)> _StartAttack;
	};
}
