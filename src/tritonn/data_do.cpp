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

#include "data_do.h"
#include <string.h>
#include "tinyxml2.h"
#include "event/eid.h"
#include "event/manager.h"
#include "text_id.h"
#include "data_manager.h"
#include "data_snapshot.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "io/manager.h"
#include "io/do_channel.h"
#include "xml_util.h"
#include "generator_md.h"
#include "comment_defines.h"

const UDINT DO_LE_KEYPAD_ON  = 0x00000001;
const UDINT DO_LE_KEYPAD_OFF = 0x00000002;
const UDINT DO_LE_CODE_FAULT = 0x00000004;

rBitsArray rDO::m_flagsMode;
rBitsArray rDO::m_flagsSetup;
rBitsArray rDO::m_flagsStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rDO::rDO(const rStation* owner) : rSource(owner)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS)  , COMMENT::MODE_PHYS)
				.add("KEYPAD", static_cast<UINT>(Mode::KEYPAD), COMMENT::MODE_KEYPAD);
	}

	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"     , static_cast<UINT>(Setup::OFF)        , COMMENT::SETUP_OFF)
				.add("SUCCESS1", static_cast<UINT>(Setup::SUCCESS_ON) , COMMENT::SETUP_SUCCESS1)
				.add("WARNING1", static_cast<UINT>(Setup::WARNING_ON) , COMMENT::SETUP_WARNING1)
				.add("ALARM1"  , static_cast<UINT>(Setup::ALARM_ON)   , COMMENT::SETUP_ALARM1)
				.add("SUCCESS0", static_cast<UINT>(Setup::SUCCESS_OFF), COMMENT::SETUP_SUCCESS0)
				.add("WARNING0", static_cast<UINT>(Setup::WARNING_OFF), COMMENT::SETUP_WARNING0)
				.add("ALARM0"  , static_cast<UINT>(Setup::ALARM_OFF)  , COMMENT::SETUP_ALARM0);
	}

	if (m_flagsStatus.empty()) {
		m_flagsStatus
				.add("", static_cast<UINT>(Status::UNDEF) , COMMENT::STATUS_UNDEF)
				.add("", static_cast<UINT>(Status::OFF)   , COMMENT::STATUS_OFF)
				.add("", static_cast<UINT>(Status::NORMAL), COMMENT::STATUS_NORMAL)
				.add("", static_cast<UINT>(Status::FAULT) , COMMENT::STATUS_FAULT);
	}

	m_lockErr = 0;
	m_setup   = Setup::OFF;
	m_mode    = Mode::PHIS;
	m_status  = Status::UNDEF;

	initLink(rLink::Setup::INOUTPUT | rLink::Setup::WRITABLE,
			 m_present , U_DIMLESS, SID::PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::initLimitEvent(rLink& link)
{
	UNUSED(link);
	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rDO::calculate()
{	
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	// Если аналоговый сигнал выключен, то выходим
	if (m_setup.Value & Setup::OFF) {
		m_physical        = 0;
		m_present.m_value = 0;
		m_mode            = Mode::PHIS;
		m_status          = Status::UNDEF;
		return TRITONN_RESULT_OK;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_setup.Compare(reinitEvent(EID_DO_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	m_status = Status::NORMAL;

	if (m_mode == Mode::PHIS) {
		if (isSetModule()) {
			auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
			auto channel     = static_cast<rIODOChannel*>(channel_ptr);

			if (channel == nullptr) {
				rEventManager::instance().add(reinitEvent(EID_DO_MODULE) << m_module << m_channel);
				rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
				return DATACFGERR_REALTIME_MODULELINK;
			}

			rEvent event_s;
			rEvent event_f;
			checkExpr(channel->m_state, DO_LE_CODE_FAULT,
					  event_f.reinit(EID_DO_CH_FAULT) << m_ID << m_descr,
					  event_s.reinit(EID_DO_CH_OK)    << m_ID << m_descr);

			if (channel->m_state) {
				m_fault  = true;
				m_status = Status::FAULT; // выставляем флаг ошибки
			} else {
				if (m_oldvalue != m_present.m_value) {
					auto module = rIOManager::instance().getModule(m_module);
					rSnapshot ss(rDataManager::instance().getVariableClass());

					m_physical = static_cast<USINT>(m_present.m_value);
					ss.add(module->getAlias() + String_format(".ch_%u.value", channel->m_index), m_physical);
					ss.set();
				}
			}

			if (channel_ptr) {
				delete channel_ptr;
			}
		}
	}

	// Через oldmode делать нельзя, так как нам нужно поймать и ручное переключение
	// можно сделать через m_oldMode, но это не красиво
	if (m_mode == Mode::KEYPAD && !(m_lockErr & DO_LE_KEYPAD_ON)) {
		m_lockErr |=  DO_LE_KEYPAD_ON;
		m_lockErr &= ~DO_LE_KEYPAD_OFF;
		
		rEventManager::instance().add(reinitEvent(EID_DO_KEYPAD_ON));
	}
	
	if (m_mode == Mode::PHIS && !(m_lockErr & DO_LE_KEYPAD_OFF)) {
		m_lockErr |=  DO_LE_KEYPAD_OFF;
		m_lockErr &= ~DO_LE_KEYPAD_ON;
		
		rEventManager::instance().add(reinitEvent(EID_DO_KEYPAD_OFF));
	}

	if (m_present.m_value != m_oldvalue) {
		if (m_present.m_value != 0.0) {
			if (m_setup.Value & Setup::SUCCESS_ON) {
				rEventManager::instance().add(reinitEvent(EID_DO_SUCCESS_ON));
			}

			if (m_setup.Value & Setup::WARNING_ON) {
				rEventManager::instance().add(reinitEvent(EID_DO_WARNING_ON));
			}

			if (m_setup.Value & Setup::ALARM_ON) {
				rEventManager::instance().add(reinitEvent(EID_DO_ALARM_ON));
			}
		} else {
			if (m_setup.Value & Setup::SUCCESS_OFF) {
				rEventManager::instance().add(reinitEvent(EID_DO_SUCCESS_OFF));
			}

			if (m_setup.Value & Setup::WARNING_OFF) {
				rEventManager::instance().add(reinitEvent(EID_DO_WARNING_OFF));
			}

			if (m_setup.Value & Setup::ALARM_OFF) {
				rEventManager::instance().add(reinitEvent(EID_DO_ALARM_OFF));
			}
		}
	}

	m_oldvalue = m_present.m_value;
	
	postCalculate();
		
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(m_alias + ".setup" , TYPE_UINT , rVariable::Flags::RS_D, &m_setup.Value, U_DIMLESS, ACCESS_SA    , COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".mode"  , TYPE_UINT , rVariable::Flags::___D, &m_mode       , U_DIMLESS, ACCESS_KEYPAD, COMMENT::MODE + m_flagsMode.getInfo(true));
	list.add(m_alias + ".status", TYPE_UINT , rVariable::Flags::R___, &m_status     , U_DIMLESS, 0            , COMMENT::STATUS + m_flagsStatus.getInfo());

	list.add(m_alias + ".fault" , TYPE_UDINT, rVariable::Flags::R___, &m_fault      , U_DIMLESS, 0            , COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	auto module_xml = element->FirstChildElement(XmlName::IOLINK);

	if (module_xml) {
		if (rDataModule::loadFromXML(module_xml, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	UDINT fault = 0;

	m_physical = XmlUtils::getTextUSINT(element->FirstChildElement(XmlName::VALUE), 0, fault);
	fault      = 0;

	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));
	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));

	if (fault) {
		return err.set(DATACFGERR_DO, element->GetLineNum(), "");
	}

	m_present.m_limit.m_setup.Init (rLimit::Setup::OFF);

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rDO::generateMarkDown(rGeneratorMD& md)
{
	m_present.m_limit.m_setup.Init(rLimit::Setup::OFF | rLimit::Setup::OPTIONAL);

	md.add(this, true, rGeneratorMD::Type::IOCHANNEL_OPT)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addProperty(XmlName::MODE , &m_flagsMode, true)
			.addXml(XmlName::VALUE, m_present.m_value, true);

	return TRITONN_RESULT_OK;
}

