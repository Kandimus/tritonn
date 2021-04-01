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
	reset(rPrecision::DEFAUILT);
}

rPrecision::~rPrecision()
{
	;
}

USINT rPrecision::get(UDINT unit)
{
	if(unit >= MAX_UNITS_COUNT) return m_defPrec;

	return m_unitPrec[unit];
}



void rPrecision::reset(USINT def)
{
	m_defPrec = def;

	for(UDINT ii = 0; ii < MAX_UNITS_COUNT; ++ii)
	{
		m_unitPrec[ii] = m_defPrec;
	}
}


UDINT rPrecision::load(tinyxml2::XMLElement* element, rError& err)
{
	UDINT fault = 0;

	if (!element) {
		return TRITONN_RESULT_OK;
	}

	m_defPrec = XmlUtils::getAttributeUSINT(element, XmlName::DEFAULT, m_defPrec);

	XML_FOR(xml_prec, element, XmlName::UNIT) {
		UDINT id  = XmlUtils::getAttributeUDINT(xml_prec, XmlName::ID, MAX_UNITS_COUNT);
		USINT val = XmlUtils::getTextUSINT(xml_prec, m_defPrec, fault);

		if (id >= MAX_UNITS_COUNT || fault) {
			return err.set(DATACFGERR_PREC_ID, xml_prec->GetLineNum(), "");
		}

		m_unitPrec[id] = val;
	}

	return TRITONN_RESULT_OK;
}
