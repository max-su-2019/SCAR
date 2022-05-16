#pragma once

namespace RE
{
	class TESActionData
	{
	public:
		inline static constexpr auto RTTI = RTTI_TESActionData;

		virtual ~TESActionData();  // 00

		virtual void function1() = 0;  //01
		virtual void function2() = 0;  //02
		virtual void function3() = 0;  //03
		virtual void function4() = 0;  //04
		virtual void function5() = 0;  //05

		TESObjectREFR* Subject_8;         //08
		TESObjectREFR* Target_10;         //10
		BGSAction* Action_18;             //18
		std::int32_t _unk_20;             //20
		std::int32_t _unk_24;             //24
		BSFixedString AnimationEvent_28;  //28
		std::int64_t _unk_30;             //30
		std::int32_t _unk_38;             //38
		std::int32_t _unk_3C;             //3C
		std::int64_t _unk_40;             //40
		TESIdleForm* IdleForm_48;         //48
		std::int64_t _unk_50;             //50
		std::int32_t Flags_58;            //58
		std::int32_t _unk_5C;             //5C
	};
	static_assert(sizeof(TESActionData) == 0x60);
}
