//=================================================================================================
//===
//=== total.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Нарастающие
//===
//=================================================================================================


#include "precision.h"
#include "total.h"


UDINT rTotal::CheckMax()
{
	UDINT result = 0;

	if(Raw.Mass >= MAX_TOTAL_LIMIT)
	{
		result   |= TOTAL_MAX_MASS;
		Raw.Mass -= MAX_TOTAL_LIMIT;
	}

	if(Raw.Volume >= MAX_TOTAL_LIMIT)
	{
		result     |= TOTAL_MAX_VOLUME;
		Raw.Volume -= MAX_TOTAL_LIMIT;
	}

	if(Raw.Volume15 >= MAX_TOTAL_LIMIT)
	{
		result       |= TOTAL_MAX_VOLUME15;
		Raw.Volume15 -= MAX_TOTAL_LIMIT;
	}

	if(Raw.Volume20 >= MAX_TOTAL_LIMIT)
	{
		result       |= TOTAL_MAX_VOLUME20;
		Raw.Volume20 -= MAX_TOTAL_LIMIT;
	}

	return result;
}



UDINT rTotal::Calculate(UDINT unitmass, UDINT unitvol)
{
	UDINT result = 0;

	Past             = Present;

	Raw.Mass         = Round(Raw.Mass     + Inc.Mass    , 5);
	Raw.Volume       = Round(Raw.Volume   + Inc.Volume  , 5);
	Raw.Volume15     = Round(Raw.Volume15 + Inc.Volume15, 5);
	Raw.Volume20     = Round(Raw.Volume20 + Inc.Volume20, 5);
	Raw.Count       += Inc.Count;

	result           = CheckMax();

	Present.Mass     = Round(Raw.Mass    , rPrecision::Instance().Get(unitmass));
	Present.Volume   = Round(Raw.Volume  , rPrecision::Instance().Get(unitvol));
	Present.Volume15 = Round(Raw.Volume15, rPrecision::Instance().Get(unitvol));
	Present.Volume20 = Round(Raw.Volume20, rPrecision::Instance().Get(unitvol));

	Inc.Mass         = 0.0;
	Inc.Volume       = 0.0;
	Inc.Volume15     = 0.0;
	Inc.Volume20     = 0.0;
	Inc.Count        = 0;

	return result;
}


// Корректная разница нарастающих, с учетом сброса по пределу MAX_TOTAL_LIMIT
LREAL rTotal::Sub(LREAL sub1, LREAL sub2)
{
	LREAL result = sub1 - sub2;

	if(result >= 0.0) return result;

	return (-result) + (MAX_TOTAL_LIMIT - sub2);
}


void rTotal::Clear(rBaseTotal &total)
{
	total.Count    = 0;
	total.Mass     = 0.0;
	total.Volume   = 0.0;
	total.Volume15 = 0.0;
	total.Volume20 = 0.0;
}
