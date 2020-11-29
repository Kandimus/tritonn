#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_di.h"
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

		ss.add("hardware.di8do8_1.ch_01.simulate.type" , static_cast<USINT>(rIODIChannel::SimType::Const));
		ss.add("hardware.di8do8_1.ch_01.simulate.value", sim_val);
		ss.add("io.di00.mode"                          , static_cast<UINT>(rDI::Mode::PHIS));
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.di00.present.value");
		ss.get();

		REQUIRE(ss("io.di00.present.value"));
		CHECK  (ss("io.di00.present.value")->getValueUSINT() == sim_val);

		sim_val = !sim_val;
		ss.clear();
		ss.add("hardware.di8do8_1.ch_01.simulate.value", sim_val);
		ss.set();

		mSleep(rTest::sleepValue);

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
/*
	SECTION("test error -> keypad") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		UINT test_val = static_cast<UINT> (rIOAIChannel::Scale_mA_4_20::Min) + 100;

		ss.add("hardware.ai6_1.ch_02.type"          , static_cast<USINT>(rIOAIChannel::Type::mA_4_20));
		ss.add("hardware.ai6_1.ch_02.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::Const));
		ss.add("hardware.ai6_1.ch_02.simulate.value", test_val);
		ss.add("io.ai01.scales.min"                 , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Min));
		ss.add("io.ai01.scales.max"                 , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Max));
		ss.add("io.ai01.keypad"                     , 50.0);
		ss.set();

		mSleep(SleepValue);

		ss.clear();
		ss.add("io.ai01.mode"                       , static_cast<UINT>(rAI::Mode::PHIS));
		ss.set();

		mSleep(SleepValue * 3);

		ss.clear();
		ss.add("io.ai01.status");
		ss.add("io.ai01.mode");
		ss.get();

		REQUIRE(ss("io.ai01.status"));
		REQUIRE(ss("io.ai01.mode"));
		CHECK  (ss("io.ai01.mode")->getValueUINT()   == static_cast<UINT>(rAI::Mode::PHIS));
		CHECK  (ss("io.ai01.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		ss.add("hardware.ai6_1.ch_02.simulate.value", 0);
		ss.set();
		mSleep(SleepValue * 2);

		ss.clear();
		ss.add("io.ai01.present.value");
		ss.add("io.ai01.mode");
		ss.get();

		REQUIRE(ss("io.ai01.present.value"));
		REQUIRE(ss("io.ai01.mode"));
		CHECK  (ss("io.ai01.present.value")->getValueUINT() == test_val);
		CHECK  (ss("io.ai01.mode")->getValueUINT()           == static_cast<UINT>(rAI::Mode::LASTGOOD));
	}


	SECTION("virtual set higher to Scale.Max, less that Scales.Min"){
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL test_val = -500.0;

		ss.add("io.ai_virt.keypad"       , 50.0);
		ss.add("io.ai_virt.scales.min"   , 0.0);
		ss.add("io.ai_virt.scales.max"   , 100.0);
		ss.add("io.ai_virt.present.value", test_val);
		ss.set();
		mSleep(SleepValue);

		ss.clear();
		ss.add("io.ai_virt.present.value");
		ss.add("io.ai_virt.mode");
		ss.add("io.ai_virt.status");
		ss.get();

		REQUIRE(ss("io.ai_virt.present.value"));
		REQUIRE(ss("io.ai_virt.mode"));
		REQUIRE(ss("io.ai_virt.status"));
		CHECK  (ss("io.ai_virt.present.value")->getValueLREAL() == test_val);
		CHECK  (ss("io.ai_virt.mode")->getValueUINT() == static_cast<UINT>(rAI::Mode::PHIS));
		CHECK  (ss("io.ai_virt.status")->getValueUINT() == static_cast<UINT>(rAI::Status::MIN));

		test_val = 500.0;
		ss.clear();
		ss.add("io.ai_virt.present.value", test_val);
		ss.set();
		mSleep(SleepValue);

		ss.add("io.ai_virt.present.value");
		ss.add("io.ai_virt.mode");
		ss.add("io.ai_virt.status");
		ss.get();

		REQUIRE(ss("io.ai_virt.present.value"));
		REQUIRE(ss("io.ai_virt.mode"));
		REQUIRE(ss("io.ai_virt.status"));
		CHECK  (ss("io.ai_virt.present.value")->getValueLREAL() == test_val);
		CHECK  (ss("io.ai_virt.mode")->getValueUINT() == static_cast<UINT>(rAI::Mode::PHIS));
		CHECK  (ss("io.ai_virt.status")->getValueUINT() == static_cast<UINT>(rAI::Status::MAX));
	}
*/
}
