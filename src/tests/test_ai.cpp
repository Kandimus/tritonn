#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "io/ai_channel.h"
#include "data/ai.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

// Set mode

TEST_CASE("testing analog input. IO simulate", "[io_ai]")
{
	SECTION("Virtual analog. Set present value") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL test_val = 22.0;

		ss.add("io.ai_virt.present.value", test_val);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.ai_virt.present.value");
		ss.get();

		REQUIRE(ss("io.ai_virt.present.value"));
		CHECK  (ss("io.ai_virt.present.value")->getValueLREAL() == test_val);
	}

	SECTION("Set simulate IO. set ai channel = 4mA") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL min_val = -10.0;

		ss.add("hardware.ai6_1.ch_00.type"          , static_cast<USINT>(rIOAIChannel::Mode::mA_4_20));
		ss.add("hardware.ai6_1.ch_00.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::CONST));
		ss.add("hardware.ai6_1.ch_00.simulate.value", static_cast<UINT>(rIOAIChannel::Scale_mA_4_20::Min));
		ss.add("io.ai00.scales.low"                 , min_val);
		ss.add("io.ai00.scales.high"                , 100.0);
		REQUIRE(ss("hardware.ai6_1.ch_00.type"));
		REQUIRE(ss("hardware.ai6_1.ch_00.simulate.type"));
		REQUIRE(ss("hardware.ai6_1.ch_00.simulate.value"));
		REQUIRE(ss("io.ai00.scales.low"));
		REQUIRE(ss("io.ai00.scales.high"));
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.ai00.mode" , static_cast<UINT>(rAI::Mode::PHIS));
		REQUIRE(ss("io.ai00.mode"));
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("hardware.ai6_1.ch_00.type");
		ss.add("hardware.ai6_1.ch_00.simulate.value");
		ss.add("io.ai00.present.value");
		ss.add("io.ai00.mode");
		ss.get();

		REQUIRE(ss("io.ai00.present.value"));
		CHECK  (ss("io.ai00.mode")->getValueUINT() == static_cast<UINT>(rAI::Mode::PHIS));
		CHECK  (ss("io.ai00.present.value")->getValueLREAL() == min_val);
		CHECK  (ss("hardware.ai6_1.ch_00.simulate.value")->getValueUINT() == static_cast<UINT>(rIOAIChannel::Scale_mA_4_20::Min));
	}

	SECTION("Set keypad value") {
		const LREAL testvalue = 22.0;
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("io.ai00.keypad", testvalue);
		ss.add("io.ai00.mode"  , static_cast<UINT>(rAI::Mode::MKEYPAD));
		ss.set();
		ss.clear();

		mSleep(rTest::sleepValue);

		ss.add("io.ai00.present.value");
		ss.get();

		REQUIRE(ss("io.ai00.present.value"));
		CHECK  (ss("io.ai00.present.value")->getValueLREAL() == testvalue);
	}

	SECTION("Scales & limits (hihi, hi, lo, lolo)") {
		rSnapshot set_ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		rSnapshot get_ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		set_ss.add("io.ai00.scales.low"  , -10.0);
		set_ss.add("io.ai00.scales.high" , 100.0);
		set_ss.add("io.ai00.present.lolo", 0.0);
		set_ss.add("io.ai00.present.lo"  , 20.0);
		set_ss.add("io.ai00.present.hi"  , 40.0);
		set_ss.add("io.ai00.present.hihi", 60.0);
		set_ss.add("io.ai00.keypad"      , -11.0);
		set_ss.add("io.ai00.mode"        , static_cast<UINT>(rAI::Mode::MKEYPAD));
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.add("io.ai00.present.status");
		get_ss.add("io.ai00.status");
		get_ss.get();

		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::LOLO));
		CHECK  (get_ss("io.ai00.status")->getValueUINT()         == static_cast<UINT>(rAI::Status::MIN));

		// LOLO
		set_ss.clear();
		set_ss.add("io.ai00.keypad", -5.0);
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.resetAssign();
		get_ss.get();
		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::LOLO));
		CHECK  (get_ss("io.ai00.status")->getValueUINT()         == static_cast<UINT>(rAI::Status::NORMAL));

		// LO
		set_ss.clear();
		set_ss.add("io.ai00.keypad", 5.0);
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.resetAssign();
		get_ss.get();
		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::LO));
		CHECK  (get_ss("io.ai00.status")->getValueUINT()         == static_cast<UINT>(rAI::Status::NORMAL));

		// NORMAL
		set_ss.clear();
		set_ss.add("io.ai00.keypad", 30.0);
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.resetAssign();
		get_ss.get();
		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::NORMAL));
		CHECK  (get_ss("io.ai00.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// HI
		set_ss.clear();
		set_ss.add("io.ai00.keypad", 50.0);
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.resetAssign();
		get_ss.get();
		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::HI));
		CHECK  (get_ss("io.ai00.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// HIHI
		set_ss.clear();
		set_ss.add("io.ai00.keypad", 70.0);
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.resetAssign();
		get_ss.get();
		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::HIHI));
		CHECK  (get_ss("io.ai00.status")->getValueUINT() == static_cast<UINT>(rAI::Status::NORMAL));

		// MAX
		set_ss.clear();
		set_ss.add("io.ai00.keypad", 101.0);
		set_ss.set();
		mSleep(rTest::sleepValue);

		get_ss.resetAssign();
		get_ss.get();
		REQUIRE(get_ss("io.ai00.present.status"));
		REQUIRE(get_ss("io.ai00.status"));
		CHECK  (get_ss("io.ai00.present.status")->getValueUINT() == static_cast<UINT>(rLimit::Status::HIHI));
		CHECK  (get_ss("io.ai00.status")->getValueUINT()         == static_cast<UINT>(rAI::Status::MAX));
	}

	SECTION("Limits current (hihi, lolo)") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		// LOLO
		ss.add("hardware.ai6_1.ch_01.type"          , static_cast<USINT>(rIOAIChannel::Mode::mA_4_20));
		ss.add("hardware.ai6_1.ch_01.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::CONST));
		ss.add("hardware.ai6_1.ch_01.simulate.value", static_cast<UINT> (rIOAIChannel::Scale_mA_4_20::Min) + 2);
		ss.add("io.ai00.scales.low"  , 4.0);
		ss.add("io.ai00.scales.high" , 20.0);
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

		ss.add("hardware.ai6_1.ch_01.type"          , static_cast<USINT>(rIOAIChannel::Mode::mA_4_20));
		ss.add("hardware.ai6_1.ch_01.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::CONST));
		ss.add("hardware.ai6_1.ch_01.simulate.value", static_cast<UINT> (rIOAIChannel::Scale_mA_4_20::Min) + 100);
		ss.add("io.ai00.scales.low"                 , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Min));
		ss.add("io.ai00.scales.high"                , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Max));
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

	SECTION("test error -> last good") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		UINT test_val = static_cast<UINT> (rIOAIChannel::Scale_mA_4_20::Min) + 100;

		ss.add("hardware.ai6_1.ch_02.type"          , static_cast<USINT>(rIOAIChannel::Mode::mA_4_20));
		ss.add("hardware.ai6_1.ch_02.simulate.type" , static_cast<USINT>(rIOAIChannel::SimType::CONST));
		ss.add("hardware.ai6_1.ch_02.simulate.value", test_val);
		ss.add("io.ai01.scales.low"                 , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Min));
		ss.add("io.ai01.scales.high"                , static_cast<LREAL>(rIOAIChannel::Scale_mA_4_20::Max));
		ss.add("io.ai01.keypad"                     , 50.0);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("io.ai01.mode"                       , static_cast<UINT>(rAI::Mode::PHIS));
		ss.set();

		mSleep(rTest::sleepValue * 3);

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
		mSleep(rTest::sleepValue * 2);

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
		ss.add("io.ai_virt.scales.low"   , 0.0);
		ss.add("io.ai_virt.scales.hight"  , 100.0);
		ss.add("io.ai_virt.present.value", test_val);
		ss.set();
		mSleep(rTest::sleepValue);

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
		mSleep(rTest::sleepValue);

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
}
