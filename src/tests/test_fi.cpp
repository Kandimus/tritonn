#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_ai.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "io/fi_channel.h"

// Set mode

TEST_CASE("testing frequency input. IO simulate", "[FIInput]")
{
	SECTION("Virtual counter. Set present values") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL test_freq = 2222.22;
		LREAL test_period = 3.123456;
		UDINT test_count  = 54321;

		ss.add("io.fi_virt.present.value", test_freq);
		ss.add("io.fi_virt.present.value", test_period);
		ss.add("io.fi_virt.present.value", test_count);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.fi_virt.present.value");
		ss.get();

		REQUIRE(ss("io.fi_virt.present.value"));
		CHECK  (ss("io.fi_virt.present.value")->getValueLREAL() == test_val);
	}

	SECTION("Set simulate IO (const)") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL min_val = -10.0;

		ss.add("hardware.fi4_1.ch_01.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::Const));
		ss.add("hardware.fi4_1.ch_01.simulate.value", 10000);
		ss.set();

		mSleep(rTest::sleepValue + 1000);

		ss.clear();
		ss.add("hardware.fi6_1.ch_01.simulate.value");
		ss.add("io.fi00.frequency.value");
		ss.get();

		REQUIRE(ss("io.ai00.present.value"));
		CHECK  (ss("io.ai00.mode")->getValueUINT() == static_cast<UINT>(rAI::Mode::PHIS));
		CHECK  (ss("io.ai00.present.value")->getValueLREAL() == min_val);
	}

	SECTION("Limits current (hihi, lolo)") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		// LOLO
		ss.add("hardware.ai6_1.ch_01.type"          , static_cast<USINT>(rIOAIChannel::Type::mA_4_20));
		ss.add("hardware.ai6_1.ch_01.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::Const));
		ss.add("hardware.ai6_1.ch_01.simulate.value", static_cast<UINT> (rIOAIChannel::Scale_mA_4_20::Min) + 2);
		ss.add("io.ai00.scales.min"  , 4.0);
		ss.add("io.ai00.scales.max"  , 20.0);
		ss.add("io.ai00.current.lolo", 8.0);
		ss.add("io.ai00.current.hihi", 16.0);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.ai00.current.status");
		ss.get();

		REQUIRE(ss("io.ai00.current.status"));
		CHECK  (ss("io.ai00.current.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::LOLO));

		// HIHI
		ss.clear();
		ss.add("hardware.ai6_1.ch_01.simulate.value", static_cast<UINT>(rIOAIChannel::Scale_mA_4_20::Max) - 2);
		ss.set();

		mSleep(rTest::sleepValue * 2);

		ss.clear();
		ss.add("io.ai00.current.status");
		ss.add("io.ai00.current.value");
		ss.get();

		REQUIRE(ss("io.ai00.current.status"));
		CHECK  (ss("io.ai00.current.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::HIHI));
	}

	SECTION("test error -> keypad") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("hardware.ai6_1.ch_01.type"          , static_cast<USINT>(rIOAIChannel::Type::mA_4_20));
		ss.add("hardware.ai6_1.ch_01.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::Const));
		ss.add("hardware.ai6_1.ch_01.simulate.value", static_cast<UINT> (rIOAIChannel::Scale_mA_4_20::Min) + 100);
		ss.add("io.ai00.scales.min"                 , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Min));
		ss.add("io.ai00.scales.max"                 , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Max));
		ss.add("io.ai00.keypad"                     , 50.0);
		ss.add("io.ai00.mode"                       , 0);
		ss.set();
		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.ai00.status");
		ss.add("io.ai00.mode");
		ss.get();

		REQUIRE(ss("io.ai00.status"));
		REQUIRE(ss("io.ai00.mode"));
		CHECK  (ss("io.ai00.mode")->getValueUINT()   == static_cast<UINT>(rAI::Mode::PHIS));
		CHECK  (ss("io.ai00.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		ss.add("hardware.ai6_1.ch_01.simulate.value", 0);
		ss.set();
		mSleep(rTest::sleepValue * 2);

		ss.clear();
		ss.add("io.ai00.present.value");
		ss.add("io.ai00.keypad");
		ss.add("io.ai00.mode");
		ss.get();

		REQUIRE(ss("io.ai00.present.value"));
		REQUIRE(ss("io.ai00.keypad"));
		REQUIRE(ss("io.ai00.mode"));
		CHECK  (ss("io.ai00.present.value")->getValueLREAL() == ss("io.ai00.keypad")->getValueLREAL());
		CHECK  (ss("io.ai00.mode")->getValueUINT()           == static_cast<UINT>(rAI::Mode::AKEYPAD));
	}
}
