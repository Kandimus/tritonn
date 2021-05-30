#include <limits>
#include <cmath>
#include "../catchtest/catch.hpp"
#include "test.h"
#include "data_rvar.h"
#include "data_manager.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"

TEST_CASE("live check", "[Live]")
{
	SECTION("Check const") {
		rSnapshot ss(rDataManager::instance().getVariableClass(), ACCESS_MASK_ADMIN);

		ss.add("system.state.live");
		ss.get();

		REQUIRE(ss("system.state.live"));
		CHECK  (ss("system.state.live")->getValueUINT() == static_cast<UINT>(Live::RUNNING));
	}
}
