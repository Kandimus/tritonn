/*
 *
 * data/ai.cpp
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "xml_util.h"
#include "../error.h"
#include "../text_id.h"
#include "../event/eid.h"
#include "../event/manager.h"
#include "../data_manager.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../io/manager.h"
#include "../io/aiinterface.h"
#include "../generator_md.h"
#include "../comment_defines.h"
#include "ai.h"

const UDINT AI_LE_SIM_AUTO   = 0x00000002;
const UDINT AI_LE_SIM_MANUAL = 0x00000004;
const UDINT AI_LE_SIM_OFF    = 0x00000008;
const UDINT AI_LE_SIM_LAST   = 0x00000010;
const UDINT AI_LE_CODE_FAULT = 0x00000020;

rBitsArray rAI::m_flagsMode;
rBitsArray rAI::m_flagsSetup;
rBitsArray rAI::m_flagsStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rAI::rAI(const rStation* owner) : rSource(owner), m_keypad(0.0), m_setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS)    , COMMENT::MODE_PHYS)
				.add("KEYPAD", static_cast<UINT>(Mode::MKEYPAD) , COMMENT::MODE_KEYPAD)
				.add(""      , static_cast<UINT>(Mode::LASTGOOD), "Используется последнее действительное значение")
				.add(""      , static_cast<UINT>(Mode::AKEYPAD) , "Автоматическое переключение на ручной ввод");
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , static_cast<UINT>(Setup::OFF)         , COMMENT::SETUP_OFF)
				.add("KEYPAD"   , static_cast<UINT>(Setup::ERR_KEYPAD)  , COMMENT::SETUP_KEYPAD)
				.add("LASTGOOD" , static_cast<UINT>(Setup::ERR_LASTGOOD), "Переключение в последнее действительное значение при недействительном значении")
				.add("NOICE"    , static_cast<UINT>(Setup::NOICE)       , "Подавление дребезга около инженерных уставок");
	}

	if (m_flagsStatus.empty()) {
		m_flagsStatus
				.add("", static_cast<UINT>(Status::UNDEF) , COMMENT::STATUS_UNDEF)
				.add("", static_cast<UINT>(Status::OFF)   , COMMENT::STATUS_OFF)
				.add("", static_cast<UINT>(Status::NORMAL), COMMENT::STATUS_NORMAL)
				.add("", static_cast<UINT>(Status::MIN)   , "Значение ниже инженерного минимума")
				.add("", static_cast<UINT>(Status::MAX)   , "Значение выше инженерного максимума")
				.add("", static_cast<UINT>(Status::FAULT) , COMMENT::STATUS_FAULT);
	}

	m_lockErr = 0;
	m_setup   = Setup::OFF;
	m_mode    = Mode::PHIS;
	m_status  = Status::UNDEF;

	//NOTE Единицы измерения добавим после загрузки сигнала
	initLink(rLink::Setup::OUTPUT | rLink::Setup::MUSTVIRT,
								   m_present, U_any, SID::PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_phValue, U_any, SID::PHYSICAL, XmlName::PHYSICAL, rLink::SHADOW_NONE);
	initLink(rLink::Setup::OUTPUT, m_current, U_mA , SID::CURRENT , XmlName::CURRENT , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_AI_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_AI_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_AI_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_AI_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_AI_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_AI_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_AI_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_AI_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_AI_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_AI_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_AI_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_AI_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}

UDINT rAI::calculate()
{
	Status oldStatus = m_status;
	rEvent event_s;
	rEvent event_f;
	
	if(rSource::calculate()) return TRITONN_RESULT_OK;

	// Если аналоговый сигнал выключен, то выходим
	if(m_setup.Value & rAI::Setup::OFF)
	{
		m_phValue.m_value = std::numeric_limits<LREAL>::quiet_NaN();
		m_present.m_value = std::numeric_limits<LREAL>::quiet_NaN();
		m_mode            = Mode::PHIS;
		m_status          = Status::UNDEF;
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_scale.m_min.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_AI_NEW_MIN)      << m_present.m_unit);
	m_scale.m_max.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_AI_NEW_MAX)      << m_present.m_unit);
	m_keypad.Compare     (COMPARE_LREAL_PREC, reinitEvent(EID_AI_NEW_SIMULATE) << m_present.m_unit);
	m_setup.Compare(reinitEvent(EID_AI_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	m_status = rAI::Status::UNDEF;

	//-------------------------------------------------------------------------------------------
	// Преобразуем код АЦП в значение
	if (isSetModule()) {
		auto interface = dynamic_cast<rIOAIInterface*>(rIOManager::instance().getModuleInterface(m_module, rIOBaseModule::Type::UNDEF));

		if (!interface) {
			rEventManager::instance().add(reinitEvent(EID_AI_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		UDINT fault    = TRITONN_RESULT_OK;
		USINT state    = interface->getState   (m_channel, rIOBaseChannel::Type::AI, fault);
		UDINT adc      = interface->getValue   (m_channel, rIOBaseChannel::Type::AI, fault);
		UINT  rangeADC = interface->getRange   (m_channel, rIOBaseChannel::Type::AI, fault);
		UINT  minADC   = interface->getMinValue(m_channel, rIOBaseChannel::Type::AI, fault);

		m_phValue.m_value = m_scale.m_min.Value + static_cast<LREAL>(m_scale.getRange() / rangeADC) * static_cast<LREAL>(adc - minADC);
		m_current.m_value = interface->getCurrent(m_channel, rIOBaseChannel::Type::AI, fault);

		if (fault != TRITONN_RESULT_OK) {
			rEventManager::instance().add(reinitEvent(EID_AI_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, fault);
			return fault;
		}

		checkExpr(state, AI_LE_CODE_FAULT,
				  event_f.reinit(EID_AI_CH_FAULT) << m_ID << m_descr,
				  event_s.reinit(EID_AI_CH_OK)    << m_ID << m_descr);

		if (state) {
			m_fault = true;

			if (m_mode == Mode::PHIS) {
				// если симуляция разрешена, то симулируем этот сигнал
				if(m_setup.Value & Setup::ERR_KEYPAD) {
					m_mode = Mode::AKEYPAD;
				} else if(m_setup.Value & Setup::ERR_LASTGOOD) {
					m_mode = Mode::LASTGOOD;
				}

				m_status = Status::FAULT; // выставляем флаг ошибки
			}

			setFault();
		}
	}
	else //if !virtual
	{
		//aidata->StatusCh = OFAISTATUSCH_OK;
		//SetFault();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// РЕЖИМЫ РАБОТЫ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Через oldmode делать нельзя, так как нам нужно поймать и ручное переключение
	// можно сделать через m_oldMode, но это не красиво
	if(m_mode == Mode::MKEYPAD && !(m_lockErr & AI_LE_SIM_MANUAL))
	{
		m_lockErr |= AI_LE_SIM_MANUAL;
		m_lockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_AUTO | AI_LE_SIM_LAST);
		
		rEventManager::instance().add(reinitEvent(EID_AI_SIM_MANUAL));
	}
	
	if(m_mode == Mode::AKEYPAD && !(m_lockErr & AI_LE_SIM_AUTO))
	{
		m_lockErr |= AI_LE_SIM_AUTO;
		m_lockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_MANUAL | AI_LE_SIM_LAST);
		
		rEventManager::instance().add(reinitEvent(EID_AI_SIM_AUTO));
	}
	
	if(m_mode == Mode::LASTGOOD && !(m_lockErr & AI_LE_SIM_LAST))
	{
		m_lockErr |= AI_LE_SIM_LAST;
		m_lockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_AUTO | AI_LE_SIM_MANUAL);
		
		rEventManager::instance().add(reinitEvent(EID_AI_SIM_LAST));
	}
	
	if(m_mode == Mode::PHIS && !(m_lockErr & AI_LE_SIM_OFF))
	{
		m_lockErr |= AI_LE_SIM_OFF;
		m_lockErr &= ~(AI_LE_SIM_MANUAL | AI_LE_SIM_AUTO | AI_LE_SIM_LAST);
		
		rEventManager::instance().add(reinitEvent(EID_AI_SIM_OFF));
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ВЫХОДНОЕ ЗНАЧЕНИЕ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Если сигнал за засимулирован, то значение равно значению симуляции
	if (m_mode == Mode::MKEYPAD || m_mode == Mode::AKEYPAD) {
		m_present.m_value = m_keypad.Value;
	}
	// Если используем последнее "хорошее" значение
	else if (m_mode == Mode::LASTGOOD) {
		m_present.m_value = m_lastGood;
	}
	// Если используем "физическое" значение
	else if (isSetModule()) {
		// Получаем значение с датчика
		m_present.m_value = m_phValue.m_value;

		// Проверяем округление 4 и 20мА
		//TODO перенести в Module
		if(m_setup.Value & Setup::NOICE)
		{
			LREAL d = 2.0 * m_scale.getRange() / 100.0; // 2% зона нечуствительности

			if(m_phValue.m_value < m_scale.m_min.Value && m_present.m_value >= m_scale.m_min.Value - d)
			{
				m_present.m_value = m_scale.m_min.Value;
			}
			if(m_phValue.m_value > m_scale.m_max.Value && m_present.m_value <= m_scale.m_max.Value + d)
			{
				m_present.m_value = m_scale.m_max.Value;
			}
		}
	}

	//---------------------------------------------------------------------------------------------
	// ПРЕДЕЛЬНЫЕ ЗНАЧЕНИЯ
	if(m_status != Status::FAULT)
	{
		// Если значение больше чем инж. максимум
		// ИЛИ
		// Значение больше чем инж. максимум минус дельта И статус уже равен выходу за инж. максимум  (нужно что бы на гестерезисе попасть в эту ветку, а не поймать AMAX)
		if (m_present.m_value > m_scale.m_max.Value) {
			if (oldStatus != Status::MAX) {
				rEventManager::instance().add(reinitEvent(EID_AI_MAX) << m_present.m_unit << m_present.m_value << m_scale.m_max.Value);
			}

			m_status = Status::MAX;
		}
		// Инженерный минимум
		else if (m_present.m_value < m_scale.m_min.Value) {
			if(oldStatus != Status::MIN) {
				rEventManager::instance().add(reinitEvent(EID_AI_MIN) << m_present.m_unit << m_present.m_value << m_scale.m_min.Value);
			}

			m_status = Status::MIN;
		}
		else
		{
			m_status = Status::NORMAL;

			if (m_mode == Mode::PHIS) {
				m_lastGood = m_present.m_value;
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rAI::setFault()
{
	m_phValue.m_value = std::numeric_limits<LREAL>::quiet_NaN();
	m_present.m_value = std::numeric_limits<LREAL>::quiet_NaN();
	m_current.m_value = std::numeric_limits<LREAL>::quiet_NaN();
	m_status          = Status::FAULT;
	m_fault           = 1;

	return m_fault;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rAI::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	m_scale.generateVars(list, m_alias, m_present.m_unit);

	// Variables
	list.add(m_alias + ".keypad"    ,             rVariable::Flags::___D, &m_keypad.Value   , m_present.m_unit, ACCESS_KEYPAD, COMMENT::KEYPAD);
	list.add(m_alias + ".setup"     , TYPE::UINT, rVariable::Flags::RS__, &m_setup.Value    , U_DIMLESS       , ACCESS_SA    , COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".mode"      , TYPE::UINT, rVariable::Flags::___D, &m_mode           , U_DIMLESS       , ACCESS_KEYPAD, COMMENT::MODE + m_flagsMode.getInfo(true));
	list.add(m_alias + ".status"    , TYPE::UINT, rVariable::Flags::R___, &m_status         , U_DIMLESS       , 0            , COMMENT::STATUS + m_flagsStatus.getInfo());

	list.add(m_alias + ".fault"     ,             rVariable::Flags::R___, &m_fault          , U_DIMLESS       , 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);
	tinyxml2::XMLElement* xml_unit   = element->FirstChildElement(XmlName::UNIT);

	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	if (!xml_unit) {
		return err.set(DATACFGERR_AI, element->GetLineNum(), "cant found unit");
	}

	UDINT fault = 0;
	STRID Unit  = XmlUtils::getTextUDINT(element->FirstChildElement(XmlName::UNIT), U_any, fault);

	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));
	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));
	m_keypad.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::KEYPAD) , 0.0, fault));

	if (fault) {
		return err.set(DATACFGERR_AI, element->GetLineNum(), "error in mode or setup or keypad");
	}

	m_scale.loadFromXml(element, err);
	if (err.getError()) {
		return err.getError();
	}

	m_present.m_unit = Unit;
	m_phValue.m_unit = Unit;

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rAI::generateMarkDown(rGeneratorMD& md)
{
	m_present.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_phValue.m_limit.m_setup.Init(LIMIT_SETUP_ALL);
	m_current.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::IOCHANNEL_OPT)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addProperty(XmlName::MODE , &m_flagsMode, true)
			.addXml(XmlName::UNIT  , static_cast<UDINT>(U_any))
			.addXml(XmlName::KEYPAD, m_keypad.Value)
			.addXml("<" + std::string(XmlName::SCALE) + ">")
			.addXml(XmlName::MIN, m_scale.m_min.Value, false, "\t")
			.addXml(XmlName::MAX, m_scale.m_max.Value, false, "\t")
			.addXml("</" + std::string(XmlName::SCALE) + ">");

	return TRITONN_RESULT_OK;
}
