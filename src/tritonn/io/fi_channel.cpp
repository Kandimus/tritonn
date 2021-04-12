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
#include "../comment_defines.h"

rBitsArray rIOFIChannel::m_flagsSetup;
rBitsArray rIOFIChannel::m_flagsSimType;

rIOFIChannel::rIOFIChannel(USINT index, const std::string& comment)
	: rIOBaseChannel(rIOBaseChannel::Type::FI, index, comment)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(Setup::OFF)    , COMMENT::SETUP_OFF)
				.add("AVERAGE", static_cast<UINT>(Setup::AVERAGE), "Усреднять значение частоты");
	}

	if (m_flagsSimType.empty()) {
		m_flagsSimType
				.add("", static_cast<UINT>(SimType::NONE)  , COMMENT::SIMTYPE_NONE)
				.add("", static_cast<UINT>(SimType::CONST) , COMMENT::SIMTYPE_CONST)
				.add("", static_cast<UINT>(SimType::LINEAR), COMMENT::SIMTYPE_LINEAR)
				.add("", static_cast<UINT>(SimType::SINUS) , COMMENT::SIMTYPE_SINUS)
				.add("", static_cast<UINT>(SimType::RANDOM), COMMENT::SIMTYPE_RANDOM);
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
		m_average.clear();

		return TRITONN_RESULT_OK;
	}

	if (m_setup & AVERAGE) {
		m_average.push_back(m_freq);

		LREAL sum = 0.0;
		for (auto freq : m_average) {
			sum += freq;
		}
		sum /= m_average.size();

		while (m_average.size() > MAX_AVERAGE) {
			m_average.pop_front();
		}

		m_freq = sum;
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
		case SimType::NONE: {
			m_value = 0;
			m_freq  = 0;
			return TRITONN_RESULT_OK;
		}

		case SimType::CONST: {
			count = m_simValue;
			break;
		}

		case SimType::SINUS: {
			m_simValue += m_simSpeed;
			if(m_simValue >= 360) {
				m_simValue -= 360;
			}
			LREAL tmp = (sin(static_cast<LREAL>(m_simValue) * 0.017453293) + 1.0) / 2.0;
			tmp   = m_simMin + static_cast<LREAL>(m_simMax - m_simMin) * tmp;
			count = static_cast<UINT>(tmp);
			break;
		}

		case SimType::RANDOM: {
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

	list.add(p + "setup"   , TYPE_UINT , rVariable::Flags::RS_, &m_setup  , U_DIMLESS , 0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(p + "count"   ,             rVariable::Flags::R__, &m_value  , U_DIMLESS , 0, "Количество накопленных импульсов");
	list.add(p + "frequecy",             rVariable::Flags::R__, &m_freq   , U_Hz      , 0, "Частота");
	list.add(p + "state"   ,             rVariable::Flags::R__, &m_state  , U_DIMLESS , 0, COMMENT::STATUS + "Нет данных");

	if (issimulate) {
		list.add(p + "simulate.max"  , rVariable::Flags::___, &m_simMax  , U_DIMLESS , 0, COMMENT::IMP_SIM + COMMENT::SIMULATE_MAX);
		list.add(p + "simulate.min"  , rVariable::Flags::___, &m_simMin  , U_DIMLESS , 0, COMMENT::IMP_SIM + COMMENT::SIMULATE_MIN);
		list.add(p + "simulate.value", rVariable::Flags::___, &m_simValue, U_DIMLESS , 0, COMMENT::IMP_SIM + COMMENT::SIMULATE_VALUE);
		list.add(p + "simulate.speed", rVariable::Flags::___, &m_simSpeed, U_DIMLESS , 0, COMMENT::IMP_SIM + COMMENT::SIMULATE_SPEED);
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
