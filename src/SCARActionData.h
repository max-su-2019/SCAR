#pragma once
#include "nlohmann/json.hpp"

namespace SCAR
{
	using json = nlohmann::json;

	using DefaultObject = RE::BGSDefaultObjectManager::DefaultObject;
	using ATTACK_TYPE = RE::CombatBehaviorContextMelee::ATTACK_TYPE;

	struct SCARActionData
	{
	private:
		float weight = 0.f;

		std::string IdleAnimationEditorID = "";

		std::string attackDataName = "";

		float minDistance = 0.f;

		float maxDistance = 150.f;

		float startAngle = -60.f;

		float endAngle = 60.f;

		float chance = 100.f;

		std::string actionType = "RA";

		std::int32_t variantID = 0;

		float triggerStartTime = 0.f;

		float triggerEndTime = std::numeric_limits<float>::max();

		std::optional<float> weaponLength;

		std::optional<float> coolDownTime;

		std::optional<std::string> coolDownAlias;

		std::optional<std::string> conditionsAlias;

	public:
		friend void from_json(const json& j, SCARActionData& a_data);
		friend class DataHandler;

		bool PerformSCARAction(RE::Actor* a_attacker, RE::Actor* a_target, RE::CombatBehaviorContextMelee* a_context, RE::hkbClipGenerator* a_clip);

		static bool SortByWeight(SCARActionData a_data1, SCARActionData a_data2);

	private:
		const DefaultObject GetActionObject() const;
		const ATTACK_TYPE GetAttackType() const;
		bool PerformSpecialIdle(RE::Actor* a_attacker, RE::Actor* a_target, RE::BGSAction* a_action, RE::TESIdleForm* a_Idle);

		_NODISCARD const float GetStartRadian() const { return startAngle / 180.f * std::numbers::pi; };
		_NODISCARD const float GetEndRadian() const { return endAngle / 180.f * std::numbers::pi; };

		_NODISCARD const bool IsLeftAttack() const;
		_NODISCARD const bool IsBashAttack() const;
		_NODISCARD float GetWeaponReach(RE::Actor* a_attacker) const;

		_NODISCARD const bool IsInAttackCoolDown(RE::CombatBehaviorContextMelee::CombatAttackData* a_coolDownData) const;
		void UpdateAttackCoolDown(RE::CombatBehaviorContextMelee::CombatAttackData* a_coolDownData);
	};

	void from_json(const json& j, SCARActionData& a_data);
}
