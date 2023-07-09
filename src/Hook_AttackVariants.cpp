#include "Hook_AttackVariants.h"
#include "DataHandler.h"
#include "OpenAnimationReplacer/API/OpenAnimationReplacerAPI-Animations.h"

namespace SCAR
{
	void AnimClipActivateHook::Hook_Activate(RE::hkbClipGenerator* a_clip, const RE::hkbContext& a_context)
	{
		func(a_clip, a_context);

		using namespace OAR_API::Animations;

		auto API = GetAPI();
		if (API && a_clip) {
			auto character = a_context.character;
			if (!character) {
				return;
			}

			const RE::BShkbAnimationGraph* animGraph = SKSE::stl::adjust_pointer<RE::BShkbAnimationGraph>(character, -0xC0);
			if (!animGraph)
				return;

			RE::Actor* actor = animGraph->holder;
			if (actor && DataHandler::IsSCARVariantClip(a_clip)) {
				auto replacementInfo = API->GetCurrentReplacementAnimationInfo(a_clip);
				if (replacementInfo.variantFilename.empty()) {
					return;
				}

				auto variantsId = DataHandler::GetSCARAttackVariants(replacementInfo.variantFilename.c_str());
				if (variantsId >= 0) {
					if (actor->SetGraphVariableInt("SCAR_AttackVariants", variantsId))
						DEBUG("Set {}-{} Attack Variants ID to {}", actor->GetName(), actor->GetFormID(), variantsId);
				}
			}
		}
	}

}