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
rBitsArray rIOAOChannel::m_flagsMode;

rIOAOChannel::rIOAOChannel(USINT index, const std::string& comment) : rIOBaseChannel(rIOBaseChannel::Type::AO, index, comment)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF", static_cast<UINT>(Setup::OFF)    , COMMENT::SETUP_OFF);
	}

	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("ACTIVE" , static_cast<UINT>(Mode::ACTIVE) , "Активный канал")
				.add("PASSIVE", static_cast<UINT>(Mode::PASSIVE), "Пасивный канал");
	}
}

UDINT rIOAOChannel::processing()
{
	rIOBaseChannel::processing();

	if (m_ADC > getMaxValue()) {
		m_ADC = getMaxValue();
	}

	if (m_ADC < getMinValue()) {
		m_ADC = getMinValue();
	}

	return TRITONN_RESULT_OK;
}

UINT rIOAOChannel::getMinValue() const
{
	return m_regime == Regime::REDUCED_DAC ? 0 : 4000;
}

UINT rIOAOChannel::getMaxValue() const
{
	return m_regime == Regime::REDUCED_DAC ? 65535 : 24000;
}

UINT rIOAOChannel::getRange() const
{
	return getMaxValue() - getMinValue();
}

UDINT rIOAOChannel::simulate()
{
	return TRITONN_RESULT_OK;
}

UDINT rIOAOChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	UNUSED(issimulate);

	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, false);

	list.add(p + "setup"  , TYPE::UINT, rVariable::Flags::RS__, &m_setup  , U_DIMLESS,         0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(p + "adc"    ,             rVariable::Flags::_SH_, &m_ADC    , U_DIMLESS, ACCESS_SA, "Код АЦП");
	list.add(p + "current",             rVariable::Flags::R___, &m_current, U_mA     ,         0, "Выдаваемый ток");
	list.add(p + "mode"   , TYPE::UINT, rVariable::Flags::R___, &m_mode   , U_DIMLESS,         0, "Тип канала:<br/>" + m_flagsMode.getInfo(true));
	list.add(p + "regime" , TYPE::UINT, rVariable::Flags::RSH_, &m_regime , U_DIMLESS, ACCESS_SA, "Режим канала:<br/>0 : код АЦП<br/>1 : значение в мкА");

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

std::string rIOAOChannel::getMarkDownFlags() const
{
	return m_flagsSetup.getMarkDown(getStrType() + "setup");
}
