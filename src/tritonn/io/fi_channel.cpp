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

rIOFIChannel::rIOFIChannel()
{
	m_simTimer = rTickCount::SysTick();
	srand(time(NULL));
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

	switch(m_simType) {
		case SimType::None: m_value = m_simValue; return TRITONN_RESULT_OK;

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

	UDINT timer = rTickCount::SysTick();
	UDINT dt    = timer - m_simTimer;
	LREAL fcount = 0;
	UDINT curimp = 0;

	if (m_simTimer) {
//	m_value       = count * (dt + m_simTimerRem) / 1000;
//	m_simTimerRem = dt - m_value * 1000 / m_simValue;
		fcount  = count * dt / 1000.0;
		curimp = floor(fcount + m_simCountRem);
		m_simCountRem = fcount + m_simCountRem - curimp;
	}

	m_value       += curimp;
	m_simTimer    = timer;

printf("timer %u, count/sec %u, count %u, fcount %g, rem %g, value %u\n", timer, count, curimp, fcount, m_simCountRem, m_value);

	return TRITONN_RESULT_OK;
}

