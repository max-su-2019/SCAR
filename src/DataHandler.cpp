#include "DataHandler.h"

namespace SCAR
{
	static auto MainConfig = RUNTIME_PROXY("SCAR.ini"sv);

	DataHandler::Settings::Settings()
	{
		MainConfig.Bind(enableDebugLog, false);
		MainConfig.Bind(enableDebugOverlay, true);

		MainConfig.Load();

		PrintSettingValue(enableDebugLog);
		PrintSettingValue(enableDebugOverlay);
	}

	DataHandler::DataHandler()
	{
		precisionPtr = reinterpret_cast<PRECISION_API::IVPrecision1*>(PRECISION_API::RequestPluginAPI());
		settings = std::make_unique<Settings>();
	}

	static inline RE::hkbClipGenerator* ToClipGenerator(RE::hkbNode* a_node)
	{
		constexpr char CLASS_NAME[] = "hkbClipGenerator";

		if (a_node && a_node->GetClassType()) {
			if (_strcmpi(a_node->GetClassType()->name, CLASS_NAME) == 0)
				return skyrim_cast<RE::hkbClipGenerator*>(a_node);
		}

		return nullptr;
	}

	std::vector<SCARActionData> DataHandler::GetSCARActionData(const RE::hkbClipGenerator* a_clip)
	{
		constexpr std::string_view prefix = "SCAR_ActionData";

		std::vector<SCARActionData> dataArr;

		auto binding = a_clip ? a_clip->binding : nullptr;
		auto animation = binding ? binding->animation : nullptr;

		if (!animation || animation->annotationTracks.empty())
			return dataArr;

		for (auto anno : animation->annotationTracks[0].annotations) {
			std::string_view text{ anno.text.c_str() };
			if (text.starts_with(prefix)) {
				try {
					auto j = json::parse(text.substr(prefix.size()));
					auto actionData = j.get<SCARActionData>();
					actionData.weight = anno.time;
					dataArr.push_back(actionData);
					DEBUG("Get a Action Data, Name : {}, Distance: {}-{}, Angle: {}-{}, Chance: {}, Type: {}, Weight {}",
						actionData.IdleAnimationEditorID, actionData.minDistance, actionData.maxDistance, actionData.startAngle, actionData.endAngle, actionData.chance, actionData.actionType, actionData.weight);
				} catch (json::exception& ex) {
					ERROR("Caught an expection when convert annoation: {}", ex.what());
					continue;
				}
			}
		}

		return dataArr;
	}

	const RE::hkbClipGenerator* DataHandler::GetSCARDataClip(RE::Actor* a_actor)
	{
		if (!a_actor)
			return nullptr;

		RE::BSAnimationGraphManagerPtr graphMgr;
		if (a_actor->GetAnimationGraphManager(graphMgr) && graphMgr) {
			auto behaviourGraph = graphMgr->graphs[0] ? graphMgr->graphs[0]->behaviorGraph : nullptr;
			auto activeNodes = behaviourGraph ? behaviourGraph->activeNodes : nullptr;
			if (activeNodes) {
				for (auto nodeInfo : *activeNodes) {
					auto nodeClone = nodeInfo.nodeClone;
					if (nodeClone && nodeClone->GetClassType()) {
						auto clipGenrator = ToClipGenerator(nodeClone);
						if (clipGenrator) {
							auto binding = clipGenrator->binding;
							auto animation = binding ? binding->animation : nullptr;
							if (animation && !animation->annotationTracks.empty()) {
								for (auto anno : animation->annotationTracks[0].annotations) {
									std::string_view text{ anno.text.c_str() };
									if (text.starts_with("SCAR_ActionData"))
										return clipGenrator;
								}
							}
						}
					}
				}
			}
		}

		return nullptr;
	}
}
