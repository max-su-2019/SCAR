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

		PrintSettingValue(enableDebugLog);
		PrintSettingValue(enableDebugOverlay);
		PrintSettingValue(startAngle);
		PrintSettingValue(endAngle);
		PrintSettingValue(powerAttackChance);
	}

	DataHandler::DataHandler()
	{
		settings = std::make_unique<Settings>();
	}
}
