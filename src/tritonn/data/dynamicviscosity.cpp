//=================================================================================================
//===
//=== dynamicviscosity.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "dynamicviscosity.h"
#include <limits>
#include "xml_util.h"
#include "../event/eid.h"
#include "../event/manager.h"
#include "../text_id.h"
#include "../data_config.h"
#include "../data_manager.h"
#include "../error.h"
#include "../variable_list.h"
#include "../generator_md.h"
#include "../comment_defines.h"


rDynamicViscosity::rDynamicViscosity(const rStation* owner) : rSource(owner)
{
	initLink(rLink::Setup::OUTPUT, m_dynVisc, U_sP   , SID::RESULT , XmlName::RESULT , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_kinVisc, U_sSt  , SID::KINVISC, XmlName::KINVISC, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_density, U_kg_m3, SID::DENSITY, XmlName::DENSITY, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDynamicViscosity::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_DYNVISC_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_DYNVISC_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_DYNVISC_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_DYNVISC_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_DYNVISC_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_DYNVISC_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_DYNVISC_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_DYNVISC_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_DYNVISC_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_DYNVISC_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_DYNVISC_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_DYNVISC_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}

UDINT rDynamicViscosity::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	m_dynVisc.m_value = m_kinVisc.m_value / m_density.m_value * 1000;

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}


UDINT rDynamicViscosity::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(m_alias + ".fault", rVariable::Flags::R___, &m_fault, U_DIMLESS, 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rDynamicViscosity::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	auto xml_kinvisc = element->FirstChildElement(XmlName::KINVISC);
	auto xml_density = element->FirstChildElement(XmlName::DENSITY);

	if (!xml_kinvisc) {
		return err.set(DATACFGERR_DYNVISC_NOKINVISC, element->GetLineNum(), "");
	}

	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_kinvisc->FirstChildElement(XmlName::LINK), m_kinVisc)) {
		return err.getError();
	}

	if (!xml_density) {
		return err.set(DATACFGERR_DYNVISC_NODENSITY, element->GetLineNum(), "");
	}

	if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_density->FirstChildElement(XmlName::LINK), m_density)) {
		return err.getError();
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rDynamicViscosity::generateMarkDown(rGeneratorMD& md)
{
	m_density.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_kinVisc.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_dynVisc.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE);

	return TRITONN_RESULT_OK;
}
