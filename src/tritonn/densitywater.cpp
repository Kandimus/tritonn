//=================================================================================================
//===
//=== densitywater.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "densitywater.h"

namespace rDensity
{

LREAL getDensityOfWater(LREAL tempwater)
{
	if (tempwater < 0) {
		tempwater = 0;
	}

	if (tempwater > 100) {
		tempwater = 100;
	}

	DINT temp = static_cast<DINT>(tempwater * 10 + 0.5);

	return DENSWATER[temp];
}

}
