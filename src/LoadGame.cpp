#include "LoadGame.h"
#include "DataHandler.h"
#include "DebugAPI/DebugAPI.h"
#include "Hook_AttackCombo.h"
#include "Hook_AttackStart.h"
#include "Hook_MainUpdate.h"

namespace SCAR
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			// For First Attack
			AttackDistanceHook::Install();
			AttackAngleHook::InstallHook();
			AttackActionHook::InstallHook();

			// For Combo
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
				//DebugOverlayMenu
				DebugOverlayMenu::Register();
				MainUpdateHook::Hook();
				DEBUG("Enable Debug Overlay!");
			}
		}
	}

}
