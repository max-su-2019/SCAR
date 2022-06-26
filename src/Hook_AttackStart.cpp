#include "DataHandler.h"
#include "Function.h"
#include "Hook_AttackStart.h"

namespace SCAR
{
	bool RecheckAttackDistancHook::RecheckAttackDistance(bool a_originResult, RE::Actor* a_attacker, RE::Actor* a_target, RE::AttackData*)
	{
		return a_attacker && a_target && DataHandler::GetSCARDataClip(a_attacker) ? true : a_originResult;
	}

	bool AttackAngleHook::GetAttackAngle(RE::Actor* a_attacker, RE::Actor* a_target, const RE::NiPoint3& a3, const RE::NiPoint3& a4, RE::BGSAttackData* a_attackData, float a6, void* a7, bool a8)
	{
		return a_attacker && a_target && DataHandler::GetSCARDataClip(a_attacker) ? true : _GetAttackAngle(a_attacker, a_target, a3, a4, a_attackData, a6, a7, a8);
	}

	bool AttackActionHook::PerformAttackAction(RE::TESActionData* a_actionData)
	{
		auto attacker = a_actionData && a_actionData->Subject_8 ? a_actionData->Subject_8->As<RE::Actor>() : nullptr;
		auto targ = attacker ? attacker->currentCombatTarget.get() : nullptr;
		auto scarClip = attacker ? DataHandler::GetSCARDataClip(attacker) : nullptr;
		if (scarClip && targ && attacker->currentProcess && !attacker->IsPlayerRef() && attacker->RequestLOS(targ.get()) && AttackRangeCheck::CheckPathing(attacker, targ.get())) {
			logger::debug("Find SCAR Action Data in clip \"{}\" of \"{}\"", scarClip->animationName.c_str(), attacker->GetName());
			auto dataArr = DataHandler::GetSCARActionData(scarClip);
			std::sort(dataArr.begin(), dataArr.end(), SCARActionData::SortByWeight);
			for (auto data : dataArr) {
				if (data.PerformSCARAction(attacker, targ.get()))
					return true;
			}

			return false;
		}

		return _PerformAttackAction(a_actionData);
	}

}
