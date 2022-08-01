#include "DataHandler.h"
#include "DebugAPI/DebugAPI.h"
#include "Hook_AttackCombo.h"
#include "Hook_AttackStart.h"
#include "Hook_MainUpdate.h"
#include "LoadGame.h"

namespace SCAR
{
	void EventCallback(SKSE::MessagingInterface::Message* msg)
	{
		if (msg->type == SKSE::MessagingInterface::kPostLoad) {
			// For First Attack
			SCAR::RecheckAttackDistancHook::Install();
			SCAR::AttackAngleHook::InstallHook();
			SCAR::AttackActionHook::InstallHook();

			// For Combo
			SCAR::AnimEventHook::InstallHook();

			//DebugOverlayMenu
			DebugOverlayMenu::Register();
			SCAR::MainUpdateHook::Hook();

			auto dataHandler = SCAR::DataHandler::GetSingleton();
			if (dataHandler->precisionPtr) {
				logger::info("Obtained PrecisionAPI - {:x}", (uintptr_t)dataHandler->precisionPtr);
			}

			if (dataHandler->settings->enableDebugLog.get_data()) {
				spdlog::set_level(spdlog::level::debug);
				logger::debug("Enable Debug Log!");
			}
		}
	}

}
