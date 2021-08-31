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
		LREAL freq  = 350.0;
		LREAL dens15 = 830.0;
		LREAL b15    = 0.00089124;
		LREAL y15    = 0.00076979;
		LREAL dens   = 819.8323017886;
		LREAL temp   = 30.0;
		LREAL pres   = 1.4;

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", freq);
		ss.add("var.test_dens15.value"              , dens15);
		ss.add("var.test_b15.value"                 , b15);
		ss.add("var.test_temp.value"                , temp);
		ss.add("var.test_pres.value"                , pres);
		ss.set();

		mSleep(1250);

		ss.clear();
		ss.add("sikn1.line2.temperature.value");
		ss.add("sikn1.line2.pressure.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.add("io.fi00.impulse.value");
		ss.add("sikn1.line2.flowrate.volume.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.add("sikn1.line2.flowrate.volume.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("sikn1.line2.temperature.value"));
		REQUIRE(ss("sikn1.line2.pressure.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));

		REQUIRE(ss("io.fi00.impulse.value"));
		REQUIRE(ss("sikn1.line2.flowrate.volume.value"));

		CHECK  (ss("sikn1.line2.temperature.value")->getValueLREAL() == temp);
		CHECK  (ss("sikn1.line2.pressure.value")->getValueLREAL() == pres);
		CHECK  (ss("sikn1.line2.obj.stn_dens.density.value")->getValueLREAL() == dens);

		CHECK  (ss("io.fi00.impulse.value")->getValueLREAL() == freq);
		CHECK  (ss("sikn1.line2.flowrate.volume.value")->getValueLREAL() == 0.2270433954);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
