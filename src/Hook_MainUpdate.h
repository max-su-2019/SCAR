#pragma once

namespace SCAR
{
	class MainUpdateHook
	{
	public:
		static void Hook()
		{
			REL::Relocation<uintptr_t> hook{ RELOCATION_ID(35551, 36544) };
			auto& trampoline = SKSE::GetTrampoline();
			SKSE::AllocTrampoline(1 << 4);
			_Update = trampoline.write_call<5>(hook.address() + REL::Relocate(0x11F, 0x160), Update);
		};

	private:
		static void Update(RE::Main* a_this, float a2);

		static inline REL::Relocation<decltype(Update)> _Update;
	};
}
