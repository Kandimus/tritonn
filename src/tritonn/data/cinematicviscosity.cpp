//=================================================================================================
//===
//=== cinematicviscosity.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "cinematicviscosity.h"
#include <limits>
#include "../event/eid.h"
#include "../event/manager.h"
#include "../text_id.h"
#include "../data_config.h"
#include "../data_manager.h"
#include "../densitywater.h"
#include "../error.h"
#include "../variable_list.h"
#include "../xml_util.h"
#include "../generator_md.h"
#include "../comment_defines.h"


rCinematicViscosity::rCinematicViscosity(const rStation* owner) : rSource(owner)
{
	initLink(rLink::Setup::OUTPUT, m_cinVisc, U_sSt  , SID::CINVISC, XmlName::CINVISC, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_dynVisc, U_sP   , SID::DYNVISC, XmlName::DYNVISC, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_density, U_kg_m3, SID::DENSITY, XmlName::DENSITY, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rCinematicViscosity::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_CINVISC_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_CINVISC_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_CINVISC_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_CINVISC_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_CINVISC_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_CINVISC_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_CINVISC_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_CINVISC_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_CINVISC_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_CINVISC_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_CINVISC_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_CINVISC_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}

UDINT rCinematicViscosity::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	m_cinVisc.m_value = m_dynVisc.m_value * m_density.m_value / 1000;

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}


UDINT rCinematicViscosity::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(m_alias + ".fault", rVariable::Flags::R__, &m_fault, U_DIMLESS, 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rCinematicViscosity::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	auto xml_dynvisc = element->FirstChildElement(XmlName::DYNVISC);
	auto xml_density = element->FirstChildElement(XmlName::DENSITY);

	if (!xml_dynvisc) {
		return err.set(DATACFGERR_CINVISC_NODYNVISC, element->GetLineNum(), "");
	}

	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_dynvisc->FirstChildElement(XmlName::LINK), m_dynVisc)) return err.getError();

	if (!xml_density) {
		return err.set(DATACFGERR_CINVISC_NODENSITY, element->GetLineNum(), "");
	}

	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_density->FirstChildElement(XmlName::LINK), m_density)) return err.getError();

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rCinematicViscosity::generateMarkDown(rGeneratorMD& md)
{
	m_density.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_cinVisc.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_dynVisc.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE);

	return TRITONN_RESULT_OK;
}

