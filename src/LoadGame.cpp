#include "LoadGame.h"
#include "DataHandler.h"
#include "DebugAPI/DebugAPI.h"
#include "Hook_AttackCombo.h"
#include "Hook_AttackStart.h"
#include "Hook_AttackVariants.h"
#include "Hook_MainUpdate.h"
#include "DKUtil/Hook.hpp"

namespace SCAR
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			// For First Attack
			AIAttackStartHook::InstallHook();

			// For Combos Attack
			AnimEventHook::InstallHook();

			//For OAR Attack Variants
			AnimClipActivateHook::InstallHook();

			auto dataHandler = SCAR::DataHandler::GetSingleton();
			if (dataHandler->precisionPtr) {
				INFO("Obtained PrecisionAPI - {:x}", AsAddress(dataHandler->precisionPtr));
			}

			if (dataHandler->settings->enableDebugLog.get_data()) {
				spdlog::set_level(spdlog::level::debug);
				DEBUG("Enable Debug Log!");
			}

			if (dataHandler->settings->enableDebugOverlay.get_data()) {
				//DebugOverlayMenu
				if (REL::Module::IsVR()) {
					WARN("DebugOverlay not supported in VR; ignoring setting");
					return;
				}
				DebugOverlayMenu::Register();
				MainUpdateHook::Hook();
				DEBUG("Enable Debug Overlay!");
			}
		}
	}

}
