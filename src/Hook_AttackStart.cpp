#include "Hook_AttackStart.h"
#include "DataHandler.h"
#include "Function.h"

namespace SCAR
{

	static inline bool PerformSCARAttack(RE::Actor* a_attacker, RE::Actor* a_targ, RE::hkbClipGenerator* a_clip, RE::CombatBehaviorContextMelee* a_context)
	{
		if (!a_context || !a_context->combatattackdatas.size())
			return false;

		//Reproduce bethesda 's stupid cooldown timer
		if ((RE::AITimer::QTimer() - a_context->finishedAttackTime.timeStamp <= GetGameSettingFloat("fCombatAttackAnimationDrivenDelayTime", 1.5f)) && a_attacker->IsPathing()) {
			return false;
		}

		if (a_clip) {
			if (a_targ && a_attacker->GetActorRuntimeData().currentProcess && !a_attacker->IsPlayerRef() && a_attacker->RequestLOS(a_targ) && AttackRangeCheck::CheckPathing(a_attacker, a_targ)) {
				DEBUG("Find SCAR Action Data in clip \"{}\" of \"{}\"", a_clip->animationName.c_str(), a_attacker->GetName());

				auto dataArr = DataHandler::GetSCARActionData(a_clip);
				std::sort(dataArr.begin(), dataArr.end(), SCARActionData::SortByWeight);
				for (auto data : dataArr) {
					if (data.PerformSCARAction(a_attacker, a_targ, a_context))
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
			return PerformSCARAttack(attacker, target, scarClip, a_context);
		}

		return _StartAttack(a_context);
	}

}
