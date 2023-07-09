#include "Hook_AttackReset.h"
#include "DataHandler.h"
#include "OpenAnimationReplacer/API/OpenAnimationReplacerAPI-Animations.h"

namespace SCAR
{

	static void ClearOarRandomCache(RE::hkbClipGenerator* a_clip, const RE::hkbContext& a_context, const RE::hkbEvent a_event)
	{
		using namespace OAR_API::Animations;

		auto API = GetAPI();
		if (API && a_clip && a_clip->localTime > 0.f && a_context.behavior && a_context.behavior->data) {
			auto character = a_context.character;
			if (!character) {
				return;
			}

			const RE::BShkbAnimationGraph* animGraph = SKSE::stl::adjust_pointer<RE::BShkbAnimationGraph>(character, -0xC0);
			if (!animGraph || _strcmpi("FirstPerson", animGraph->projectName.c_str()) == 0)
				return;

			RE::Actor* actor = animGraph->holder;
			auto stringData = a_context.behavior->data->stringData ? a_context.behavior->data->stringData.get() : nullptr;
			if (actor && actor->IsAttacking() && stringData && !stringData->eventNames.empty() && stringData->eventNames.size() > a_event.id) {
				auto eventName = stringData->eventNames[a_event.id];
				if (_strcmpi("attackStop", eventName.c_str()) == 0) {
					API->ClearRandomFloats(a_clip);
				}
			}
		}
	}

	void AnimCliphEventHook::Hook_HandleEvent(RE::hkbClipGenerator* a_clip, const RE::hkbContext& a_context, const RE::hkbEvent a_event)
	{
		ClearOarRandomCache(a_clip, a_context, a_event);
		return func(a_clip, a_context, a_event);
	}
}