#pragma once

namespace SCAR
{
	using EventResult = RE::BSEventNotifyControl;

	class AnimEventHook
	{
	public:
		static void InstallHook()
		{
#if ANNIVERSARY_EDITION
			static std::uint32_t baseID = 207890, offset = 0x1;  //Anniversary Edition
#else
			static std::uint32_t baseID = 261399, offset = 0x1;  //Special Edition
#endif
			REL::Relocation<std::uintptr_t> AnimEventSinkVtbl{ REL::ID(baseID) };
			_ProcessEvent = AnimEventSinkVtbl.write_vfunc(offset, ProcessEvent);
			logger::info("Hook Process Animation Event!");
		}

	private:
		static EventResult ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

		static inline REL::Relocation<decltype(ProcessEvent)> _ProcessEvent;
	};

}
