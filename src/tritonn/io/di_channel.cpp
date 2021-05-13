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
#include "tinyxml2.h"
#include "../error.h"
#include "../xml_util.h"
#include "../comment_defines.h"

rBitsArray rIODIChannel::m_flagsSetup;
rBitsArray rIODIChannel::m_flagsSimType;

rIODIChannel::rIODIChannel(USINT index, const std::string& comment) : rIOBaseChannel(rIOBaseChannel::Type::DI, index, comment)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"     , static_cast<UINT>(rIODIChannel::Setup::OFF)     , COMMENT::SETUP_OFF)
				.add("BOUNCE"  , static_cast<UINT>(rIODIChannel::Setup::BOUNCE)  , "Устранение дребезга")
				.add("INVERSED", static_cast<UINT>(rIODIChannel::Setup::INVERSED), COMMENT::SETUP_INVERSE);
	}

	if (m_flagsSimType.empty()) {
		m_flagsSimType
				.add("", static_cast<UINT>(SimType::NONE)  , COMMENT::SIMTYPE_NONE)
				.add("", static_cast<UINT>(SimType::CONST) , COMMENT::SIMTYPE_CONST)
				.add("", static_cast<UINT>(SimType::PULSE) , "Пульсация сигнала")
				.add("", static_cast<UINT>(SimType::RANDOM), COMMENT::SIMTYPE_RANDOM);
	}

	m_oldValue = m_value;
	m_simTimer = rTickCount::SysTick();
	srand(time(NULL));
}

UDINT rIODIChannel::generateVars(const std::string& name, rVariableList& list, bool issimulate)
{
	std::string p = name + ".";

	rIOBaseChannel::generateVars(name, list, issimulate);

	list.add(p + "setup"  , TYPE_UINT , rVariable::Flags::RS__, &m_setup , U_DIMLESS, 0, COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(p + "value"  ,             rVariable::Flags::R___, &m_value , U_DIMLESS, 0, COMMENT::VALUE);
	list.add(p + "state"  ,             rVariable::Flags::R___, &m_state , U_DIMLESS, 0, COMMENT::STATUS + "Нет данных");
	list.add(p + "bounce" ,             rVariable::Flags::___D, &m_bounce, U_msec   , 0, "Значение таймера антидребезга");

	if (issimulate) {
		list.add(p + "simulate.value", rVariable::Flags::____, &m_simValue, U_DIMLESS, 0, "Значение симулированного значения");
		list.add(p + "simulate.blink", rVariable::Flags::____, &m_simBlink, U_msec   , 0, "Период мигания");
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

	++m_pullingCount;

	switch(m_simType) {
		case SimType::NONE:
			break;

		case SimType::CONST: {
			m_hardValue = m_simValue;
			break;
		}

		case SimType::PULSE: {
			if (rTickCount::SysTick() - m_simTimer >= m_simBlink) {
				m_simValue  = !m_simValue;
				m_hardValue = m_simValue;
				m_simTimer  = rTickCount::SysTick();
			}
			break;
		}

		case SimType::RANDOM: {
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

UDINT rIODIChannel::loadFromXML(tinyxml2::XMLElement* element, rError& err)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP , "");
	UDINT       fault    = 0;

	m_setup = m_flagsSetup.getValue(strSetup, fault);

	if (fault) {
		return err.set(DATACFGERR_IO_CHANNEL, element->GetLineNum(), "invalide setup");
	}

	return TRITONN_RESULT_OK;
}
