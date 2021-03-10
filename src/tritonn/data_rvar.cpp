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
				.add("CONST", static_cast<UINT>(Setup::CONST), "Установить как константу");
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

	tinyxml2::XMLElement* xml_value = element->FirstChildElement(XmlName::VALUE);
	tinyxml2::XMLElement* xml_unit  = element->FirstChildElement(XmlName::UNIT);

	if(!xml_value || !xml_unit) {
		return err.set(DATACFGERR_VAR, element->GetLineNum(), "fault value or unit");
	}

	UDINT fault = 0;
	m_setup         = m_flagsSetup.getValue(strSetup, fault);
	m_value.m_value = XmlUtils::getTextLREAL(xml_value, 0.0  , fault);
	m_value.m_unit  = XmlUtils::getTextUDINT(xml_unit , U_any, fault);

	if (fault) {
		return err.set(DATACFGERR_VAR, element->GetLineNum(), "");
	}

	// Если переменная константа, то снимаем флаг записи
	if(m_setup & Setup::CONST) {
		m_value.m_setup &= ~rLink::Setup::WRITABLE;
	}

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rRVar::generateMarkDown(rGeneratorMD& md)
{
	m_value.m_limit.m_setup.Init (LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::CALCULATE)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addXml(XmlName::VALUE, m_value.m_value)
			.addXml(XmlName::UNIT , static_cast<UDINT>(m_value.m_unit));

	return TRITONN_RESULT_OK;
}
