#pragma once

namespace SCAR
{
	class MainUpdateHook
	{
	public:
		static void Hook()
		{
#if ANNIVERSARY_EDITION
			static std::uint32_t baseID = 36544, offset = 0x160;  //Anniversary Edition
#else
			static std::uint32_t baseID = 35551, offset = 0x11F;  //Special Edition
		};
#endif
			REL::Relocation<uintptr_t> hook{ REL::ID(baseID) };
			auto& trampoline = SKSE::GetTrampoline();
			SKSE::AllocTrampoline(1 << 4);
			_Update = trampoline.write_call<5>(hook.address() + offset, Update);
		}

	private:
		static void Update(RE::Main* a_this, float a2);

		static inline REL::Relocation<decltype(Update)> _Update;
	};
}
