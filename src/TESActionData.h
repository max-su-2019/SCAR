#pragma once

namespace RE
{
	class TESActionData
	{
	public:
		inline static constexpr auto RTTI = RTTI_TESActionData;

		virtual void function1() = 0;

		virtual void function2() = 0;

		virtual void function3() = 0;

		virtual void function4() = 0;

		virtual void function5() = 0;

		TESObjectREFR* Subject_8;
		TESObjectREFR* Target_10;
		BGSAction* Action_18;
		std::int32_t _unk_20;
		std::int32_t _unk_24;
		BSFixedString AnimationEvent_28;
		std::int64_t _unk_30;
		std::int32_t _unk_38;
		std::int32_t _unk_3C;
		std::int64_t _unk_40;
		TESIdleForm* IdleForm_48;
		std::int64_t _unk_50;
		std::int32_t Flags_58;
		std::int32_t _unk_5C;
	};
	static_assert(sizeof(TESActionData) == 0x60);
}
