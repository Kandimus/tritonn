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
	SECTION("Check calculating flowrate. Linear KF") {
printf("=======================================================================\n");
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA);
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
		ss.add("sikn1.line2.total.reset"            , 1);
		ss.add("sikn1.line2.factors.set.point_1.kf" , 100);
		ss.add("sikn1.line2.factors.set.point_1.hz" , 100);
		ss.add("sikn1.line2.factors.set.point_2.kf" , 500);
		ss.add("sikn1.line2.factors.set.point_2.hz" , 500);
		ss.add("sikn1.line2.factors.set.point_3.kf" , 2000);
		ss.add("sikn1.line2.factors.set.point_3.hz" , 1000);
		ss.add("sikn1.line2.factors.set.point_4.kf" , 0);
		ss.add("sikn1.line2.factors.set.point_4.hz" , 0);
		ss.add("sikn1.line2.factors.set.accept"     , 1);
		ss.set();

		mSleep(1800);

		ss.clear();
		ss.add("sikn1.line2.temperature.value");
		ss.add("sikn1.line2.pressure.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.add("sikn1.line2.total.present.mass");
		ss.add("sikn1.line2.total.present.impulse");
		ss.add("sikn1.line2.flowrate.mass.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("sikn1.line2.temperature.value"));
		REQUIRE(ss("sikn1.line2.pressure.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line2.total.present.mass"));
		REQUIRE(ss("sikn1.line2.total.present.impulse"));
		REQUIRE(ss("sikn1.line2.flowrate.mass.value"));

		CHECK  (ss("sikn1.line2.temperature.value")->getValueLREAL() == temp);
		CHECK  (ss("sikn1.line2.pressure.value")->getValueLREAL() == pres);
		CHECK  (ss("sikn1.line2.obj.stn_dens.density.value")->getValueLREAL() == dens);
		CHECK  (ss("sikn1.line2.total.present.mass")->getValueLREAL() == 2.0);
		CHECK  (ss("sikn1.line2.total.present.impulse")->getValueUDINT() == freq * 2);
		CHECK  (ss("sikn1.line2.flowrate.mass.value")->getValueLREAL() == 3600);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
