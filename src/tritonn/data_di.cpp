//=================================================================================================
//===
//=== data_di.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс дискретного входного сигнала (DI)
//===
//=================================================================================================

#include "data_di.h"
#include <string.h>
#include "tinyxml2.h"
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "io/manager.h"
#include "io/di_channel.h"
#include "xml_util.h"

const UDINT DI_LE_KEYPAD_ON  = 0x00000001;
const UDINT DI_LE_KEYPAD_OFF = 0x00000002;
const UDINT DI_LE_CODE_FAULT = 0x00000004;

rBitsArray rDI::m_flagsMode;
rBitsArray rDI::m_flagsSetup;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rDI::rDI(const rStation* owner) : rSource(owner), m_keypadValue(0), m_setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS))
				.add("KEYPAD", static_cast<UINT>(Mode::KEYPAD));
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"     , static_cast<UINT>(Setup::OFF))
				.add("KEYPAD"  , static_cast<UINT>(Setup::ERR_KEYPAD))
				.add("SUCCESS1", static_cast<UINT>(Setup::SUCCESS_ON))
				.add("SUCCESS0", static_cast<UINT>(Setup::SUCCESS_OFF))
				.add("WARNING1", static_cast<UINT>(Setup::WARNING_ON))
				.add("WARNING0", static_cast<UINT>(Setup::WARNING_OFF))
				.add("ALARM1"  , static_cast<UINT>(Setup::ALARM_ON))
				.add("ALARM0"  , static_cast<UINT>(Setup::ALARM_OFF));
	}

	m_lockErr = 0;
	m_setup   = Setup::OFF;
	m_mode    = Mode::PHIS;
	m_status  = Status::UNDEF;

	initLink(rLink::Setup::OUTPUT, m_present , U_discrete, SID::PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_physical, U_discrete, SID::PHYSICAL, XmlName::PHYSICAL, rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDI::initLimitEvent(rLink& link)
{
	UNUSED(link);
	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rDI::calculate()
{
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	// Если аналоговый сигнал выключен, то выходим
	if (m_setup.Value & Setup::OFF) {
		m_physical.m_value = 0;
		m_present.m_value  = 0;
		m_mode             = Mode::PHIS;
		m_status           = Status::UNDEF;
		return TRITONN_RESULT_OK;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_keypadValue.Compare(reinitEvent(EID_DI_NEW_SIMULATE));
	m_setup.Compare(reinitEvent(EID_DI_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	m_status = Status::UNDEF;

	//-------------------------------------------------------------------------------------------
	// Преобразуем код АЦП в значение
	if (isSetModule()) {
		auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
		auto channel     = static_cast<rIODIChannel*>(channel_ptr.get());

		if (channel == nullptr) {
			rEventManager::instance().Add(reinitEvent(EID_DI_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		rEvent event_s;
		rEvent event_f;
		checkExpr(channel->m_state, DI_LE_CODE_FAULT,
				  event_f.Reinit(EID_DI_CH_FAULT) << m_ID << m_descr,
				  event_s.Reinit(EID_DI_CH_OK)    << m_ID << m_descr);

		m_physical.m_value = channel->getValue();
		m_status           = Status::NORMAL;

		if (channel->m_state) {
			m_fault = true;

			if (m_mode == Mode::PHIS) {
				// если симуляция разрешена, то симулируем этот сигнал
				if (m_setup.Value & Setup::ERR_KEYPAD) {
					m_mode = Mode::KEYPAD;
				}

				m_status = Status::FAULT; // выставляем флаг ошибки
			}

			setFault();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// РЕЖИМЫ РАБОТЫ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Через oldmode делать нельзя, так как нам нужно поймать и ручное переключение
	// можно сделать через m_oldMode, но это не красиво
	if (m_mode == Mode::KEYPAD && !(m_lockErr & DI_LE_KEYPAD_ON)) {
		m_lockErr |= DI_LE_KEYPAD_ON;
		m_lockErr &= ~DI_LE_KEYPAD_OFF;
		
		rEventManager::instance().Add(reinitEvent(EID_DI_KEYPAD_ON));
	}
	
	if (m_mode == Mode::PHIS && !(m_lockErr & DI_LE_KEYPAD_OFF)) {
		m_lockErr |= DI_LE_KEYPAD_OFF;
		m_lockErr &= ~DI_LE_KEYPAD_ON;
		
		rEventManager::instance().Add(reinitEvent(EID_DI_KEYPAD_OFF));
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ВЫХОДНОЕ ЗНАЧЕНИЕ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	LREAL oldvalue = m_present.m_value;

	// Если сигнал за засимулирован, то значение равно значению симуляции
	if (m_mode == Mode::KEYPAD) {
		m_present.m_value = m_keypadValue.Value;

	// Если используем "физическое" значение
	} else if (isSetModule()) {
		m_present.m_value = m_physical.m_value;
	}

	if (m_present.m_value != oldvalue && m_present.m_value != 0.0) {
		if (m_setup.Value & Setup::SUCCESS_ON) {
			rEventManager::instance().Add(reinitEvent(EID_DI_SUCCESS_ON));
		}

		if (m_setup.Value & Setup::WARNING_ON) {
			rEventManager::instance().Add(reinitEvent(EID_DI_WARNING_ON));
		}

		if (m_setup.Value & Setup::ALARM_ON) {
			rEventManager::instance().Add(reinitEvent(EID_DI_ALARM_ON));
		}
	}

	if (m_present.m_value != oldvalue && m_present.m_value == 0) {
		if (m_setup.Value & Setup::SUCCESS_OFF) {
			rEventManager::instance().Add(reinitEvent(EID_DI_SUCCESS_OFF));
		}

		if (m_setup.Value & Setup::WARNING_OFF) {
			rEventManager::instance().Add(reinitEvent(EID_DI_WARNING_OFF));
		}

		if (m_setup.Value & Setup::ALARM_OFF) {
			rEventManager::instance().Add(reinitEvent(EID_DI_ALARM_OFF));
		}
	}

	postCalculate();
		
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rDI::setFault()
{
	m_physical.m_value = 0;
	m_present.m_value  = 0;
	m_status           = Status::FAULT;
	m_fault            = true;

	return m_fault;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDI::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(m_alias + ".keypad", TYPE_USINT, rVariable::Flags::___L, &m_keypadValue.Value, U_DIMLESS, ACCESS_KEYPAD);
	list.add(m_alias + ".setup" , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value      , U_DIMLESS, ACCESS_SA);
	list.add(m_alias + ".mode"  , TYPE_UINT , rVariable::Flags::___L, &m_mode             , U_DIMLESS, ACCESS_KEYPAD);
	list.add(m_alias + ".status", TYPE_UINT , rVariable::Flags::R___, &m_status           , U_DIMLESS, 0);

	list.add(m_alias + ".fault" , TYPE_UDINT, rVariable::Flags::R___, &m_fault            , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDI::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* module = element->FirstChildElement(XmlName::IOLINK);

	if (module) {
		if (rDataModule::loadFromXML(module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	UDINT fault = 0;
	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));
	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));
	m_keypadValue.Init(XmlUtils::getTextBOOL(element->FirstChildElement(XmlName::KEYPAD), false, fault));

	if (fault) {
		return err.set(DATACFGERR_DI, element->GetLineNum(), "fault mode or setup or keypad");
	}

	m_physical.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_present.m_limit.m_setup.Init (rLimit::Setup::OFF);

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rDI::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	UNUSED(isio);

	m_physical.m_limit.m_setup.Init(rLimit::Setup::OFF);
	m_present.m_limit.m_setup.Init (rLimit::Setup::OFF);

	return rSource::saveKernel(true, objname, comment, isglobal);
}


