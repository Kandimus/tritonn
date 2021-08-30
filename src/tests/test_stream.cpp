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
		LREAL freq = 350.0;

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", freq);
		ss.add("var.test_dens15.value"              , 880.0);
		ss.add("var.test_b15.value"                 , 880.0);
		ss.add("sikn1.line2.io.temp.present.value"  , 30);
		ss.add("sikn1.line2.io.press.present.value" , 1.4);
		ss.set();

		mSleep(1250);

		ss.clear();
		ss.add("io.fi00.impulse.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.add("sikn1.line2.flowrate.volume.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("io.fi00.impulse.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line2.flowrate.volume.value"));

		CHECK  (ss("io.fi00.impulse.value")->getValueLREAL() == freq);
		CHECK  (ss("sikn1.line2.obj.stn_dens.density.value")->getValueLREAL() == 0.2270433954);
		CHECK  (ss("sikn1.line2.flowrate.volume.value")->getValueLREAL() == 0.2270433954);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
