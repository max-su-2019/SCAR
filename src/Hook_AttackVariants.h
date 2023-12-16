#pragma once

namespace SCAR
{
	class AnimClipActivateHook
	{
	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> Vtbl{ REL::ID(278766) };  //1.5.97 171D7A8
			func = Vtbl.write_vfunc(0x4, &Hook_Activate);
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static void Hook_Activate(RE::hkbClipGenerator* a_clip, const RE::hkbContext& a_context);

		static inline REL::Relocation<decltype(&RE::hkbClipGenerator::Activate)> func;
	};

}