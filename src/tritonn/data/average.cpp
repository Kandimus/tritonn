//=================================================================================================
//===
//=== average.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "average.h"
#include <limits>
//#include <cmath>
//#include <string.h>
#include "tinyxml2.h"
#include "event_eid.h"
#include "../text_id.h"
#include "../event_manager.h"
#include "../data_config.h"
#include "../data_manager.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../xml_util.h"
#include "../generator_md.h"
#include "../comment_defines.h"

rBitsArray rAverage::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rAverage::rAverage(const rStation* owner) : rSource(owner)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("NOAVRFAULT"  , static_cast<UINT>(Setup::NOAVRFAULT)  , "Не усреднять недостоверные входные значения")
				.add("CHECKFAULT"  , static_cast<UINT>(Setup::CHECKFAULT)  , "Выдавать недостоверность при наличии недостоверных значений на любом из входов")
				.add("NOCHECKFAULT", static_cast<UINT>(Setup::NOCHECKFAULT), "При усреднении не проверять флаг достоверности");
	}

	initLink(rLink::Setup::OUTPUT, m_outValue  , U_any     , SID::OUTPUT , XmlName::OUTPUT , rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_inValue[0], U_any     , SID::INPUT_1, XmlName::INPUT_1, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_inValue[1], U_any     , SID::INPUT_2, XmlName::INPUT_2, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_inValue[2], U_any     , SID::INPUT_3, XmlName::INPUT_3, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_inValue[3], U_any     , SID::INPUT_4, XmlName::INPUT_4, rLink::SHADOW_NONE);
	initLink(rLink::Setup::INPUT , m_inFault[0], U_discrete, SID::FAULT_1, XmlName::FAULT_1, XmlName::INPUT_1);
	initLink(rLink::Setup::INPUT , m_inFault[1], U_discrete, SID::FAULT_2, XmlName::FAULT_2, XmlName::INPUT_2);
	initLink(rLink::Setup::INPUT , m_inFault[2], U_discrete, SID::FAULT_3, XmlName::FAULT_3, XmlName::INPUT_3);
	initLink(rLink::Setup::INPUT , m_inFault[3], U_discrete, SID::FAULT_4, XmlName::FAULT_4, XmlName::INPUT_4);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAverage::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_AI_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_AI_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_AI_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_AI_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_AI_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_AI_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_AI_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_AI_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_AI_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_AI_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_AI_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_AI_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}

UDINT rAverage::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	UDINT count = 0;
	LREAL value = 0;

	for (auto ii = 0; ii < m_count; ++ii) {
		if (m_inFault[ii].m_value) {
			if (m_setup == Setup::CHECKFAULT) {
				m_fault = true;
				break;
			}

			if (m_setup == Setup::NOAVRFAULT) {
				continue;
			}
		}
		value += m_inValue[ii].m_value;
		++count;
	}

	m_outValue.m_value = (m_fault || !count) ? std::numeric_limits<LREAL>::quiet_NaN() : value / count;

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}


UDINT rAverage::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(m_alias + ".setup", TYPE_LREAL, rVariable::Flags::___, &m_setup, U_DIMLESS, 0, COMMENT::SETUP + m_flagsSetup.getInfo(true));

//	list.add(m_alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R__, &m_fault          , U_DIMLESS       , 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}

UDINT rAverage::check(rError& err)
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
UDINT rAverage::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
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

UDINT rAverage::generateMarkDown(rGeneratorMD& md)
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

