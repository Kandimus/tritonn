#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "io/fi_channel.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

TEST_CASE("testing stream calc.", "[Stream]")
{
	SECTION("Check calculating flowrate") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", 350.0);
		ss.add("var.test_dens15.value"              , 880.0);
		ss.add("var.test_b15.value"                 , 880.0);
		ss.add("sikn1.line2.io.temp.present.value"  , 30);
		ss.add("sikn1.line2.io.press.present.value" , 1.4);
		ss.set();

		mSleep(rTest::sleepValue * 2);

		ss.clear();
		ss.add("var.testdens.value");
		ss.add("sikn1.obj.masswater.result.value");
		ss.add("sikn1.obj.volwater.result.value");
		ss.add("sikn1.obj.masswater.densitywater");
		ss.add("sikn1.obj.volwater.densitywater");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("var.testdens.value"));
		REQUIRE(ss("sikn1.obj.masswater.result.value"));
		REQUIRE(ss("sikn1.obj.volwater.result.value"));
		REQUIRE(ss("sikn1.obj.masswater.densitywater"));
		REQUIRE(ss("sikn1.obj.volwater.densitywater"));
		CHECK  (ss("var.testdens.value")->getValueLREAL() == 880.0);
		CHECK  (ss("sikn1.obj.masswater.result.value")->getValueLREAL() == 0.2270433954);
		CHECK  (ss("sikn1.obj.volwater.result.value")->getValueLREAL() == 0.2);
		CHECK  (ss("sikn1.obj.masswater.densitywater")->getValueLREAL() == 998.991);
		CHECK  (ss("sikn1.obj.volwater.densitywater" )->getValueLREAL() == 998.991);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
