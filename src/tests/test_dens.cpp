#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_ai.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "io/fi_channel.h"
#include "data_denssol.h"

// Set mode

TEST_CASE("testing densitometer.", "[DensSol]")
{
	SECTION("Set fi value. Calculating density") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL density_val = 888.072;

		ss.add("hardware.fi4_2.ch_01.simulate.type"  ,  static_cast<USINT>(rIOFIChannel::SimType::Const));
		ss.add("hardware.fi4_2.ch_01.simulate.value" , 750);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("sikn1.bik.obj.dens1.factor.set.k0"  , -1.27753000E+03);
		ss.add("sikn1.bik.obj.dens1.factor.set.k1"  , -2.99673000E-01);
		ss.add("sikn1.bik.obj.dens1.factor.set.k2"  ,  1.35555000E-03);
		ss.add("sikn1.bik.obj.dens1.factor.set.k18" , -1.90175000E-05);
		ss.add("sikn1.bik.obj.dens1.factor.set.k19" ,  1.54297000E-02);
		ss.add("sikn1.bik.obj.dens1.factor.set.k20a", -2.31789000E-04);
		ss.add("sikn1.bik.obj.dens1.factor.set.k20b",  1.24990000E-06);
		ss.add("sikn1.bik.obj.dens1.factor.set.k21a",  1.98986000E-01);
		ss.add("sikn1.bik.obj.dens1.factor.set.k21b", -3.19623000E-03);
		ss.add("sikn1.bik.obj.dens1.factor.set.accept", 1);
		ss.add("sikn1.bik.io.temp1.present.value", 10);
		ss.add("sikn1.bik.io.pres1.present.value", 0.1);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("sikn1.bik.obj.dens1.density");
		ss.get();

		REQUIRE(ss("sikn1.bik.obj.dens1.density"));
		CHECK  (ss("sikn1.bik.obj.dens1.density")->getValueLREAL() == density_val);
	}
}
