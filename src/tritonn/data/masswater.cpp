//=================================================================================================
//===
//=== masswater.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "masswater.h"
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

rBitsArray rMassWater::m_flagsSetup;

rMassWater::rMassWater(const rStation* owner) : rSource(owner)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("NOWATER", static_cast<UINT>(Setup::NOWATER), "При расчетах не учитывать значение плотности воды.");
	}

	initLink(rLink::Setup::OUTPUT, m_massWater  , U_perc_m, SID::RESULT     , XmlName::RESULT  , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_volWater   , U_perc_v, SID::VOLWATER   , XmlName::VOLWATER, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_density    , U_kg_m3 , SID::DENSITY    , XmlName::DENSITY , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_temperature, U_C     , SID::TEMPERATURE, XmlName::TEMP    , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rMassWater::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_MASWTR_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_MASWTR_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_MASWTR_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_MASWTR_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_MASWTR_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_MASWTR_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_MASWTR_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_MASWTR_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_MASWTR_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_MASWTR_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_MASWTR_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_MASWTR_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}

UDINT rMassWater::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	if (m_setup & Setup::NOWATER) {
		m_waterDensity = 1.0;
	} else {
		m_waterDensity = rDensity::getDensityOfWater(m_temperature.m_value);
	}

	m_massWater.m_value = m_volWater.m_value * m_waterDensity / m_density.m_value;

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}


UDINT rMassWater::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(m_alias + ".setup"       , TYPE_UINT, rVariable::Flags::R__D, &m_setup       , U_DIMLESS, 0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".densitywater",            rVariable::Flags::R___, &m_waterDensity, U_kg_m3  , 0, "Вычисленное значение плотности воды");

	list.add(m_alias + ".fault"       ,            rVariable::Flags::R___, &m_fault, U_DIMLESS, 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rMassWater::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(static_cast<UDINT>(Setup::NONE)));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	UDINT fault = 0;
	m_setup = static_cast<Setup>(m_flagsSetup.getValue(strSetup, fault));
	if (fault) {
		return err.set(DATACFGERR_MASSWATER_SETUP, element->GetLineNum(), "");
	}

	auto xml_volwater = element->FirstChildElement(XmlName::VOLWATER);

	if (!xml_volwater) {
		return err.set(DATACFGERR_MASSWATER_NOVOLWATER, element->GetLineNum(), "");
	}

	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_volwater->FirstChildElement(XmlName::LINK), m_volWater)) return err.getError();

	auto xml_density = element->FirstChildElement(XmlName::DENSITY);

	if (!xml_density) {
		return err.set(DATACFGERR_MASSWATER_NODENSITY, element->GetLineNum(), "");
	}

	if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_density->FirstChildElement(XmlName::LINK), m_density)) return err.getError();

	if (!(m_setup & Setup::NOWATER)) {
		auto xml_temperature = element->FirstChildElement(XmlName::TEMP);

		if (!xml_temperature) {
			return err.set(DATACFGERR_MASSWATER_NOTEMPERATURE, element->GetLineNum(), "");
		}

		if(TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_temperature->FirstChildElement(XmlName::LINK), m_temperature)) return err.getError();
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rMassWater::generateMarkDown(rGeneratorMD& md)
{
	m_temperature.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_density.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_volWater.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_massWater.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE)
			.addProperty(XmlName::SETUP, &m_flagsSetup, true)
			.addRemark("> При отсутвие флага NOWATER производтся расчет плотности воды согласно таблице из \"__ГСССД 2-77__ ВОДА. ПЛОТНОСТЬ ПРИ АТМОСФЕРНОМ ДАВЛЕНИИ И ТЕМПЕРАТУРАХ от 0 до 100 °С\".");

	return TRITONN_RESULT_OK;
}

