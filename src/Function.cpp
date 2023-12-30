#include "Function.h"
#include "DataHandler.h"
#include "DebugAPI/DebugAPI.h"

namespace SCAR
{
	float GetGameSettingFloat(const std::string a_name, const float a_default)
	{
		auto setting = RE::GameSettingCollection::GetSingleton()->GetSetting(a_name.c_str());
		if (setting) {
			return setting->GetFloat();
		}

		return a_default;
	}

	bool AttackRangeCheck::CheckPathing(RE::Actor* a_attacker, RE::Actor* a_target)
	{
		if (!a_attacker || !a_target || !a_attacker->Get3D() || !a_target->Get3D() || !a_target->GetActorRuntimeData().currentProcess || !a_attacker->GetActorRuntimeData().currentProcess)
			return false;

		auto GetMeleeWeaponRange = [](RE::Actor* a_actor) -> float {
			using TYPE = RE::CombatInventoryItem::TYPE;
			float result = 0.f;
			if (a_actor) {
				auto combatCtrl = a_actor->GetActorRuntimeData().combatController;
				auto combatInv = combatCtrl ? combatCtrl->inventory : nullptr;
				if (combatInv) {
					for (const auto item : combatInv->equippedItems) {
						if (item.item && item.item->GetType() == TYPE::kMelee) {
							auto range = item.item->GetMaxRange();
							if (range > result)
								result = range;
						}
					}
				}
			}

			return result;
		};

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_target->Is3rdPersonVisible() ? a_target->Get3D()->world.translate : a_target->GetPosition();
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiMatrix3 invMatrix = matrix.Transpose();

		auto localVector = invMatrix * (targPos - attackerPos);
		auto localDistance = std::sqrtf(localVector.x * localVector.x + localVector.y * localVector.y);
		if (localDistance <= GetMeleeWeaponRange(a_attacker) + a_target->GetBoundRadius())
			return true;

		return a_attacker->CanNavigateToPosition(a_attacker->GetPosition(), a_target->GetPosition(), 2.0f, a_attacker->GetBoundRadius());
	}

	bool AttackRangeCheck::WithinAttackRange(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startRadian, float a_endRadian)
	{
		if (!a_attacker || !a_targ || !a_attacker->Get3D() || !a_targ->Get3D())
			return false;

		max_distance += a_targ->GetBoundRadius();
		if (min_distance > 0.f)
			min_distance += a_targ->GetBoundRadius() + a_attacker->GetBoundRadius();

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_targ->Is3rdPersonVisible() ? a_targ->Get3D()->world.translate : a_targ->GetPosition();
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiMatrix3 invMatrix = matrix.Transpose();

		auto GetAabbTopBottomHeight = [](RE::Actor* a_actor, float& TopHeight, float& BottomHeight) -> bool {
			if (a_actor->Is3rdPersonVisible()) {
				ObjectBound bound;
				if (Util::GetBBCharacter(a_actor, bound)) {
					TopHeight = std::max(bound.worldBoundMin.z, bound.worldBoundMax.z);
					BottomHeight = std::min(bound.worldBoundMin.z, bound.worldBoundMax.z);
					return true;
				}
			} else {
				TopHeight = a_actor->GetPositionZ() + a_actor->GetBoundMax().z;
				BottomHeight = a_actor->GetPositionZ();
				return true;
			}

			return false;
		};

		float attackerBottomHeight, attackerTopHeight, targBottomHeight, targTopHeight;
		if (!GetAabbTopBottomHeight(a_attacker, attackerTopHeight, attackerBottomHeight) || !GetAabbTopBottomHeight(a_targ, targTopHeight, targBottomHeight))
			return false;

		auto CheckHeight = [](const float attackerBottomHeight, const float attackerTopHeight, const float targBottomHeight, const float targTopHeight) -> bool {
			return (attackerTopHeight >= targTopHeight && attackerBottomHeight <= targTopHeight) || (attackerTopHeight >= targBottomHeight && attackerBottomHeight <= targBottomHeight) || (attackerTopHeight <= targTopHeight && attackerBottomHeight >= targBottomHeight);
		};

		if (!CheckHeight(attackerBottomHeight, attackerTopHeight, targBottomHeight, targTopHeight))
			return false;

		auto localVector = invMatrix * (targPos - attackerPos);

		auto localDistance = std::sqrtf(localVector.x * localVector.x + localVector.y * localVector.y);
		auto localAngle = std::atan2f(localVector.x, localVector.y);
		if (localAngle < -std::numbers::pi)
			localAngle += 2 * std::numbers::pi;
		else if (localAngle > std::numbers::pi)
			localAngle -= 2 * std::numbers::pi;

		const bool withInAngle = a_startRadian < a_endRadian ? localAngle >= a_startRadian && localAngle <= a_endRadian : !(localAngle >= a_endRadian && localAngle <= a_startRadian);

		return localDistance <= max_distance && localDistance >= min_distance && withInAngle;
	}

	void AttackRangeCheck::DrawOverlay(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startRadian, float a_endRadian)
	{
		auto dataHandler = DataHandler::GetSingleton();
		if (!dataHandler || !dataHandler->settings || !dataHandler->settings->enableDebugOverlay.get_data() || REL::Module::IsVR() || !dataHandler->trueHUD_API)
			return;

		if (!a_attacker || !a_targ || !a_attacker->Get3D() || !a_targ->Get3D())
			return;

		static constexpr float time = 1.f;
		static constexpr uint32_t redColor = 0xFF0000FF;
		static constexpr uint32_t greenColor = 0x00FF00FF;
		static constexpr uint32_t yellowColor = 0xFFFF00FF;

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_targ->Is3rdPersonVisible() ? a_targ->Get3D()->world.translate : a_targ->GetPosition();
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiMatrix3 invMatrix = matrix.Transpose();

		auto localVector = invMatrix * (targPos - attackerPos);
		auto targCenter = attackerPos + matrix * RE::NiPoint3(localVector.x, localVector.y, 0.f);

		dataHandler->trueHUD_API->DrawLine(attackerPos, targCenter, time, yellowColor);

		dataHandler->trueHUD_API->DrawArc(attackerPos, max_distance, a_startRadian, a_endRadian, matrix, 16, time, greenColor);
		dataHandler->trueHUD_API->DrawArc(attackerPos, min_distance, a_startRadian, a_endRadian, matrix, 16, time, redColor);

		auto axis_x = RE::NiPoint3(matrix.entry[0][0], matrix.entry[0][1], matrix.entry[0][2]), axis_y = RE::NiPoint3(matrix.entry[1][0], matrix.entry[1][1], matrix.entry[1][2]);
		dataHandler->trueHUD_API->DrawCircle(targCenter, axis_x, axis_y, a_targ->GetBoundRadius(), 26, time, yellowColor);
	}

}
