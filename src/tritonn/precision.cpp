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
#include "error.h"
#include "xml_util.h"


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


UDINT rPrecision::Load(tinyxml2::XMLElement* element, rError& err)
{
	UDINT fault = 0;

	if (!element) {
		return TRITONN_RESULT_OK;
	}

	DefPrec = XmlUtils::getAttributeUSINT(element, XmlName::DEFAULT, DefPrec);

	XML_FOR(xml_prec, element, XmlName::UNIT) {
		UDINT id  = XmlUtils::getAttributeUDINT(xml_prec, XmlName::ID, MAX_UNITS_COUNT);
		USINT val = XmlUtils::getTextUSINT(xml_prec, DefPrec, fault);

		if (id >= MAX_UNITS_COUNT || fault) {
			return err.set(DATACFGERR_PREC_ID, xml_prec->GetLineNum(), "");
		}

		UnitPrec[id] = val;
	}

	return TRITONN_RESULT_OK;
}
