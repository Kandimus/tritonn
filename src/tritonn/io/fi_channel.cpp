//=================================================================================================
//===
//=== fi_channel.cpp
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс частотного канала
//===
//=================================================================================================

#include "fi_channel.h"
#include <math.h>
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "tickcount.h"
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"

rBitsArray rIOFIChannel::m_flagsSetup;

rIOFIChannel::rIOFIChannel(USINT index) :  rIOBaseChannel(rIOBaseChannel::Type::FI, index)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(Setup::OFF))
				.add("AVERAGE", static_cast<UINT>(Setup::AVERAGE));
	}

	m_simTimer = rTickCount::SysTick();
	srand(time(NULL));
}


UDINT rIOFIChannel::processing()
{
	m_state = false;

	// Изменяем статус
	if (m_hardState) {
		m_state = true;

		return TRITONN_RESULT_OK;
	}

	return TRITONN_RESULT_OK;
}


UDINT rIOFIChannel::simulate()
{
	UDINT count = 0;
	m_hardState = false;

	UDINT timer = rTickCount::SysTick();
	if (timer - m_simTimer < 1000) {
		return TRITONN_RESULT_OK;
	}

	++m_pullingCount;

	switch(m_simType) {
		case SimType::None: {
			m_value = 0;
			m_freq  = 0;
			return TRITONN_RESULT_OK;
		}

		case SimType::Const: {
			count = m_simValue;
			break;
		}

		case SimType::Sinus: {
			m_simValue += m_simSpeed;
			if(m_simValue >= 360) {
				m_simValue -= 360;
			}
			LREAL tmp = (sin(static_cast<LREAL>(m_simValue) * 0.017453293) + 1.0) / 2.0;
			tmp   = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * tmp;
			count = static_cast<UINT>(tmp);
			break;
		}

		case SimType::Random: {
			LREAL tmp = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * (rand() / static_cast<LREAL>(RAND_MAX));
			m_simValue = static_cast<UINT>(tmp);
			count      = m_simValue;
			break;
		}
	}

	m_value   += count;
	m_freq     = m_simValue;
	m_simTimer = timer;

	return TRITONN_RESULT_OK;
}

UDINT rIOFIChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, issimulate);

	list.add(p + "setup"  , TYPE_UINT , rVariable::Flags::RS__, &m_setup  , U_DIMLESS , 0);
	list.add(p + "count"  , TYPE_UDINT, rVariable::Flags::R___, &m_value  , U_DIMLESS , 0);
	list.add(p + "state"  , TYPE_USINT, rVariable::Flags::R___, &m_state  , U_DIMLESS , 0);

	if (issimulate) {
		list.add(p + "simulate.max"  , TYPE_UINT, rVariable::Flags::____, &m_simMax  , U_DIMLESS , 0);
		list.add(p + "simulate.min"  , TYPE_UINT, rVariable::Flags::____, &m_simMin  , U_DIMLESS , 0);
		list.add(p + "simulate.value", TYPE_UINT, rVariable::Flags::____, &m_simValue, U_DIMLESS , 0);
		list.add(p + "simulate.speed", TYPE_INT , rVariable::Flags::____, &m_simSpeed, U_DIMLESS , 0);
	}

	return TRITONN_RESULT_OK;
}

UDINT rIOFIChannel::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP , "");
	UDINT       fault    = 0;

	m_setup = m_flagsSetup.getValue(strSetup, fault);

	if (fault) {
		return err.set(DATACFGERR_IO_CHANNEL, element->GetLineNum(), "invalide setup");
	}

	return TRITONN_RESULT_OK;
}
