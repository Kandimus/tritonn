//=================================================================================================
//===
//=== precision.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс хранения точности отображения, по единицам измерения
//===
//=================================================================================================

#include "precision.h"


//-------------------------------------------------------------------------------------------------
//
rPrecision::rPrecision()
{
	Reset(PRECISION_DEFAUILT);
}



rPrecision::~rPrecision()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
rPrecision &rPrecision::Instance()
{
	static rPrecision Singleton;

	return Singleton;
}



USINT rPrecision::Get(UDINT unit)
{
	if(unit >= MAX_UNITS_COUNT) return DefPrec;

	return UnitPrec[unit];
}



UDINT rPrecision::Reset(USINT def)
{
	DefPrec = def;

	for(UDINT ii = 0; ii < MAX_UNITS_COUNT; ++ii)
	{
		UnitPrec[ii] = DefPrec;
	}

	return 0;
}


UDINT rPrecision::Load(tinyxml2::XMLElement *element)
{
	UDINT err = 0;

	if(nullptr == element) return tinyxml2::XML_SUCCESS;

	DefPrec = rDataConfig::GetAttributeUSINT(element, CFGNAME_DEFAULT, DefPrec);

	for(tinyxml2::XMLElement *prec = element->FirstChildElement(CFGNAME_UNIT); prec != nullptr; prec = prec->NextSiblingElement(CFGNAME_UNIT))
	{
		UDINT id  = rDataConfig::GetAttributeUDINT(prec, CFGNAME_ID, MAX_UNITS_COUNT);
		USINT val = rDataConfig::GetTextUSINT(prec, DefPrec, err);

		if(id >= MAX_UNITS_COUNT || err) return DATACFGERR_PREC_ID;

		UnitPrec[id] = val;
	}

	return tinyxml2::XML_SUCCESS;
}
