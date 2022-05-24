#pragma once
#include "DKUtil/Config.hpp"

namespace SCAR
{
	using namespace DKUtil::Alias;

	class DataHandler
	{
		struct Settings
		{
		private:
			template <class T>
			static void PrintSettingValue(const T& a_setting)
			{
				logger::info("Setting:\"{}\" is {}"sv, a_setting.get_key(), a_setting.get_data());
			}

		public:
			Settings();

			Boolean enableDebugLog{ "EnableDebugLog", "Debug" };
			Boolean enableDebugOverlay{ "EnableDebugOverlay", "Debug" };

			Double powerAttackChance{ "PowerAttackChance", "AttackChance" };

			float GetStartAngle() const { return startAngle.get_data() / 180.f * std::numbers::pi; };
			float GetEndAngle() const { return endAngle.get_data() / 180.f * std::numbers::pi; };

		private:
			Double startAngle{ "StartAngle", "AttackAngle" };
			Double endAngle{ "EndAngle", "AttackAngle" };
		};

	public:
		static DataHandler* GetSingleton()
		{
			static DataHandler singleton;
			return std::addressof(singleton);
		}

		std::unique_ptr<Settings> settings;

	private:
		DataHandler();

		~DataHandler() = default;

		DataHandler(const DataHandler&) = delete;

		DataHandler(DataHandler&&) = delete;

		DataHandler& operator=(const DataHandler&) = delete;

		DataHandler& operator=(DataHandler&&) = delete;
	};

}
