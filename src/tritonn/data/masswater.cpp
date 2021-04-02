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
#include "tinyxml2.h"
#include "../event/eid.h"
#include "../event/manager.h"
#include "../text_id.h"
#include "../data_config.h"
#include "../data_manager.h"
#include "../error.h"
#include "../variable_list.h"
#include "../xml_util.h"
#include "../generator_md.h"
#include "../comment_defines.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rMassWater::rMassWater(const rStation* owner) : rSource(owner)
{
	initLink(rLink::Setup::OUTPUT, m_massWater, U_perc_m   , SID::MASSWATER, XmlName::MASSWATER, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_volWater , U_perc_v   , SID::VOLWATER , XmlName::VOLWATER , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_density  , U_kg_m3    , SID::DENSITY  , XmlName::DENSITY  , rLink::SHADOW_NONE);
//	initLink(rLink::Setup::INPUT , m_inFault[0], U_discrete, SID::FAULT_1, XmlName::FAULT_1, XmlName::INPUT_1);
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
	list.add(m_alias + ".water.density", TYPE_LREAL, rVariable::Flags::___, &m_waterDensity, U_kg_m3, ACCESS_FACTORS, "Вычисленное значение плотности воды");

//	list.add(m_alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R__, &m_fault          , U_DIMLESS       , 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}

UDINT rMassWater::check(rError& err)
{
	STRID unit = m_inValue[0].getSourceUnit();

	for (UDINT ii = 1; ii < m_count; ++ii) {
		if (unit != m_inValue[ii].getSourceUnit()) {
			return err.set(DATACFGERR_AVERAGE_DIFFUNITS, m_lineNum, "");
		}
	}

	m_outValue.m_unit = unit;

	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rMassWater::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(static_cast<UDINT>(Setup::NOAVRFAULT)));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	UDINT fault = 0;
	m_setup = static_cast<Setup>(m_flagsSetup.getValue(strSetup, fault));
	if (fault) {
		return err.set(DATACFGERR_AVERAGE_SETUP, element->GetLineNum(), "");
	}

	tinyxml2::XMLElement* xml_inputs = element->FirstChildElement(XmlName::INPUTS);

	if (!xml_inputs) {
		return err.set(DATACFGERR_AVERAGE_NOINPUT, element->GetLineNum(), "");
	}

	m_count = 0;
	XML_FOR(xml_input, xml_inputs, XmlName::INPUT) {
		if (m_count >= COUNT) {
			return err.set(DATACFGERR_AVERAGE_TOOMANYINPUT, xml_input->GetLineNum(), "");
		}

		if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_input->FirstChildElement(XmlName::LINK), m_inValue[m_count])) {
			return err.getError();
		}
		++m_count;
	}

	if (m_count < 2) {
		return err.set(DATACFGERR_AVERAGE_TOOFEWINPUT, xml_inputs->GetLineNum(), "");
	}

	tinyxml2::XMLElement* xml_faults = element->FirstChildElement(XmlName::FAULTS);

	if (xml_faults) {
		UDINT count = 0;
		XML_FOR(xml_fault, xml_faults, XmlName::FAULT) {
			if (count >= COUNT) {
				return err.set(DATACFGERR_AVERAGE_TOOMANYFAULTS, xml_fault->GetLineNum(), "");
			}

			if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_fault->FirstChildElement(XmlName::LINK), m_inFault[count], m_inValue[count], XmlName::FAULT)) {
				return err.getError();
			}
			++count;
		}

		if (m_count != count) {
			return err.set(DATACFGERR_AVERAGE_DIFFFAULTS, xml_faults->GetLineNum(), "");
		}
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rMassWater::generateMarkDown(rGeneratorMD& md)
{
	m_inValue[0].m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_inValue[1].m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_inValue[2].m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_inValue[3].m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_outValue.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, false, rGeneratorMD::Type::CALCULATE)
			.addProperty(XmlName::SETUP, &m_flagsSetup, true)
			.addXml("<" + std::string(XmlName::INPUTS) + ">")
			.addLink(XmlName::INPUT, false, "\t")
			.addLink(XmlName::INPUT, false, "\t")
			.addLink(XmlName::INPUT, true, "\t")
			.addLink(XmlName::INPUT, true, "\t")
			.addXml("</" + std::string(XmlName::INPUTS) + ">")
			.addXml("<" + std::string(XmlName::FAULTS) + ">" + rGeneratorMD::rItem::XML_OPTIONAL)
			.addLink(XmlName::FAULT, false, "\t")
			.addLink(XmlName::FAULT, false, "\t")
			.addLink(XmlName::FAULT, false, "\t")
			.addLink(XmlName::FAULT, false, "\t")
			.addXml("</" + std::string(XmlName::FAULTS) + ">")
			.addXml(getXmlLimits("\t"))
			.addRemark("> Единицы измерений для всех входных значений должны быть одинаковы! В противном случае конфигурация не будет загружена.\n"
					   "> Если указаны флаги недостоверности, то их количество должно быть равно количеству усредняемым входам.");

	return TRITONN_RESULT_OK;
}

