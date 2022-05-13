#include "Hook_MainUpdate.h"
#include "DebugAPI/DebugAPI.h"

namespace SCAR
{
	void MainUpdateHook::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);

		DebugAPI::Update();
	}
}
