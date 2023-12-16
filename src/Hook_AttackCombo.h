#pragma once

namespace SCAR
{
	using EventResult = RE::BSEventNotifyControl;

	class AnimEventHook
	{
	public:
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> AnimEventSinkVtbl{ RELOCATION_ID(261399, 207890) };
			_ProcessEvent = AnimEventSinkVtbl.write_vfunc(0x1, ProcessEvent);
			INFO("{} Done!", __FUNCTION__);
		}

	private:
		static EventResult ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event, RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource);

		static inline REL::Relocation<decltype(ProcessEvent)> _ProcessEvent;
	};

}
