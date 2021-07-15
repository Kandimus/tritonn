/*
 *
 * ao_channel.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "ao_channel.h"
#include <math.h>
#include "xml_util.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "../error.h"
#include "../comment_defines.h"

rBitsArray rIOAOChannel::m_flagsSetup;
rBitsArray rIOAOChannel::m_flagsType;

rIOAOChannel::rIOAOChannel(USINT index, const std::string& comment) : rIOBaseChannel(rIOBaseChannel::Type::AO, index, comment)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF", static_cast<UINT>(Setup::OFF)    , COMMENT::SETUP_OFF);
	}

	if (m_flagsType.empty()) {
		m_flagsType
				.add("ACTIVE" , static_cast<UINT>(Type::ACTIVE) , "Активный канал")
				.add("PASSIVE", static_cast<UINT>(Type::PASSIVE), "Пасивный канал");
	}
}

UDINT rIOAOChannel::processing()
{
	rIOBaseChannel::processing();

	if (m_ADC > MAX) {
		m_ADC = MAX;
	}

	if (m_ADC < MIN) {
		m_ADC = MIN;
	}

	return TRITONN_RESULT_OK;
}

UDINT rIOAOChannel::simulate()
{
	++m_pullingCount;

	return TRITONN_RESULT_OK;
}

UDINT rIOAOChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, false);

	list.add(p + "setup"  , TYPE::UINT, rVariable::Flags::RS__, &m_setup  , U_DIMLESS, 0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(p + "current",             rVariable::Flags::____, &m_ADC    , U_mkA    , 0, "Выдаваемый ток");
	list.add(p + "type"   , TYPE::UINT, rVariable::Flags::R___, &m_type   , U_DIMLESS, 0, "Тип канала:<br/>" + m_flagsType.getInfo(true));

	return TRITONN_RESULT_OK;
}

UDINT rIOAOChannel::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP , "");
	UDINT       fault    = 0;

	m_setup = m_flagsSetup.getValue(strSetup, fault);

	if (fault) {
		return err.set(DATACFGERR_IO_CHANNEL, element->GetLineNum(), "invalid setup");
	}

	return TRITONN_RESULT_OK;
}
