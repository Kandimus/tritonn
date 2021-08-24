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
#include "xml_util.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../def_arguments.h"
#include "../units.h"
#include "../error.h"
#include "../comment_defines.h"

rBitsArray rIODOChannel::m_flagsSetup;

rIODOChannel::rIODOChannel(USINT index, const std::string& comment)
	: rIOBaseChannel(rIOBaseChannel::Type::DO, index, comment)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"     , static_cast<UINT>(rIODOChannel::Setup::OFF)     , COMMENT::SETUP_OFF)
				.add("PULSE"   , static_cast<UINT>(rIODOChannel::Setup::PULSE)   , "Выдать одиночный импульс")
				.add("INVERSED", static_cast<UINT>(rIODOChannel::Setup::INVERSED), COMMENT::SETUP_INVERSE);
	}
}

UDINT rIODOChannel::processing()
{
	if (m_setup & Setup::PULSE) {
		if (m_timer.isFinished()) {
			m_timer.stop();
			m_value    = !m_value;
			m_oldValue = m_value;
		}

		if (m_oldValue != m_value) {
			if (!m_timer.isStarted()) {
				m_timer.start(m_pulse);
			}
		} else {
			m_timer.stop();
		}
	} else {
		m_oldValue = m_value;
	}

	m_phValue = (m_setup & Setup::INVERSED) ? !m_value : m_value;

	return TRITONN_RESULT_OK;
}

void rIODOChannel::setValue(bool value)
{
	m_value = value;
}

bool rIODOChannel::simulate()
{
	return true;
}


UDINT rIODOChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, false);

	list.add(p + "setup", TYPE::UINT, rVariable::Flags::RS__, &m_setup, U_DIMLESS , 0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(p + "value",             rVariable::Flags::____, &m_value, U_DIMLESS , 0, COMMENT::VALUE);
	list.add(p + "pulse",             rVariable::Flags::____, &m_pulse, U_msec    , 0, "Длина одиночного импульса");

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

std::string rIODOChannel::getMarkDownFlags() const
{
	return m_flagsSetup.getMarkDown(getStrType() + " setup");
}
