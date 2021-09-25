#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "io/fi_channel.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

TEST_CASE("testing station calc.", "[Station]")
{
	SECTION("Check calculating flowrate. All streams is enable") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA);
		LREAL dens15 = 830.0;
		LREAL b15    = 0.00089124;
		LREAL y15    = 0.00076979;

		LREAL freq1  = 300.0;
		LREAL dens1  = 823.3259981875;
		LREAL temp1  = 25.0;
		LREAL pres1  = 1.1;

		LREAL freq2  = 350.0;
		LREAL dens2  = 819.8323017886;
		LREAL temp2  = 30.0;
		LREAL pres2  = 1.4;

		ss.add("hardware.fi4_1.ch_01.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_01.simulate.value", freq1);
		ss.add("var.test_dens15.value"              , dens15);
		ss.add("var.test_b15.value"                 , b15);
		ss.add("sikn1.line1.io.temp.present.value"  , temp1);
		ss.add("sikn1.line1.io.pres.present.value"  , pres1);
		ss.add("sikn1.line1.linearization"          , 0);
		ss.add("sikn1.line1.maintenance"            , 0);
		ss.add("sikn1.line1.total.reset"            , 1);
		ss.add("sikn1.line1.factors.set.kf"         , 100);
		ss.add("sikn1.line1.factors.set.accept"     , 1);

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", freq2);
		ss.add("var.test_dens15.value"              , dens15);
		ss.add("var.test_b15.value"                 , b15);
		ss.add("var.test_temp.value"                , temp2);
		ss.add("var.test_pres.value"                , pres2);
		ss.add("sikn1.line2.total.reset"            , 1);
		ss.add("sikn1.line2.linearization"          , 1);
		ss.add("sikn1.line2.maintenance"            , 0);
		ss.add("sikn1.line2.factors.set.point_1.kf" , 100);
		ss.add("sikn1.line2.factors.set.point_1.hz" , 100);
		ss.add("sikn1.line2.factors.set.point_2.kf" , 500);
		ss.add("sikn1.line2.factors.set.point_2.hz" , 500);
		ss.add("sikn1.line2.factors.set.point_3.kf" , 2000);
		ss.add("sikn1.line2.factors.set.point_3.hz" , 1000);
		ss.add("sikn1.line2.factors.set.point_4.kf" , 0);
		ss.add("sikn1.line2.factors.set.point_4.hz" , 0);
		ss.add("sikn1.line2.factors.set.accept"     , 1);

		ss.add("var.test_sikn_temp"                 , 10);

		ss.add("sikn1.total.reset"                  , 1);

		ss.set();

		mSleep(1900);

		ss.clear();
		ss.add("sikn1.line1.temperature.value");
		ss.add("sikn1.line1.pressure.value");
		ss.add("sikn1.line1.obj.stn_dens.density.value");
		ss.add("sikn1.line1.total.present.mass");
		ss.add("sikn1.line1.total.present.impulse");
		ss.add("sikn1.line1.flowrate.mass.value");
		ss.add("sikn1.line1.obj.stn_dens.density.value");

		ss.add("sikn1.line2.temperature.value");
		ss.add("sikn1.line2.pressure.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.add("sikn1.line2.total.present.mass");
		ss.add("sikn1.line2.total.present.impulse");
		ss.add("sikn1.line2.flowrate.mass.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");

		ss.add("sikn1.temperature.value");
		ss.add("sikn1.pressure.value");
		ss.add("sikn1.density.value");
		ss.add("sikn1.total.present.mass");
		ss.add("sikn1.total.present.impulse");
		ss.add("sikn1.flowrate.mass.value");
		ss.add("var.test_sikn_temp.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("sikn1.line1.temperature.value"));
		REQUIRE(ss("sikn1.line1.pressure.value"));
		REQUIRE(ss("sikn1.line1.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line1.total.present.mass"));
		REQUIRE(ss("sikn1.line1.total.present.impulse"));
		REQUIRE(ss("sikn1.line1.flowrate.mass.value"));

		REQUIRE(ss("sikn1.line2.temperature.value"));
		REQUIRE(ss("sikn1.line2.pressure.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line2.total.present.mass"));
		REQUIRE(ss("sikn1.line2.total.present.impulse"));
		REQUIRE(ss("sikn1.line2.flowrate.mass.value"));

		REQUIRE(ss("sikn1.temperature.value"));
		REQUIRE(ss("sikn1.pressure.value"));
		REQUIRE(ss("sikn1.density.value"));
		REQUIRE(ss("sikn1.total.present.mass"));
		REQUIRE(ss("sikn1.flowrate.mass.value"));
		REQUIRE(ss("var.test_sikn_temp.value"));

		CHECK  (ss("sikn1.line1.temperature.value")->getValueLREAL() == temp1);
		CHECK  (ss("sikn1.line1.pressure.value")->getValueLREAL() == pres1);
		CHECK  (ss("sikn1.line1.obj.stn_dens.density.value")->getValueLREAL() == dens1);
		CHECK  (ss("sikn1.line1.total.present.mass")->getValueLREAL() == 6.0);
		CHECK  (ss("sikn1.line1.total.present.impulse")->getValueUDINT() == freq1 * 2);
		CHECK  (ss("sikn1.line1.flowrate.mass.value")->getValueLREAL() == 10800);

		CHECK  (ss("sikn1.line2.temperature.value")->getValueLREAL() == temp2);
		CHECK  (ss("sikn1.line2.pressure.value")->getValueLREAL() == pres2);
		CHECK  (ss("sikn1.line2.obj.stn_dens.density.value")->getValueLREAL() == dens2);
		CHECK  (ss("sikn1.line2.total.present.mass")->getValueLREAL() == 2.0);
		CHECK  (ss("sikn1.line2.total.present.impulse")->getValueUDINT() == freq2 * 2);
		CHECK  (ss("sikn1.line2.flowrate.mass.value")->getValueLREAL() == 3600);

		CHECK  (ss("sikn1.temperature.value")->getValueLREAL() == ss("var.test_sikn_temp.value")->getValueLREAL());
		CHECK  (ss("sikn1.pressure.value")->getValueLREAL() == 1.175);
		CHECK  (ss("sikn1.density.value")->getValueLREAL() == 822.4525740878);
		CHECK  (ss("sikn1.total.present.mass")->getValueLREAL() == 6.0 + 2.0);
		CHECK  (ss("sikn1.flowrate.mass.value")->getValueLREAL() == 10800 + 3600);

		Catch::Epsilon::instance().setDouble(epsilon);
	}

	SECTION("Check calculating flowrate. One stream is enable") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_SA);
		LREAL dens15 = 830.0;
		LREAL b15    = 0.00089124;
		LREAL y15    = 0.00076979;

		LREAL freq1  = 300.0;
		LREAL dens1  = 823.3259981875;
		LREAL temp1  = 25.0;
		LREAL pres1  = 1.1;

		LREAL freq2  = 350.0;
		LREAL dens2  = 819.8323017886;
		LREAL temp2  = 30.0;
		LREAL pres2  = 1.4;

		ss.add("hardware.fi4_1.ch_01.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_01.simulate.value", freq1);
		ss.add("var.test_dens15.value"              , dens15);
		ss.add("var.test_b15.value"                 , b15);
		ss.add("sikn1.line1.io.temp.present.value"  , temp1);
		ss.add("sikn1.line1.io.pres.present.value"  , pres1);
		ss.add("sikn1.line1.linearization"          , 0);
		ss.add("sikn1.line1.maintenance"            , 0);
		ss.add("sikn1.line1.total.reset"            , 1);
		ss.add("sikn1.line1.factors.set.kf"         , 100);
		ss.add("sikn1.line1.factors.set.accept"     , 1);

		ss.add("hardware.fi4_1.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_1.ch_00.simulate.value", freq2);
		ss.add("var.test_dens15.value"              , dens15);
		ss.add("var.test_b15.value"                 , b15);
		ss.add("var.test_temp.value"                , temp2);
		ss.add("var.test_pres.value"                , pres2);
		ss.add("sikn1.line2.total.reset"            , 1);
		ss.add("sikn1.line2.linearization"          , 1);
		ss.add("sikn1.line2.maintenance"            , 1);
		ss.add("sikn1.line2.factors.set.point_1.kf" , 100);
		ss.add("sikn1.line2.factors.set.point_1.hz" , 100);
		ss.add("sikn1.line2.factors.set.point_2.kf" , 500);
		ss.add("sikn1.line2.factors.set.point_2.hz" , 500);
		ss.add("sikn1.line2.factors.set.point_3.kf" , 2000);
		ss.add("sikn1.line2.factors.set.point_3.hz" , 1000);
		ss.add("sikn1.line2.factors.set.point_4.kf" , 0);
		ss.add("sikn1.line2.factors.set.point_4.hz" , 0);
		ss.add("sikn1.line2.factors.set.accept"     , 1);

		ss.add("var.test_sikn_temp"                 , 10);

		ss.add("sikn1.total.reset"                  , 1);

		ss.set();

		mSleep(1900);

		ss.clear();
		ss.add("sikn1.line1.temperature.value");
		ss.add("sikn1.line1.pressure.value");
		ss.add("sikn1.line1.obj.stn_dens.density.value");
		ss.add("sikn1.line1.total.present.mass");
		ss.add("sikn1.line1.total.present.impulse");
		ss.add("sikn1.line1.flowrate.mass.value");
		ss.add("sikn1.line1.obj.stn_dens.density.value");

		ss.add("sikn1.line2.temperature.value");
		ss.add("sikn1.line2.pressure.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");
		ss.add("sikn1.line2.total.present.mass");
		ss.add("sikn1.line2.total.present.impulse");
		ss.add("sikn1.line2.flowrate.mass.value");
		ss.add("sikn1.line2.obj.stn_dens.density.value");

		ss.add("sikn1.temperature.value");
		ss.add("sikn1.pressure.value");
		ss.add("sikn1.density.value");
		ss.add("sikn1.total.present.mass");
		ss.add("sikn1.total.present.impulse");
		ss.add("sikn1.flowrate.mass.value");
		ss.add("var.test_sikn_temp.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("sikn1.line1.temperature.value"));
		REQUIRE(ss("sikn1.line1.pressure.value"));
		REQUIRE(ss("sikn1.line1.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line1.total.present.mass"));
		REQUIRE(ss("sikn1.line1.total.present.impulse"));
		REQUIRE(ss("sikn1.line1.flowrate.mass.value"));

		REQUIRE(ss("sikn1.line2.temperature.value"));
		REQUIRE(ss("sikn1.line2.pressure.value"));
		REQUIRE(ss("sikn1.line2.obj.stn_dens.density.value"));
		REQUIRE(ss("sikn1.line2.total.present.mass"));
		REQUIRE(ss("sikn1.line2.total.present.impulse"));
		REQUIRE(ss("sikn1.line2.flowrate.mass.value"));

		REQUIRE(ss("sikn1.temperature.value"));
		REQUIRE(ss("sikn1.pressure.value"));
		REQUIRE(ss("sikn1.density.value"));
		REQUIRE(ss("sikn1.total.present.mass"));
		REQUIRE(ss("sikn1.flowrate.mass.value"));
		REQUIRE(ss("var.test_sikn_temp.value"));

		CHECK  (ss("sikn1.line1.temperature.value")->getValueLREAL() == temp1);
		CHECK  (ss("sikn1.line1.pressure.value")->getValueLREAL() == pres1);
		CHECK  (ss("sikn1.line1.obj.stn_dens.density.value")->getValueLREAL() == dens1);
		CHECK  (ss("sikn1.line1.total.present.mass")->getValueLREAL() == 6.0);
		CHECK  (ss("sikn1.line1.total.present.impulse")->getValueUDINT() == freq1 * 2);
		CHECK  (ss("sikn1.line1.flowrate.mass.value")->getValueLREAL() == 10800);

		CHECK  (ss("sikn1.line2.temperature.value")->getValueLREAL() == temp2);
		CHECK  (ss("sikn1.line2.pressure.value")->getValueLREAL() == pres2);
		CHECK  (ss("sikn1.line2.obj.stn_dens.density.value")->getValueLREAL() == dens2);
		CHECK  (ss("sikn1.line2.total.present.mass")->getValueLREAL() == 0.0);
		CHECK  (ss("sikn1.line2.total.present.impulse")->getValueUDINT() == 0);
		CHECK  (ss("sikn1.line2.flowrate.mass.value")->getValueLREAL() == 3600);

		CHECK  (ss("sikn1.temperature.value")->getValueLREAL() == ss("var.test_sikn_temp.value")->getValueLREAL());
		CHECK  (ss("sikn1.pressure.value")->getValueLREAL() == pres1);
		CHECK  (ss("sikn1.density.value")->getValueLREAL() == dens1);
		CHECK  (ss("sikn1.total.present.mass")->getValueLREAL() == 6.0);
		CHECK  (ss("sikn1.flowrate.mass.value")->getValueLREAL() == 10800);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
