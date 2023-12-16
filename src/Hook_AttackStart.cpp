#include "Hook_AttackStart.h"
#include "DataHandler.h"
#include "Function.h"

namespace SCAR
{

	static inline bool PerformSCARAction(RE::Actor* a_attacker, RE::Actor* a_targ, RE::hkbClipGenerator* a_clip)
	{
		if (a_clip) {
			if (a_targ && a_attacker->GetActorRuntimeData().currentProcess && !a_attacker->IsPlayerRef() && a_attacker->RequestLOS(a_targ) && AttackRangeCheck::CheckPathing(a_attacker, a_targ)) {
				DEBUG("Find SCAR Action Data in clip \"{}\" of \"{}\"", a_clip->animationName.c_str(), a_attacker->GetName());
				bool IsAttacking = a_attacker->IsAttacking();
				auto attackState = a_attacker->GetAttackState();
				auto attackData = a_attacker->GetActorRuntimeData().currentProcess->high->attackData;
				if (attackData) {
					attackData->data;
				}

				auto dataArr = DataHandler::GetSCARActionData(a_clip);
				std::sort(dataArr.begin(), dataArr.end(), SCARActionData::SortByWeight);
				for (auto data : dataArr) {
					if (data.PerformSCARAction(a_attacker, a_targ))
						return true;
				}
			}
		}

		return false;
	}

	bool AIAttackStartHook::StartAttack(RE::CombatBehaviorContextMelee* a_context)
	{
		RE::Actor* attacker = RE::CombatBehaviorTreeUtils::CombatBehaviorAttacker();
		RE::Actor* target = RE::CombatBehaviorTreeUtils::CombatBehaviorTarget();
		auto scarClip = attacker ? DataHandler::GetSCARDataClip(attacker) : nullptr;
		if (scarClip && target) {
			return PerformSCARAction(attacker, target, scarClip);
		}

		return _StartAttack(a_context);
	}

}
