#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_counter.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "io/fi_channel.h"

// Set mode

TEST_CASE("testing frequency input. IO simulate", "[FIInput]")
{
	SECTION("Virtual counter. Set present values") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL test_freq   = 2222.22;
		LREAL test_period = 3.123456;
		UDINT test_count  = 54321;

		ss.add("io.fi_virt.frequency.value", test_freq);
		ss.add("io.fi_virt.period.value"   , test_period);
		ss.add("io.fi_virt.impulse.value"  , test_count);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.resetAssign();
		ss.get();

		REQUIRE(ss("io.fi_virt.frequency.value"));
		REQUIRE(ss("io.fi_virt.period.value"));
		REQUIRE(ss("io.fi_virt.impulse.value"));
		CHECK  (ss("io.fi_virt.frequency.value")->getValueLREAL() == test_freq);
		CHECK  (ss("io.fi_virt.period.value")->getValueLREAL()    == test_period);
		CHECK  (ss("io.fi_virt.impulse.value")->getValueUDINT()   == test_count);
	}

	SECTION("Set simulate IO (const)") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		UDINT freq = 1000;

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", freq);
		ss.set();

		mSleep(rTest::sleepValue + 1030);

		ss.clear();
		ss.add("io.fi00.frequency.value");
		ss.add("io.fi00.period.value");
		ss.get();

		REQUIRE(ss("io.fi00.frequency.value"));
		REQUIRE(ss("io.fi00.frequency.value"));
		CHECK  (ss("io.fi00.frequency.value")->getValueLREAL() == freq);
		CHECK  (ss("io.fi00.period.value")->getValueLREAL() == 1000000.0 / freq);
	}

	SECTION("Set simulate IO (const) 2") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL freq = 5432.0;

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", freq);
		ss.set();

		mSleep(rTest::sleepValue + 1030);

		ss.clear();
		ss.add("io.fi00.frequency.value");
		ss.add("io.fi00.period.value");
		ss.get();

		REQUIRE(ss("io.fi00.frequency.value"));
		REQUIRE(ss("io.fi00.frequency.value"));
		CHECK  (ss("io.fi00.frequency.value")->getValueLREAL() == freq);
		CHECK  (ss("io.fi00.period.value")->getValueLREAL() == 1000000.0 / freq);
	}
}
