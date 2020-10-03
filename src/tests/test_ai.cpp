#include <limits>
#include <cmath>
#include "simpletest.h"
#include "data_manager.h"
#include "simpletest.h"


S_NEW_TEST( AnalogInput, "testing analog input. IO simulate")
{
	S_SECTION("set keypad value") {
		const LREAL testvalue = 22.0;
		rSnapshot set_ss(ACCESS_MASK_ADMIN);
		rSnapshot get_ss(ACCESS_MASK_ADMIN);

		set_ss.Add("io.ai01.keypad", testvalue);
		set_ss.Add("io.ai01.mode", AI_MODE_MKEYPAD);
		rDataManager::Instance().Set(set_ss);

		mSleep(600);

		get_ss.Add("io.ai01.present.value");
		rDataManager::Instance().Get(get_ss);

		S_REQUIRE(get_ss("io.ai01.present.value") != nullptr)
		S_CHECK(S_DBL_EQ(get_ss("io.ai01.present.value")->GetValueLREAL(), testvalue));
	}

	S_SECTION("test scales & limits (hihi, hi, lo, lolo)") {
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

		get_ss.Add("io.ai01.present.status");
		get_ss.Add("io.ai01.status");
		rDataManager::Instance().Get(get_ss);

		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->GetValueUINT() == LIMIT_STATUS_AMIN);
		S_CHECK(get_ss("io.ai01.status")->GetValueUINT() & AI_STATUS_MIN);

		set_ss.Clear();
		set_ss.Add("io.ai01.keypad", -5.0);
		rDataManager::Instance().Set(set_ss);
		mSleep(600);

		rDataManager::Instance().Get(get_ss);
		S_REQUIRE(get_ss("io.ai01.present.status") != nullptr);
		S_REQUIRE(get_ss("io.ai01.status") != nullptr);
		S_CHECK(get_ss("io.ai01.present.status")->GetValueUINT() == LIMIT_STATUS_AMIN);
		S_CHECK(get_ss("io.ai01.status")->GetValueUINT() == 0);
		printf("get_ss(\"io.ai01.status\")->GetValueUINT() %u\n", get_ss("io.ai01.status")->GetValueUINT());
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
