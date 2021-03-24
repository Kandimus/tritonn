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
	SECTION("Set factor value") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("sikn1.obj.dens1.factor.set.k0"  , 1.1);
		ss.add("sikn1.obj.dens1.factor.set.k1"  , 2.2);
		ss.add("sikn1.obj.dens1.factor.set.k2"  , 3.3);
		ss.add("sikn1.obj.dens1.factor.set.k18" , 4.4);
		ss.add("sikn1.obj.dens1.factor.set.k19" , 5.5);
		ss.add("sikn1.obj.dens1.factor.set.k20a", 6.6);
		ss.add("sikn1.obj.dens1.factor.set.k20b", 7.7);
		ss.add("sikn1.obj.dens1.factor.set.k21a", 8.8);
		ss.add("sikn1.obj.dens1.factor.set.k21b", 9.9);
		ss.add("sikn1.obj.dens1.factor.set.accept", 1);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("sikn1.obj.dens1.factor.k0"  );
		ss.add("sikn1.obj.dens1.factor.k1"  );
		ss.add("sikn1.obj.dens1.factor.k2"  );
		ss.add("sikn1.obj.dens1.factor.k18" );
		ss.add("sikn1.obj.dens1.factor.k19" );
		ss.add("sikn1.obj.dens1.factor.k20a");
		ss.add("sikn1.obj.dens1.factor.k20b");
		ss.add("sikn1.obj.dens1.factor.k21a");
		ss.add("sikn1.obj.dens1.factor.k21b");
		ss.add("sikn1.obj.dens1.factor.set.accept");
		ss.get();

		REQUIRE(ss("sikn1.obj.dens1.factor.k0"    ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k1"    ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k2"    ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k18"   ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k19"   ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k20a"  ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k20b"  ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k21a"  ));
		REQUIRE(ss("sikn1.obj.dens1.factor.k21b"  ));
		REQUIRE(ss("sikn1.obj.dens1.factor.set.accept"));
		CHECK  (ss("sikn1.obj.dens1.factor.k0"    )->getValueLREAL() == 1.1);
		CHECK  (ss("sikn1.obj.dens1.factor.k1"    )->getValueLREAL() == 2.2);
		CHECK  (ss("sikn1.obj.dens1.factor.k2"    )->getValueLREAL() == 3.3);
		CHECK  (ss("sikn1.obj.dens1.factor.k18"   )->getValueLREAL() == 4.4);
		CHECK  (ss("sikn1.obj.dens1.factor.k19"   )->getValueLREAL() == 5.5);
		CHECK  (ss("sikn1.obj.dens1.factor.k20a"  )->getValueLREAL() == 6.6);
		CHECK  (ss("sikn1.obj.dens1.factor.k20b"  )->getValueLREAL() == 7.7);
		CHECK  (ss("sikn1.obj.dens1.factor.k21a"  )->getValueLREAL() == 8.8);
		CHECK  (ss("sikn1.obj.dens1.factor.k21b"  )->getValueLREAL() == 9.9);
		CHECK  (ss("sikn1.obj.dens1.factor.set.accept")->getValueUINT()  == 0);
	}

	SECTION("Set fi value. Calculating density") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);
		LREAL density_freq = 730;
		LREAL density_val  = 855.68901907;

		ss.add("hardware.fi4_2.ch_00.simulate.type" , static_cast<USINT>(rIOFIChannel::SimType::CONST));
		ss.add("hardware.fi4_2.ch_00.simulate.value", density_freq);
		ss.set();

		mSleep(rTest::sleepValue + 1000);

		ss.clear();
		ss.add("sikn1.obj.dens1.factor.set.k0"  , -1.27753000E+03);
		ss.add("sikn1.obj.dens1.factor.set.k1"  , -2.99673000E-01);
		ss.add("sikn1.obj.dens1.factor.set.k2"  ,  1.35555000E-03);
		ss.add("sikn1.obj.dens1.factor.set.k18" , -1.90175000E-05);
		ss.add("sikn1.obj.dens1.factor.set.k19" ,  1.54297000E-02);
		ss.add("sikn1.obj.dens1.factor.set.k20a", -2.31789000E-04);
		ss.add("sikn1.obj.dens1.factor.set.k20b",  1.24990000E-06);
		ss.add("sikn1.obj.dens1.factor.set.k21a",  1.98986000E-01);
		ss.add("sikn1.obj.dens1.factor.set.k21b", -3.19623000E-03);
		ss.add("sikn1.obj.dens1.factor.set.accept", 1);
		ss.add("sikn1.bik.io.temp1.present.value", 10);
		ss.add("sikn1.bik.io.pres.present.value", 0.1);
		REQUIRE(ss("sikn1.bik.io.pres.present.value"));
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("sikn1.obj.dens1.density.value");
		ss.get();

		LREAL epsilon = Catch::Epsilon::instance().setDouble(0.0001);

		REQUIRE(ss("sikn1.obj.dens1.density.value"));
		CHECK  (ss("sikn1.obj.dens1.density.value")->getValueLREAL() == density_val);

		Catch::Epsilon::instance().setDouble(epsilon);
	}
}
