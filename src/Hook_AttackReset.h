#pragma once

namespace SCAR
{
	class AnimCliphEventHook
	{
	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> Vtbl{ REL::ID(278766) };  //1.5.97 171D7A8
			func = Vtbl.write_vfunc(0x6, &Hook_HandleEvent);
			INFO("Hook AnimClipEvent!");
		}

	private:
		static void Hook_HandleEvent(RE::hkbClipGenerator* a_clip, const RE::hkbContext& a_context, const RE::hkbEvent a_event);

		static inline REL::Relocation<decltype(&RE::hkbClipGenerator::handleEvent)> func;
	};

}