#include <limits>
#include <cmath>
#include "simpletest.h"
#include "data_manager.h"
#include "simpletest.h"


S_NEW_TEST( AnalogInput, "testing analog input. IO simulate")
{
	 S_SECTION("set simulate value") {
		  const LREAL testvalue = 22.0;
		  rSnapshot set_ss;
		  rSnapshot get_ss;

		  set_ss.SetAccess(ACCESS_MASK_ADMIN);
		  set_ss.Add("my_ai.sim.value", testvalue);
		  set_ss.Add("my_ai", AI_MODE_MKEYPAD);
		  rDataManager::Instance().Set(set_ss);

		  mSleep(600);

		  get_ss.Add("my_ai.sim.value");

		  S_CHECK(std::abs(get_ss("my_ai....")->GetValueLREAL() - testvalue) < std::numeric_limits<LREAL>::epsilon());
	 }

	 S_SECTION("testing equal") {
		  int a = 11;
		  S_CHECK(a == 11);
	 }
}

S_NEW_TEST( Analog2, "testing analog signals 2222")
{
	 S_SECTION("div by 0") {
		  int a = 10 / 10;
		  S_CHECK(a == 2);
	 }

	 S_SECTION("Test gist") {
		  S_CHECK(1 == 1);
	 }
}
