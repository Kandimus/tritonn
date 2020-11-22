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
#include "simpleargs.h"
#include <math.h>
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

	// Изменяем статус
	if (m_ADC < getMinValue() || m_ADC > getMaxValue() || m_hardState) {
		m_state = true;
		m_average.clear();

		//TODO send on to RedLED
		return TRITONN_RESULT_OK;
	}

	if (m_setup & AVERAGE) {
		m_average.push_back(m_ADC);

		LREAL sum = 0.0;
		for (auto adc : m_average) {
			sum += static_cast<LREAL>(adc);
		}
		sum /= m_average.size();

		while (m_average.size() > MAX_AVERAGE) {
			m_average.pop_front();
		}

		m_ADC = static_cast<UINT>(sum);
	} else {
		m_average.clear();
	}

	return TRITONN_RESULT_OK;
}

UDINT rIOAIChannel::simulate()
{
	m_hardState = false;

	switch(m_simType) {
		case SimType::None:
			break;

		case SimType::Const: {
			m_ADC = m_simValue;
			break;
		}

		case SimType::Linear: {
			DINT tmp = m_simValue + m_simSpeed;

			if (m_simSpeed > 0) {
				if(tmp >= m_simMax) {
					tmp = m_simMin;
				}
			} else if (m_simSpeed < 0) {
				if (tmp <= m_simMin) {
					tmp = m_simMax;
				}
			}
			m_ADC = m_simValue = static_cast<UINT>(tmp);
			break;
		}

		case SimType::Sinus: {
			m_simValue += m_simSpeed;
			if(m_simValue >= 360) {
				m_simValue -= 360;
			}
			LREAL tmp = (sin(static_cast<LREAL>(m_simValue) * 0.017453293) + 1.0) / 2.0;
			tmp   = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * tmp;
			m_ADC = static_cast<UINT>(tmp);
			break;
		}

		case SimType::Random: {
			LREAL tmp = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * (rand() / static_cast<LREAL>(RAND_MAX));
			m_ADC = static_cast<UINT>(tmp);
			break;
		}
	}

	// simulate current value
	switch (m_type) {
		case Type::mA_0_20:  m_current = 20.0f / getRange() * static_cast<REAL>(m_ADC); break;
		case Type::mA_4_20:  m_current = 4.0f + 16.0f / getRange() * static_cast<REAL>(m_ADC - getMinValue()); break;
		case Type::V_m10_10: m_current = -10.0f + 20.0f / getRange() * static_cast<REAL>(m_ADC - getMinValue()); break;
		case Type::V_0_10:   m_current = 10.0f / getRange() * static_cast<REAL>(m_ADC - getMinValue()); break;
	}

	return TRITONN_RESULT_OK;
}

