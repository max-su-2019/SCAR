#pragma once

namespace SCAR
{
	class AttackRangeCheck
	{
	private:
		//按第一行展开计算|A|
		static float getA(float arcs[3][3], int n);

		//计算每一行每一列的每个元素所对应的余子式，组成A*
		static void getAStart(float arcs[3][3], int n, float ans[3][3]);

		//得到给定矩阵src的逆矩阵保存到des中。
		static bool GetMatrixInverse(float src[3][3], float des[3][3], int n = 3);

	public:
		static bool CheckPathing(RE::Actor* a_attacker, RE::Actor* a_target);

		static bool WithinAttackRange(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle);

		static void DrawOverlay(RE::Actor* a_attacker, RE::Actor* a_targ, float max_distance, float min_distance, float a_startAngle, float a_endAngle);
	};
}
