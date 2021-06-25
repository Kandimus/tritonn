//=================================================================================================
//===
//=== iai_channel.cpp
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

#include "ai_channel.h"
#include <math.h>
#include "xml_util.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../units.h"
#include "../error.h"
#include "../comment_defines.h"

rBitsArray rIOAIChannel::m_flagsSetup;
rBitsArray rIOAIChannel::m_flagsType;
rBitsArray rIOAIChannel::m_flagsSimType;

rIOAIChannel::rIOAIChannel(USINT index, const std::string& comment) : rIOBaseChannel(rIOBaseChannel::Type::AI, index, comment)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(Setup::OFF)    , COMMENT::SETUP_OFF)
				.add("AVERAGE", static_cast<UINT>(Setup::AVERAGE), "Усреднение значения");
	}

	if (m_flagsType.empty()) {
		m_flagsType
				.add("", static_cast<UINT>(Type::mA_0_20) , "0..20мА")
				.add("", static_cast<UINT>(Type::mA_4_20) , "4..40мА")
				.add("", static_cast<UINT>(Type::V_m10_10), "-10..+10V")
				.add("", static_cast<UINT>(Type::V_0_10)  , "0..+10V");
	}

	if (m_flagsSimType.empty()) {
		m_flagsSimType
				.add("", static_cast<UINT>(SimType::NONE)  , COMMENT::SIMTYPE_NONE)
				.add("", static_cast<UINT>(SimType::CONST) , COMMENT::SIMTYPE_CONST)
				.add("", static_cast<UINT>(SimType::LINEAR), COMMENT::SIMTYPE_LINEAR)
				.add("", static_cast<UINT>(SimType::SINUS) , COMMENT::SIMTYPE_SINUS)
				.add("", static_cast<UINT>(SimType::RANDOM), COMMENT::SIMTYPE_RANDOM);
	}
}

UINT rIOAIChannel::getMinValue() const
{
	switch(m_type) {
		case Type::mA_0_20:  return static_cast<UINT>(Scale_mA_0_20::Min);
		case Type::mA_4_20:  return static_cast<UINT>(Scale_mA_4_20::Min);
		case Type::V_m10_10: return static_cast<UINT>(Scale_V_m10_10::Min);
		case Type::V_0_10:   return static_cast<UINT>(Scale_V_0_10::Min);
	}
	return 0;
}

UINT rIOAIChannel::getMaxValue() const
{
	switch(m_type) {
		case Type::mA_0_20:  return static_cast<UINT>(Scale_mA_0_20::Max);
		case Type::mA_4_20:  return static_cast<UINT>(Scale_mA_4_20::Max);
		case Type::V_m10_10: return static_cast<UINT>(Scale_V_m10_10::Max);
		case Type::V_0_10:   return static_cast<UINT>(Scale_V_0_10::Max);
	}
	return 0;
}

UINT rIOAIChannel::getRange() const
{
	return getMaxValue() - getMinValue();
}

UDINT rIOAIChannel::processing()
{
	m_state = false;

	rIOBaseChannel::processing();

	// Изменяем статус
	if (m_ADC < getMinValue() || m_ADC > getMaxValue() || m_hardState) {
		m_state = true;
		m_average.clear();

		//TODO send on to RedLED
		return TRITONN_RESULT_OK;
	}

	if (m_setup & Setup::AVERAGE) {
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

	++m_pullingCount;

	switch(m_simType) {
		case SimType::NONE:
			break;

		case SimType::CONST: {
			m_ADC = m_simValue;
			break;
		}

		case SimType::LINEAR: {
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

		case SimType::SINUS: {
			m_simValue += m_simSpeed;
			if(m_simValue >= 360) {
				m_simValue -= 360;
			}
			LREAL tmp = (sin(static_cast<LREAL>(m_simValue) * 0.017453293) + 1.0) / 2.0;
			tmp   = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * tmp;
			m_ADC = static_cast<UINT>(tmp);
			break;
		}

		case SimType::RANDOM: {
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

UDINT rIOAIChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, issimulate);

	list.add(p + "setup"  , TYPE::UINT , rVariable::Flags::RS__, &m_setup  , U_DIMLESS , 0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(p + "adc"    ,              rVariable::Flags::R___, &m_ADC    , U_DIMLESS , 0, "Текущий код АЦП");
	list.add(p + "current",              rVariable::Flags::R___, &m_current, U_DIMLESS , 0, "Текущее значение тока/напряжения");
	list.add(p + "state"  ,              rVariable::Flags::R___, &m_state  , U_DIMLESS , 0, "Статус канала");
	list.add(p + "type"   , TYPE::USINT, rVariable::Flags::____, &m_type   , U_DIMLESS , 0, "Тип канала:<br/>" + m_flagsType.getInfo(true));

	if (issimulate) {
		list.add(p + "simulate.max"  , rVariable::Flags::____, &m_simMax  , U_DIMLESS , 0, COMMENT::ADC + COMMENT::SIMULATE_MAX);
		list.add(p + "simulate.min"  , rVariable::Flags::____, &m_simMin  , U_DIMLESS , 0, COMMENT::ADC + COMMENT::SIMULATE_MIN);
		list.add(p + "simulate.value", rVariable::Flags::____, &m_simValue, U_DIMLESS , 0, COMMENT::ADC + COMMENT::SIMULATE_VALUE);
		list.add(p + "simulate.speed", rVariable::Flags::____, &m_simSpeed, U_DIMLESS , 0, COMMENT::ADC + COMMENT::SIMULATE_SPEED);
	}

	return TRITONN_RESULT_OK;
}

UDINT rIOAIChannel::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP , "");
	UDINT       fault    = 0;

	m_setup = m_flagsSetup.getValue(strSetup, fault);

	if (fault) {
		return err.set(DATACFGERR_IO_CHANNEL, element->GetLineNum(), "invalid setup");
	}

	return TRITONN_RESULT_OK;
}
