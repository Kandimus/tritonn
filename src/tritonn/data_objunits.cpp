//=================================================================================================
//===
//=== data_objunits.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Набор ед. изм. для станции и линии
//===
//=================================================================================================

#include "data_objunits.h"
#include "error.h"
#include "xml_util.h"

UDINT rObjUnit::LoadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	tinyxml2::XMLElement* xml_units = element->FirstChildElement(XmlName::UNITS);

	if (!xml_units) {
		return TRITONN_RESULT_OK;
	}

	UDINT fault = 0;
	m_mass        = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::MASS)   , m_mass       , fault);
	m_volume      = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::VOLUME) , m_volume     , fault);
	m_temperature = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::TEMP)   , m_temperature, fault);
	m_pressure    = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::PRES)   , m_pressure   , fault);
	m_density     = XmlUtils::getTextUDINT(xml_units->FirstChildElement(XmlName::DENSITY), m_density    , fault);

	if (fault) {
		return err.set(DATACFGERR_OBJECT_UNITS, xml_units->GetLineNum(), "");
	}
}
