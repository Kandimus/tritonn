#include "../simpletest/simpletest.h"


S_NEW_TEST(Analog, "testing analog signals")
{
	S_SECTION("testing equal (fail)") {
		S_CHECK(1 == 1);
		S_CHECK(1 == 2);
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
