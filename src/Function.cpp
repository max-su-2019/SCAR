#include "DataHandler.h"
#include "DebugAPI/DebugAPI.h"
#include "Function.h"

namespace SCAR
{
	bool GetDistanceVariable(RE::Actor* a_actor, std::map<const std::string, float>& a_map)
	{
		for (auto& pair : a_map) {
			if (!a_actor->GetGraphVariableFloat(pair.first, pair.second))
				return false;
		}

		return true;
	};

	bool ShouldNextAttack(RE::Actor* a_actor)
	{
		float nextAttackChance = 0.f;
		if (a_actor && a_actor->GetGraphVariableFloat(NEXT_ATTACK_CHANCE, nextAttackChance)) {
			logger::debug("Next Attack Chacne is {}", nextAttackChance);
			return nextAttackChance > Random::get<float>(0.f, 100.f);
		}

		return false;
	}

	bool AttackRangeCheck::CheckPathing(RE::Actor* a_attacker, RE::Actor* a_target)
	{
		if (!a_attacker || !a_target || !a_attacker->Get3D() || !a_target->Get3D() || !a_target->currentProcess || !a_attacker->currentProcess)
			return false;

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_target->Get3D()->world.translate;
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiMatrix3 invMatrix;
		if (!GetMatrixInverse(matrix.entry, invMatrix.entry))
			return false;

		auto localVector = invMatrix * (targPos - attackerPos);
		auto localDistance = std::sqrtf(localVector.x * localVector.x + localVector.y * localVector.y);

		if (localDistance <= a_attacker->currentProcess->cachedValues->cachedRadius + a_target->currentProcess->cachedValues->cachedRadius + a_attacker->GetReach())
			return true;

		return a_attacker->CanNavigateToPosition(a_attacker->GetPosition(), a_target->GetPosition());
	}

	bool AttackRangeCheck::WithinAttackRange(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle)
	{
		if (!a_attacker || !a_targ || !a_attacker->Get3D() || !a_targ->Get3D() || !a_targ->currentProcess)
			return false;

		max_distance += a_targ->currentProcess->cachedValues->cachedRadius;
		if (min_distance > 0.f)
			min_distance += a_targ->currentProcess->cachedValues->cachedRadius;

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_targ->Get3D()->world.translate;
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiMatrix3 invMatrix;
		if (!GetMatrixInverse(matrix.entry, invMatrix.entry))
			return false;

		auto GetAabbTopBottomHeight = [](RE::Actor* a_actor, float& TopHeight, float& BottomHeight) -> bool {
			ObjectBound bound;
			if (Util::GetBBCharacter(a_actor, bound)) {
				TopHeight = std::max(bound.worldBoundMin.z, bound.worldBoundMax.z);
				BottomHeight = std::min(bound.worldBoundMin.z, bound.worldBoundMax.z);
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

		if (!a_attacker || !a_targ || !a_attacker->Get3D() || !a_targ->Get3D() || !a_targ->currentProcess)
			return;

		static constexpr int time = 1000;
		static constexpr auto redColor = glm::vec4(1.f, 0.f, 0.f, 1.f);
		static constexpr auto greenColor = glm::vec4(0.f, 1.f, 0.f, 1.f);
		static constexpr auto yellowColor = glm::vec4(1.f, 1.f, 0.f, 1.f);

		auto attackerPos = a_attacker->Get3D()->world.translate;
		auto targPos = a_targ->Get3D()->world.translate;
		auto matrix = a_attacker->Get3D()->world.rotate;

		RE::NiPoint3 Rotation;
		matrix.ToEulerAnglesXYZ(Rotation);

		RE::NiMatrix3 invMatrix;
		if (!GetMatrixInverse(matrix.entry, invMatrix.entry))
			return;

		auto localVector = invMatrix * (targPos - attackerPos);
		auto targCenter = attackerPos + matrix * RE::NiPoint3(localVector.x, localVector.y, 0.f);

		DebugOverlayMenu::Load();
		DebugOverlayMenu::ToggleVisibility(true);

		DebugAPI::DrawLineForMS(Util::NiPointToVec(attackerPos), Util::NiPointToVec(targCenter), time, yellowColor);
		DebugAPI::DrawArc(attackerPos, max_distance, a_startAngle, a_endAngle, matrix, time, greenColor);
		DebugAPI::DrawArc(attackerPos, min_distance, a_startAngle, a_endAngle, matrix, time, redColor);

		DebugAPI::DrawCircle(Util::NiPointToVec(targCenter), a_targ->currentProcess->cachedValues->cachedRadius, Util::NiPointToVec(Rotation), time, yellowColor);
	}

	//按第一行展开计算|A|
	float AttackRangeCheck::getA(float arcs[3][3], int n)
	{
		if (n == 1) {
			return arcs[0][0];
		}
		float ans = 0;
		float temp[3][3] = { 0.0 };
		int i, j, k;
		for (i = 0; i < n; i++) {
			for (j = 0; j < n - 1; j++) {
				for (k = 0; k < n - 1; k++) {
					temp[j][k] = arcs[j + 1][(k >= i) ? k + 1 : k];
				}
			}
			float t = getA(temp, n - 1);
			if (i % 2 == 0) {
				ans += arcs[0][i] * t;
			} else {
				ans -= arcs[0][i] * t;
			}
		}
		return ans;
	}

	//计算每一行每一列的每个元素所对应的余子式，组成A*
	void AttackRangeCheck::getAStart(float arcs[3][3], int n, float ans[3][3])
	{
		if (n == 1) {
			ans[0][0] = 1;
			return;
		}
		int i, j, k, t;
		float temp[3][3];
		for (i = 0; i < n; i++) {
			for (j = 0; j < n; j++) {
				for (k = 0; k < n - 1; k++) {
					for (t = 0; t < n - 1; t++) {
						temp[k][t] = arcs[k >= i ? k + 1 : k][t >= j ? t + 1 : t];
					}
				}

				ans[j][i] = getA(temp, n - 1);  //此处顺便进行了转置
				if ((i + j) % 2 == 1) {
					ans[j][i] = -ans[j][i];
				}
			}
		}
	}

	//得到给定矩阵src的逆矩阵保存到des中。
	bool AttackRangeCheck::GetMatrixInverse(float src[3][3], float des[3][3], int n)
	{
		float flag = getA(src, n);
		float t[3][3];
		if (0 == flag) {
			return false;  //如果算出矩阵的行列式为0，则不往下进行
		} else {
			getAStart(src, n, t);
			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					des[i][j] = t[i][j] / flag;
				}
			}
		}

		return true;
	}
}
