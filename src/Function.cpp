#include "Function.h"
#include "DataHandler.h"
#include "DebugAPI/DebugAPI.h"

namespace SCAR
{
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

	bool AttackRangeCheck::WithinAttackRange(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle)
	{
		if (!a_attacker || !a_targ || !a_attacker->Get3D() || !a_targ->Get3D() || !a_targ->GetActorRuntimeData().currentProcess)
			return false;

		max_distance += a_targ->GetActorRuntimeData().currentProcess->cachedValues->cachedRadius;
		if (min_distance > 0.f)
			min_distance += a_targ->GetActorRuntimeData().currentProcess->cachedValues->cachedRadius;

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

		const bool withInAngle = a_startAngle < a_endAngle ? localAngle >= a_startAngle && localAngle <= a_endAngle : !(localAngle >= a_endAngle && localAngle <= a_startAngle);

		return localDistance <= max_distance && localDistance >= min_distance && withInAngle;
	}

	void AttackRangeCheck::DrawOverlay(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle)
	{
		auto dataHandler = DataHandler::GetSingleton();
		if (!dataHandler || !dataHandler->settings || !dataHandler->settings->enableDebugOverlay.get_data())
			return;

		if (!a_attacker || !a_targ || !a_attacker->Get3D() || !a_targ->Get3D() || !a_targ->GetActorRuntimeData().currentProcess)
			return;

		static constexpr int time = 1000;
		static constexpr auto redColor = glm::vec4(1.f, 0.f, 0.f, 1.f);
		static constexpr auto greenColor = glm::vec4(0.f, 1.f, 0.f, 1.f);
		static constexpr auto yellowColor = glm::vec4(1.f, 1.f, 0.f, 1.f);

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_targ->Is3rdPersonVisible() ? a_targ->Get3D()->world.translate : a_targ->GetPosition();
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiPoint3 Rotation;
		matrix.ToEulerAnglesXYZ(Rotation);

		RE::NiMatrix3 invMatrix = matrix.Transpose();

		auto localVector = invMatrix * (targPos - attackerPos);
		auto targCenter = attackerPos + matrix * RE::NiPoint3(localVector.x, localVector.y, 0.f);

		DebugOverlayMenu::Load();
		DebugOverlayMenu::ToggleVisibility(true);

		DebugAPI::DrawLineForMS(Util::NiPointToVec(attackerPos), Util::NiPointToVec(targCenter), time, yellowColor);
		DebugAPI::DrawArc(attackerPos, max_distance, a_startAngle, a_endAngle, matrix, time, greenColor);
		DebugAPI::DrawArc(attackerPos, min_distance, a_startAngle, a_endAngle, matrix, time, redColor);

		DebugAPI::DrawCircle(Util::NiPointToVec(targCenter), a_targ->GetActorRuntimeData().currentProcess->cachedValues->cachedRadius, Util::NiPointToVec(Rotation), time, yellowColor);
	}

}
