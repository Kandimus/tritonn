#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "data/average.h"

// Set mode

TEST_CASE("testing Average", "[Average]")
{
	SECTION("set 2 value, get summ") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA);
		LREAL val1 = 20.00;
		LREAL val2 = 4.69;
		LREAL test = 12.345;

		ss.add("sikn1.bik.io.temp1.present.value", val1);
		ss.add("sikn1.bik.io.temp2.present.value", val2);
		REQUIRE(ss("sikn1.bik.io.temp1.present.value"));
		REQUIRE(ss("sikn1.bik.io.temp2.present.value"));
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("sikn1.obj.avr_temp.output.value");
		ss.get();

		REQUIRE(ss("sikn1.obj.avr_temp.output.value"));
		CHECK  (ss("sikn1.obj.avr_temp.output.value")->getValueLREAL() == test);
	}

	SECTION("set fault value") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA);
		LREAL freq = 1000.0;

//		ss.add("hardware.fi4_1.ch_01.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
//		ss.add("hardware.fi4_1.ch_01.simulate.value", freq);
//		ss.set();

//		mSleep(rTest::sleepValue + 1030);

//		ss.clear();
//		ss.add("io.fi00.frequency.value");
//		ss.add("io.fi00.period.value");
//		ss.get();

//		REQUIRE(ss("io.fi00.frequency.value"));
//		REQUIRE(ss("io.fi00.frequency.value"));
//		CHECK  (ss("io.fi00.frequency.value")->getValueLREAL() == freq);
//		CHECK  (ss("io.fi00.period.value")->getValueLREAL() == 1000000.0 / freq);
	}
}
