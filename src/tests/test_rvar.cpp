#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_rvar.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

TEST_CASE("testing variables", "[RVar]")
{
	SECTION("Check const") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("var.var1const.value", 555.0);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("var.var1const.value");
		ss.get();

		REQUIRE(ss("var.var1const.value"));
		CHECK  (ss("var.var1const.value")->getValueLREAL() == 11.11);
	}

	SECTION("Check set variable") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		LREAL var = 555;
		ss.add("var.var2.value", var);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("var.var2.value");
		ss.get();

		REQUIRE(ss("var.var2.value"));
		CHECK  (ss("var.var2.value")->getValueLREAL() == var);
	}

	SECTION("Check set variable") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		LREAL var = 444;
		ss.add("var.var2.value", var);
		ss.set();

		mSleep(rTest::sleepValue);

		ss.clear();
		ss.add("var.linkvar2.value");
		ss.get();

		REQUIRE(ss("var.linkvar2.value"));
		CHECK  (ss("var.linkvar2.value")->getValueLREAL() == var);
	}
}
