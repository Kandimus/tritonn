//=================================================================================================
//===
//=== io_di_channel.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс дискретного входного канала
//===
//=================================================================================================

#include "di_channel.h"
#include <math.h>
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "tickcount.h"

rIODIChannel::rIODIChannel()
{
	m_oldValue = m_value;
	m_simTimer = rTickCount::SysTick();
	srand(time(NULL));
}

UDINT rIODIChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, issimulate);

	list.add(p + "setup"  , TYPE_UINT , rVariable::Flags::RS__, &m_setup , U_DIMLESS, 0);
	list.add(p + "value"  , TYPE_USINT, rVariable::Flags::R___, &m_value , U_DIMLESS, 0);
	list.add(p + "state"  , TYPE_USINT, rVariable::Flags::R___, &m_state , U_DIMLESS, 0);
	list.add(p + "bounce" , TYPE_UDINT, rVariable::Flags::____, &m_bounce, U_msec   , 0);

	if (issimulate) {
		list.add(p + "simulate.value", TYPE_USINT, rVariable::Flags::____, &m_simValue, U_DIMLESS, 0);
		list.add(p + "simulate.blink", TYPE_UDINT, rVariable::Flags::____, &m_simBlink, U_msec   , 0);
	}

	return TRITONN_RESULT_OK;
}

UDINT rIODIChannel::processing()
{
	m_state = false;

	// Изменяем статус
	if (m_hardState) {
		m_state = true;

		//TODO send on to RedLED
		return TRITONN_RESULT_OK;
	}

	if (m_setup & Setup::BOUNCE) {
		if (m_oldValue == m_hardValue) {
			m_bounceTimer = 0;
			m_value       = m_hardValue;
		} else {
			if (!m_bounceTimer) {
				m_bounceTimer = rTickCount::SysTick();
			} else if (rTickCount::SysTick() - m_bounceTimer >= m_bounce) {
				m_bounceTimer = 0;
				m_oldValue    = m_hardValue;
				m_value       = m_hardValue;
			}
		}
	} else {
		m_value = m_hardValue;
	}

	if (m_setup & INVERSED) {
		m_value = !m_value;
	}

	return TRITONN_RESULT_OK;
}

UDINT rIODIChannel::simulate()
{
	m_hardState = false;

	switch(m_simType) {
		case SimType::None:
			break;

		case SimType::Const: {
			m_hardValue = m_simValue;
			break;
		}

		case SimType::Pulse: {
			if (rTickCount::SysTick() - m_simTimer >= m_simBlink) {
				m_simValue  = !m_simValue;
				m_hardValue = m_simValue;
				m_simTimer  = rTickCount::SysTick();
			}
			break;
		}

		case SimType::Random: {
			if (rTickCount::SysTick() - m_simTimer >= m_simBlink) {
				m_simValue  = true == (rand() & 1);
				m_hardValue = m_simValue;
				m_simTimer  = rTickCount::SysTick();
			}
			break;
		}
	}

	return TRITONN_RESULT_OK;
}

