#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data/ai.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

TEST_CASE("testing masswater and volwater.", "[MassWater]")
{
	SECTION("Check calculating masswater and volwater") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA);

		ss.add("var.test_dens.value"        , 880.0);
		ss.add("sikn1.bik.io.water1.mode"  ,  static_cast<UINT>(rAI::Mode::MKEYPAD));
		ss.add("sikn1.bik.io.water1.keypad", 0.2);
		ss.add("sikn1.bik.io.temp1.mode"   ,  static_cast<UINT>(rAI::Mode::MKEYPAD));
		ss.add("sikn1.bik.io.temp1.keypad" ,  15.7);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("var.test_dens.value");
		ss.add("sikn1.obj.masswater.result.value");
		ss.add("sikn1.obj.volwater.result.value");
		ss.add("sikn1.obj.masswater.densitywater");
		ss.add("sikn1.obj.volwater.densitywater");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("var.test_dens.value"));
		REQUIRE(ss("sikn1.obj.masswater.result.value"));
		REQUIRE(ss("sikn1.obj.volwater.result.value"));
		REQUIRE(ss("sikn1.obj.masswater.densitywater"));
		REQUIRE(ss("sikn1.obj.volwater.densitywater"));
		CHECK  (ss("var.test_dens.value")->getValueLREAL() == 880.0);
		CHECK  (ss("sikn1.obj.masswater.result.value")->getValueLREAL() == 0.2270433954);
		CHECK  (ss("sikn1.obj.volwater.result.value")->getValueLREAL() == 0.2);
		CHECK  (ss("sikn1.obj.masswater.densitywater")->getValueLREAL() == 998.991);
		CHECK  (ss("sikn1.obj.volwater.densitywater" )->getValueLREAL() == 998.991);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
