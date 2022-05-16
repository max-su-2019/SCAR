#include "DataHandler.h"

namespace SCAR
{
	static auto MainConfig = RUNTIME_PROXY("SCAR.ini"sv);

	DataHandler::Settings::Settings()
	{
		MainConfig.Bind(enableDebugLog, false);
		MainConfig.Bind(enableDebugOverlay, true);
		MainConfig.Bind(startAngle, -60.f);
		MainConfig.Bind(endAngle, 60.f);
		MainConfig.Bind(powerAttackChance, 30.f);

		MainConfig.Load();

		PrintSettingValue(enableDebugLog.get_key(), enableDebugLog.get_data());
		PrintSettingValue(enableDebugOverlay.get_key(), enableDebugOverlay.get_data());
		PrintSettingValue(startAngle.get_key(), startAngle.get_data());
		PrintSettingValue(endAngle.get_key(), endAngle.get_data());
		PrintSettingValue(powerAttackChance.get_key(), powerAttackChance.get_data());
	}

	DataHandler::DataHandler()
	{
		settings = std::make_unique<Settings>();
	}
}
