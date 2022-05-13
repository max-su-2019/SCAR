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
		TESObjectREFR* Target_10;         //0F
		BGSAction* Action_18;             //1F
		std::int32_t _unk_20;             //23
		std::int32_t _unk_24;             //27
		BSFixedString AnimationEvent_28;  //2B
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
