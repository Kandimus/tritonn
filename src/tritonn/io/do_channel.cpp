//=================================================================================================
//===
//=== io_ai_channel.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс канала аналового входного модуля AI (CAN)
//===
//=================================================================================================

#include "do_channel.h"
#include <math.h>
#include "tickcount.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../def_arguments.h"
#include "../units.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"

rBitsArray rIODOChannel::m_flagsSetup;

rIODOChannel::rIODOChannel(USINT index) : rIOBaseChannel(index)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"     , static_cast<UINT>(rIODOChannel::Setup::OFF))
				.add("INVERSED", static_cast<UINT>(rIODOChannel::Setup::INVERSED));
	}
}

UDINT rIODOChannel::processing()
{
	m_state = false;

	if (m_setup & Setup::PULSE) {
		if (m_oldValue != m_value && !m_pulseTimer) {
			m_pulseTimer = rTickCount::SysTick();
		}

		if (rTickCount::SysTick() - m_pulseTimer >= m_pulse) {
			m_pulseTimer = 0;
			m_value      = !m_value;
		}
	}

	m_hardValue = m_value;
	m_oldValue  = m_value;

	if (m_setup & Setup::INVERSED)
	{
		m_hardValue = !m_hardValue;
	}

	return TRITONN_RESULT_OK;
}

UDINT rIODOChannel::simulate()
{
	m_hardState = false;

	++m_pullingCount;

	return TRITONN_RESULT_OK;
}


UDINT rIODOChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, issimulate);

	list.add(p + "setup"  , TYPE_UINT , rVariable::Flags::RS__, &m_setup  , U_DIMLESS , 0);
	list.add(p + "value"  , TYPE_USINT, rVariable::Flags::R___, &m_value  , U_DIMLESS , 0);
	list.add(p + "state"  , TYPE_USINT, rVariable::Flags::R___, &m_state  , U_DIMLESS , 0);

	return TRITONN_RESULT_OK;
}


UDINT rIODOChannel::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP , "");
	UDINT       fault    = 0;

	m_setup = m_flagsSetup.getValue(strSetup, fault);

	if (fault) {
		return err.set(DATACFGERR_IO_CHANNEL, element->GetLineNum(), "invalide setup");
	}

	return TRITONN_RESULT_OK;
}
