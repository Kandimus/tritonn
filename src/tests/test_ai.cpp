#include <limits>
#include <cmath>
#include "simpletest.h"
#include "data_ai.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "io_ai_channel.h"
#include "simpletest.h"

const UDINT SleepValue = 800;

S_NEW_TEST( AnalogInput, "testing analog input. IO simulate")
{
	S_SECTION("virtual") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL test_val = 22.0;

		ss.add("io.ai_virt.present.value", test_val);
		ss.set();

		mSleep(SleepValue);

		ss.clear();
		ss.add("io.ai_virt.present.value");
		ss.get();

		S_REQUIRE(ss("io.ai_virt.present.value") != nullptr)
		LREAL aa = ss("io.ai_virt.present.value")->getValueLREAL();
		S_CHECK(S_DBL_EQ(ss("io.ai_virt.present.value")->getValueLREAL(), test_val));
	}

	S_SECTION("set simulate value") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL min_val = -10.0;

		ss.add("hardware.ai6_1.ch_01.type"          , static_cast<USINT>(rIOAIChannel::Type::mA_4_20));
		ss.add("hardware.ai6_1.ch_01.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::Const));
		ss.add("hardware.ai6_1.ch_01.simulate.value", static_cast<UINT>(rIOAIChannel::Scale_mA_4_20::Min));
		ss.add("io.ai00.scales.min"                , min_val);
		ss.add("io.ai00.scales.max"                , 100.0);
		ss.set();

		mSleep(SleepValue * 2);

		ss.clear();
		ss.add("hardware.ai6_1.ch_01.type");
		ss.add("hardware.ai6_1.ch_01.simulate.value");
		ss.add("io.ai00.present.value");
		ss.get();

		S_REQUIRE(ss("io.ai00.present.value") != nullptr)
		S_CHECK(S_DBL_EQ(ss("io.ai00.present.value")->getValueLREAL(), min_val));

		printf("hardware.ai6_1.ch_01.type = %i\n", ss("hardware.ai6_1.ch_01.type")->getValueUSINT());
		printf("hardware.ai6_1.ch_01.simulate.value = %i\n", ss("hardware.ai6_1.ch_01.simulate.value")->getValueUINT());
		printf("io.ai00.present.value = %.6f\n", ss("io.ai00.present.value")->getValueLREAL());
	}

	S_SECTION("set keypad value") {
		const LREAL testvalue = 22.0;
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("io.ai01.keypad", testvalue);
		ss.add("io.ai01.mode"  , static_cast<UINT>(rAI::Mode::MKEYPAD));
		ss.set();
		ss.clear();

		mSleep(600);

		ss.add("io.ai01.present.value");
		ss.get();

		S_REQUIRE(ss("io.ai01.present.value") != nullptr)
		S_CHECK(S_DBL_EQ(ss("io.ai01.present.value")->getValueLREAL(), testvalue));
	}

	S_SECTION("test scales & limits (hihi, hi, lo, lolo)") {
		rSnapshot set_ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		rSnapshot get_ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		set_ss.add("io.ai01.scales.min"  , -10.0);
		set_ss.add("io.ai01.scales.max"  , 100.0);
		set_ss.add("io.ai01.present.lolo", 0.0);
		set_ss.add("io.ai01.present.lo"  , 20.0);
		set_ss.add("io.ai01.present.hi"  , 40.0);
		set_ss.add("io.ai01.present.hihi", 60.0);
		set_ss.add("io.ai01.keypad"      , -11.0);
		set_ss.add("io.ai01.mode"        , static_cast<UINT>(rAI::Mode::MKEYPAD));
		set_ss.set();
		mSleep(600);

		get_ss.add("io.ai01.present.status");
		get_ss.add("io.ai01.status");
		get_ss.get();

		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_AMIN);
		S_CHECK(static_cast<rAI::Status>(get_ss("io.ai01.status")->getValueUINT()) == rAI::Status::MIN);

		// LOLO
		set_ss.clear();
		set_ss.add("io.ai01.keypad", -5.0);
		set_ss.set();
		mSleep(600);

		get_ss.get();
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_AMIN);
		S_CHECK(get_ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// LO
		set_ss.clear();
		set_ss.add("io.ai01.keypad", 5.0);
		set_ss.set();
		mSleep(600);

		get_ss.get();
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_WMIN);
		S_CHECK(get_ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// NORMAL
		set_ss.clear();
		set_ss.add("io.ai01.keypad", 30.0);
		set_ss.set();
		mSleep(600);

		get_ss.get();
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_NORMAL);
		S_CHECK(get_ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// HI
		set_ss.clear();
		set_ss.add("io.ai01.keypad", 50.0);
		set_ss.set();
		mSleep(600);

		get_ss.get();
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_WMAX);
		S_CHECK(get_ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// HIHI
		set_ss.clear();
		set_ss.add("io.ai01.keypad", 70.0);
		set_ss.set();
		mSleep(600);

		get_ss.get();
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_AMAX);
		S_CHECK(get_ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// MAX
		set_ss.clear();
		set_ss.add("io.ai01.keypad", 101.0);
		set_ss.set();
		mSleep(600);

		get_ss.get();
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->getValueUINT() == LIMIT_STATUS_AMAX);
		S_CHECK(get_ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::MAX));
	}

	S_SECTION("test limits current (hihi, lolo)") {
/*
		rSnapshot set_ss(ACCESS_MASK_ADMIN);
		rSnapshot get_ss(ACCESS_MASK_ADMIN);

		set_ss.Add("io.ai01.current.lolo", 0.0);
		set_ss.Add("io.ai01.current.hihi", 60.0);
		set_ss.Add("io.ai01.keypad"      , -11.0);
		rDataManager::Instance().Set(set_ss);
		mSleep(600);

		get_ss.Add("io.ai01.current.status");
		get_ss.Add("io.ai01.status");
		rDataManager::Instance().Get(get_ss);

		S_REQUIRE(get_ss("io.ai01.current.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.current") != nullptr);
		S_CHECK(get_ss("io.ai01.current.status")->GetValueUINT() == LIMIT_STATUS_AMIN);
		S_CHECK(get_ss("io.ai01.status")->GetValueUINT() & AI_STATUS_MIN);

		// HIHI
		set_ss.Clear();
		set_ss.Add("io.ai01.keypad", 70.0);
		rDataManager::Instance().Set(set_ss);
		mSleep(600);

		rDataManager::Instance().Get(get_ss);
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->GetValueUINT() == LIMIT_STATUS_AMAX);
		S_CHECK(get_ss("io.ai01.status")->GetValueUINT() == rAIStatus::NORMAL);
*/
	}

	S_SECTION("test error -> keypad") {
/* need module IO
		rSnapshot set_ss(ACCESS_MASK_ADMIN);
		rSnapshot get_ss(ACCESS_MASK_ADMIN);

		set_ss.Add("io.ai01.scales.min"  , -10.0);
		set_ss.Add("io.ai01.scales.max"  , 100.0);
		set_ss.Add("io.ai01.present.lolo", 0.0);
		set_ss.Add("io.ai01.present.lo"  , 20.0);
		set_ss.Add("io.ai01.present.hi"  , 40.0);
		set_ss.Add("io.ai01.present.hihi", 60.0);
		set_ss.Add("io.ai01.keypad"      , -11.0);
		set_ss.Add("io.ai01.mode"        , AI_MODE_MKEYPAD);
		rDataManager::Instance().Set(set_ss);
		mSleep(600);
*/
	}

	// Set mode
	// err -> keypad
	// err -> last good
}

S_NEW_TEST( Analog2, "testing analog signals 2222")
{
	 S_SECTION("div by 0") {
		int a = 10 / 10;
		S_CHECK(a == 2);
	 }

	 S_SECTION("Test gist") {
		S_CHECK(1 == 1);
	 }
}

/*
 * ; Тест по объекта AI. Проверка пределов
; Конфигурация - test_sikn.xml
s sikn_123.bik.io.temp1.scales.max 100
s sikn_123.bik.io.temp1.limits.hihi 80
s sikn_123.bik.io.temp1.limits.hi 60
s sikn_123.bik.io.temp1.limits.lo 40
s sikn_123.bik.io.temp1.limits.lolo 20
s sikn_123.bik.io.temp1.scales.min 0
s sikn_123.bik.io.temp1.mode 1
w 500
;
s sikn_123.bik.io.temp1.Values.keypad -10.0
w 500
d sikn_123.bik.io.temp1.limits.status
;
s sikn_123.bik.io.temp1.Values.Keypad 10.0
w 500
d sikn_123.bik.io.temp1.limits.status
;
s sikn_123.bik.io.temp1.Values.Keypad 30.0
w 500
d sikn_123.bik.io.temp1.limits.status
;
s sikn_123.bik.io.temp1.Values.Keypad 50.0
w 500
d sikn_123.bik.io.temp1.limits.status
;
s sikn_123.bik.io.temp1.Values.Keypad 70.0
w 500
d sikn_123.bik.io.temp1.limits.status
;
s sikn_123.bik.io.temp1.Values.Keypad 90.0
w 500
d sikn_123.bik.io.temp1.limits.status
;
s sikn_123.bik.io.temp1.Values.Keypad 110.0
w 500
d sikn_123.bik.io.temp1.limits.status
*/
