//=================================================================================================
//===
//=== data_rvar.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс простой вещественной переменной
//===
//=================================================================================================

#include <vector>
#include "tinyxml2.h"
#include "text_id.h"
#include "data_manager.h"
#include "data_config.h"
#include "error.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_rvar.h"
#include "xml_util.h"
#include "generator_md.h"

rBitsArray rRVar::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rRVar::rRVar() : rSource(), m_setup(0)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("CONST", static_cast<UINT>(Setup::CONST), "Установить как константу")
				.add("LINK" , static_cast<UINT>(Setup::LINK ), "Запретить изменение переменной. Значение будет получено со входа");
				;
	}

	initLink(rLink::Setup::INOUTPUT | rLink::Setup::NONAME | rLink::Setup::WRITABLE,
			 m_value, U_any, SID::VAR_VALUE, XmlName::VALUE, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_VAR_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_VAR_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_VAR_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_VAR_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_VAR_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_VAR_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_VAR_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_VAR_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_VAR_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_VAR_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_VAR_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_VAR_NORMAL)    << link.m_descr << link.m_unit;

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rRVar::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, "");

	if(TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	UDINT fault = 0;
	m_setup = m_flagsSetup.getValue(strSetup, fault);

	if (fault) {
		return err.set(DATACFGERR_VAR_SETUP, element->GetLineNum(), "setup fault");
	}

	auto xml_default = element->FirstChildElement(XmlName::DEFAULT);
	auto xml_unit    = element->FirstChildElement(XmlName::UNIT);
	auto xml_value   = element->FirstChildElement(XmlName::VALUE);

	if (!xml_unit) {
		return err.set(DATACFGERR_VAR_UNIT, element->GetLineNum(), "missing unit");
	}

	m_value.m_unit = XmlUtils::getTextUDINT(xml_unit, U_any, fault);
	if (fault) {
		return err.set(DATACFGERR_VAR_UNIT, element->GetLineNum(), "fault unit");
	}

	if (!(m_setup & Setup::LINK)) {
		if (!xml_default) {
			return err.set(DATACFGERR_VAR_DEFAULT, element->GetLineNum(), "fault defautl value");
		}

		m_value.m_value = XmlUtils::getTextLREAL(xml_default, 0.0, fault);

		if (fault) {
			return err.set(DATACFGERR_VAR_DEFAULT, element->GetLineNum(), "fault default value");
		}

		m_value.m_setup &= ~rLink::Setup::WRITABLE;

	} else {
		if(!xml_value) {
			return err.set(DATACFGERR_VAR_LINK, element->GetLineNum(), "fault link");
		}

		if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_value->FirstChildElement(XmlName::LINK), m_value)) {
			return err.getError();
		}

		if (m_setup & Setup::CONST) {
			m_value.m_setup &= ~rLink::Setup::WRITABLE;
		}
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rRVar::generateMarkDown(rGeneratorMD& md)
{
	m_value.m_limit.m_setup.Init (LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addXml(XmlName::DEFAULT, m_value.m_value)
			.addXml(XmlName::UNIT   , static_cast<UDINT>(m_value.m_unit))
			.addRemark("Если установлен флаг LINK, то в переменную value запись будет не возможна.");

	return TRITONN_RESULT_OK;
}
