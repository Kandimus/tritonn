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

#include "io_ai_channel.h"
#include <math.h>
#include "variable_item.h"
#include "simpleargs.h"
#include "def_arguments.h"


/*
	Расчет кодов АПЦ для разных случаев
	Канал   Датчик    Min      Max
	0-24мА  4..20мА   10922    54612
			  0..20мА   0        54612
	±11V    0..10V    32767    62556
	±11V    -10..+10V 2979		62556
*/

UINT rIOAIChannel::getMinValue() const
{
	switch(m_type) {
		case Type::mA_0_20:  return 0;
		case Type::mA_4_20:  return 10922;
		case Type::V_0_10:   return 32767;
		case Type::V_m10_10: return 2979;
	}
	return 0;
}

UINT rIOAIChannel::getMaxValue() const
{
	switch(m_type) {
		case Type::mA_0_20:  return 54612;
		case Type::mA_4_20:  return 54612;
		case Type::V_0_10:   return 62556;
		case Type::V_m10_10: return 62556;
	}
	return 0;
}

UINT rIOAIChannel::getRange() const
{
	return getMaxValue() - getMinValue();
}

UDINT rIOAIChannel::generateVars(std::string& name, rVariableList& list)
{
	std::string p = name + ".";

	list.add(p + "setup"  , TYPE_UINT , rVariable::Flags::RS__, &m_setup  , U_DIMLESS , 0);
	list.add(p + "adc"    , TYPE_UINT , rVariable::Flags::R___, &m_ADC    , U_DIMLESS , 0);
	list.add(p + "current", TYPE_REAL , rVariable::Flags::R___, &m_current, U_DIMLESS , 0);
	list.add(p + "state"  , TYPE_USINT, rVariable::Flags::R___, &m_state  , U_DIMLESS , 0);

	if (rSimpleArgs.instance().isSet(rArg::Simulate)) {
		list.add(p + "simulate.max"  , TYPE_UINT, rVariable::Flags::____, &m_simMax  , U_DIMLESS , 0);
		list.add(p + "simulate.min"  , TYPE_UINT, rVariable::Flags::____, &m_simMin  , U_DIMLESS , 0);
		list.add(p + "simulate.value", TYPE_UINT, rVariable::Flags::____, &m_simValue, U_DIMLESS , 0);
		list.add(p + "simulate.speed", TYPE_INT , rVariable::Flags::____, &m_simSpeed, U_DIMLESS , 0);
	}
}

UDINT rIOAIChannel::simulate()
{
	m_state = 0;

	switch(m_simType) {
		case SimType::None:
			return TRITONN_RESULT_OK;

		case SimType::Const: {
			m_ADC = m_simValue;
			return TRITONN_RESULT_OK;
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
			return TRITONN_RESULT_OK;
		}

		case SimType::Sinus: {
			m_simValue += m_simSpeed;
			if(m_simValue >= 360) {
				m_simValue -= 360;
			}
			LREAL tmp = (sin(static_cast<LREAL>(m_simValue) * 0.017453293) + 1.0) / 2.0;
			tmp   = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * tmp;
			m_ADC = static_cast<UINT>(tmp);
			return TRITONN_RESULT_OK;
		}

		case SimType::Random: {
			LREAL tmp = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * (rand() / static_cast<LREAL>(RAND_MAX));
			m_ADC = static_cast<UINT>(tmp);
			return TRITONN_RESULT_OK;
		}
	}

	return TRITONN_RESULT_OK;
}

