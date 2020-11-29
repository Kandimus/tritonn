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
#include "simpleargs.h"
#include "tickcount.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../def_arguments.h"
#include "../units.h"


UDINT rIODOChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, issimulate);

	list.add(p + "setup"  , TYPE_UINT , rVariable::Flags::RS__, &m_setup  , U_DIMLESS , 0);
	list.add(p + "value"  , TYPE_USINT, rVariable::Flags::R___, &m_value  , U_DIMLESS , 0);
	list.add(p + "state"  , TYPE_USINT, rVariable::Flags::R___, &m_state  , U_DIMLESS , 0);

	return TRITONN_RESULT_OK;
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

	return TRITONN_RESULT_OK;
}

