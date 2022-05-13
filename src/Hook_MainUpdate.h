#pragma once

namespace SCAR
{
	class MainUpdateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> hook{ REL::ID(35551) };

			auto& trampoline = SKSE::GetTrampoline();
			SKSE::AllocTrampoline(1 << 4);
			_Update = trampoline.write_call<5>(hook.address() + 0x11F, Update);  // SkyrimSE.exe+5AF4EF
		}

	private:
		static void Update(RE::Main* a_this, float a2);

		static inline REL::Relocation<decltype(Update)> _Update;
	};
}
