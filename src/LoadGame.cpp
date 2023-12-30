#include "LoadGame.h"
#include "DKUtil/Hook.hpp"
#include "DataHandler.h"
#include "Hook_AttackCombo.h"
#include "Hook_AttackStart.h"

namespace SCAR
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			// For First Attack
			AIAttackStartHook::InstallHook();

			// For Combos Attack
			AnimEventHook::InstallHook();

			auto dataHandler = SCAR::DataHandler::GetSingleton();
			if (dataHandler->precisionPtr) {
				INFO("Obtained PrecisionAPI - {:x}", AsAddress(dataHandler->precisionPtr));
			}

			if (dataHandler->settings->enableDebugLog.get_data()) {
				spdlog::set_level(spdlog::level::debug);
				DEBUG("Enable Debug Log!");
			}

			if (dataHandler->settings->enableDebugOverlay.get_data()) {
				//DebugOverlay
				if (!dataHandler->trueHUD_API) {
					WARN("TrueHUD API Not Found! DebugOverlay Disabled");
					return;
				}

				if (REL::Module::IsVR()) {
					WARN("DebugOverlay not supported in VR; ignoring setting");
					return;
				}

				DEBUG("Enable Debug Overlay!");
			}
		}
	}

}
