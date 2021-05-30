#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_ai.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

TEST_CASE("testing cinematic and dynamic viscosity.", "[CinVisc]")
{
	SECTION("Check calculating cinviscosity and dynviscosity") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("var.testdens.value"      , 880.0);
		ss.add("sikn1.bik.io.visc.mode"  , static_cast<UINT>(rAI::Mode::MKEYPAD));
		ss.add("sikn1.bik.io.visc.keypad", 600);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("var.testdens.value");
		ss.add("sikn1.obj.kinvisc.result.value");
//		ss.add("sikn1.obj.dynvisc.dynviscosity.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("var.testdens.value"));
		REQUIRE(ss("sikn1.obj.kinvisc.result.value"));
//		REQUIRE(ss("sikn1.obj.dynvisc.dynviscosity.value"));
		CHECK  (ss("var.testdens.value")->getValueLREAL() == 880.0);
		CHECK  (ss("sikn1.obj.kinvisc.result.value")->getValueLREAL() == 528.0);
//		CHECK  (ss("sikn1.obj.dynvisc.dynviscosity.value")->getValueLREAL() == 600);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
