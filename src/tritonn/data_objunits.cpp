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

	return TRITONN_RESULT_OK;
}

STRID rObjUnit::getFlowVolume() const
{
	switch (m_volume.toUDINT()) {
		case U_m3   : return U_m3_h;
		case U_liter: return U_ltr_h;
		default     : return U_UNDEF;
	}
}

STRID rObjUnit::getFlowMass() const
{
	switch (m_mass.toUDINT()) {
		case U_t : return U_t_h;
		case U_kg: return U_kg_h;
		default  : return U_UNDEF;
	}
}

STRID rObjUnit::getTemperature() const
{
	return m_temperature;
}

STRID rObjUnit::getPressure() const
{
	return m_pressure;
}

STRID rObjUnit::getDensity() const
{
	return m_density;
}

STRID rObjUnit::getMass() const
{
	return m_mass;
}

STRID rObjUnit::getVolume() const
{
	return m_volume;
}

STRID rObjUnit::getMassKF() const
{
	switch (m_mass) {
		case U_t : return U_imp_t;
		case U_kg: return U_imp_kg;
		default  : return U_UNDEF;
	}
}

STRID rObjUnit::getVolumeKF() const
{
	switch (m_volume) {
		case U_m3   : return U_imp_m3;
		case U_liter: return U_imp_ltr;
		default     : return U_UNDEF;
	}
}
