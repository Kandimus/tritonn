#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data/di.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "io/di_channel.h"

// Set mode

TEST_CASE("testing discrete input. IO simulate", "[DiscreteInput]")
{
	SECTION("Virtual discrete. Set present value") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		USINT test_val = 1;

		ss.add("io.di_virt.present.value", test_val);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.di_virt.present.value");
		ss.get();

		REQUIRE(ss("io.di_virt.present.value"));
		CHECK  (ss("io.di_virt.present.value")->getValueUSINT() == test_val);
	}

	SECTION("Set simulate IO. set 1, set 0") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		USINT sim_val = 1;

		ss.add("hardware.di8do8_1.ch_00.simulate.type" , static_cast<USINT>(rIODIChannel::SimType::CONST));
		ss.add("hardware.di8do8_1.ch_00.simulate.value", sim_val);
		ss.add("io.di00.mode"                          , static_cast<UINT>(rDI::Mode::PHIS));
		ss.set();

		mSleep(rTest::sleepValue * 2);

		ss.clear();
		ss.add("io.di00.present.value");
		ss.get();

		REQUIRE(ss("io.di00.present.value"));
		CHECK  (ss("io.di00.present.value")->getValueUSINT() == sim_val);

		sim_val = !sim_val;

		ss.clear();
		ss.add("hardware.di8do8_1.ch_00.simulate.value", sim_val);
		ss.set();

		mSleep(rTest::sleepValue * 2);

		ss.clear();
		ss.add("io.di00.present.value");
		ss.get();

		REQUIRE(ss("io.di00.present.value"));
		CHECK  (ss("io.di00.present.value")->getValueUSINT() == sim_val);
	}

	SECTION("Set keypad value") {
		USINT testvalue = 1;
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("io.di00.keypad", testvalue);
		ss.add("io.di00.mode"  , static_cast<UINT>(rDI::Mode::KEYPAD));
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.di00.present.value");
		ss.get();

		REQUIRE(ss("io.di00.present.value"));
		CHECK  (ss("io.di00.present.value")->getValueUSINT() == testvalue);

		testvalue = !testvalue;
		ss.clear();
		ss.add("io.di00.keypad", testvalue);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.di00.present.value");
		ss.get();

		REQUIRE(ss("io.di00.present.value"));
		CHECK  (ss("io.di00.present.value")->getValueUSINT() == testvalue);
	}
}
