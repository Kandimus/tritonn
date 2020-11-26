//=================================================================================================
//===
//=== data_ai.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс аналового входного сигнала (AI)
//===
//=================================================================================================

#include <vector>
#include <limits>
#include <cmath>
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
#include "io/ai_channel.h"
#include "xml_util.h"
#include "data_ai.h"

const UDINT AI_LE_SIM_AUTO   = 0x00000002;
const UDINT AI_LE_SIM_MANUAL = 0x00000004;
const UDINT AI_LE_SIM_OFF    = 0x00000008;
const UDINT AI_LE_SIM_LAST   = 0x00000010;
const UDINT AI_LE_CODE_FAULT = 0x00000020;

rBitsArray rAI::m_flagsMode;
rBitsArray rAI::m_flagsSetup;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rAI::rAI() : rSource(), KeypadValue(0.0), m_setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS))
				.add("KEYPAD", static_cast<UINT>(Mode::MKEYPAD));
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , Setup::OFF)
				.add("NOBUFFER" , Setup::NOBUFFER)
				.add("VIRTUAL"  , Setup::VIRTUAL)
				.add("NOICE"    , Setup::NOICE)
				.add("KEYPAD"   , Setup::ERR_KEYPAD)
				.add("LASTGOOD" , Setup::ERR_LASTGOOD);
	}

	LockErr  = 0;
	m_setup  = Setup::OFF;
	m_mode   = Mode::PHIS;
	m_status = Status::UNDEF;

	//NOTE Единицы измерения добавим после загрузки сигнала
	InitLink(rLink::Setup::OUTPUT, m_present, U_any, SID_PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, PhValue  , U_any, SID_PHYSICAL, XmlName::PHYSICAL, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, Current  , U_mA , SID_CURRENT , XmlName::CURRENT , rLink::SHADOW_NONE);
}


rAI::~rAI()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_AI_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_AI_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_AI_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_AI_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_AI_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_AI_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_AI_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_AI_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_AI_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_AI_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_AI_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_AI_NORMAL)    << link.Descr << link.Unit;

	return 0;
}

//-------------------------------------------------------------------------------------------------
//
//UDINT rAI::GetFault(void)
//{
//	if(Setup.Value & AISETUP_VIRTUAL)
//	{
//		return 0;
//	}
	
//	// Нужно получить значения статуса модуля и канала
//	return 0; //TODO доделалать
//}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::Calculate()
{
	LREAL  Range     = 0; // Значение "чистого" диапазона от Min до Max
	Status oldStatus = m_status;
	rEvent event_success;
	rEvent event_fault;
	
	if(rSource::Calculate()) return 0;

	// Если аналоговый сигнал выключен, то выходим
	if(m_setup.Value & rAI::Setup::OFF)
	{
		PhValue.Value   = std::numeric_limits<LREAL>::quiet_NaN();
		m_present.Value = std::numeric_limits<LREAL>::quiet_NaN();
		m_mode          = Mode::PHIS;
		m_status        = Status::UNDEF;
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_scale.Min.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_MIN)      << m_present.Unit);
	m_scale.Max.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_MAX)      << m_present.Unit);
	KeypadValue.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_SIMULATE) << m_present.Unit);
	m_setup.Compare(ReinitEvent(EID_AI_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	// Вычисляем диапазон по инж. величинам (для ускорения расчетов). Вычисляем тут, потому-что это значение еще пригодится при расчете шума
	Range = m_scale.Max.Value - m_scale.Min.Value;

	m_status = rAI::Status::UNDEF;

	//-------------------------------------------------------------------------------------------
	// Преобразуем код АЦП в значение
	if(isSetModule())
	{
		auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
		auto channel     = static_cast<rIOAIChannel*>(channel_ptr.get());

		if (channel == nullptr)
		{
			rEventManager::instance().Add(ReinitEvent(EID_AI_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		CheckExpr(channel->m_state, AI_LE_CODE_FAULT, event_fault.Reinit(EID_AI_CH_FAULT) << ID << Descr, event_success.Reinit(EID_AI_CH_OK) << ID << Descr);

		PhValue.Value = m_scale.Min.Value + static_cast<LREAL>(Range / channel->getRange()) * static_cast<LREAL>(channel->m_ADC - channel->getMinValue());
		Current.Value = channel->m_current; //(24.0 / 65535.0) * static_cast<LREAL>(UsedCode);

		if(channel->m_state)
		{
			Fault = true;

			if(m_mode == Mode::PHIS)
			{
				// если симуляция разрешена, то симулируем этот сигнал
				if(m_setup.Value & Setup::ERR_KEYPAD)
				{
					m_mode = Mode::AKEYPAD;
				}
				else if(m_setup.Value & Setup::ERR_LASTGOOD)
				{
					m_mode = Mode::LASTGOOD;
				}

				m_status = Status::FAULT; // выставляем флаг ошибки
			}

			SetFault();
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
	if(m_mode == Mode::MKEYPAD && !(LockErr & AI_LE_SIM_MANUAL))
	{
		LockErr |= AI_LE_SIM_MANUAL;
		LockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_AUTO | AI_LE_SIM_LAST);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_MANUAL));
	}
	
	if(m_mode == Mode::AKEYPAD && !(LockErr & AI_LE_SIM_AUTO))
	{
		LockErr |= AI_LE_SIM_AUTO;
		LockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_MANUAL | AI_LE_SIM_LAST);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_AUTO));
	}
	
	if(m_mode == Mode::LASTGOOD && !(LockErr & AI_LE_SIM_LAST))
	{
		LockErr |= AI_LE_SIM_LAST;
		LockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_AUTO | AI_LE_SIM_MANUAL);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_LAST));
	}
	
	if(m_mode == Mode::PHIS && !(LockErr & AI_LE_SIM_OFF))
	{
		LockErr |= AI_LE_SIM_OFF;
		LockErr &= ~(AI_LE_SIM_MANUAL | AI_LE_SIM_AUTO | AI_LE_SIM_LAST);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_OFF));
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ВЫХОДНОЕ ЗНАЧЕНИЕ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Если сигнал за засимулирован, то значение равно значению симуляции
	if(m_mode == Mode::MKEYPAD || m_mode == Mode::AKEYPAD)
	{
		m_present.Value = KeypadValue.Value;
	}
	// Если используем последнее "хорошее" значение
	else if(m_mode == Mode::LASTGOOD)
	{
		m_present.Value = m_lastGood;
	}
	// Если используем "физическое" значение
	else if(isSetModule())
	{
		// Получаем значение с датчика
		m_present.Value = PhValue.Value;

		// Проверяем округление 4 и 20мА
		//TODO перенести в Module
		if(m_setup.Value & Setup::NOICE)
		{
			LREAL d = 2.0 * Range / 100.0; // 2% зона нечуствительности

			if(PhValue.Value < m_scale.Min.Value && m_present.Value >= m_scale.Min.Value - d)
			{
				m_present.Value = m_scale.Min.Value;
			}
			if(PhValue.Value > m_scale.Max.Value && m_present.Value <= m_scale.Max.Value + d)
			{
				m_present.Value = m_scale.Max.Value;
			}
		}
	}



	////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ПРЕДЕЛЬНЫЕ ЗНАЧЕНИЯ
	//
	////////////////////////////////////////////////////////////////////////////////////////////////
	if(m_status != Status::FAULT)
	{
		// Если значение больше чем инж. максимум
		// ИЛИ
		// Значение больше чем инж. максимум минус дельта И статус уже равен выходу за инж. максимум  (нужно что бы на гестерезисе попасть в эту ветку, а не поймать AMAX)
		if(m_present.Value > m_scale.Max.Value)
		{
			if(oldStatus != Status::MAX)
			{
				rEventManager::instance().Add(ReinitEvent(EID_AI_MAX) << m_present.Unit << m_present.Value << m_scale.Max.Value);
			}

			m_status = Status::MAX;
		}
		// Инженерный минимум
		else if(m_present.Value < m_scale.Min.Value)
		{
			if(oldStatus != Status::MIN)
			{
				rEventManager::instance().Add(ReinitEvent(EID_AI_MIN) << m_present.Unit << m_present.Value << m_scale.Min.Value);
			}

			m_status = Status::MIN;
		}
		else
		{
			m_status = Status::NORMAL;

			if (m_mode == Mode::PHIS) {
				m_lastGood = m_present.Value;
			}
		}
	}

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	PostCalculate();
		
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rAI::SetFault()
{
	PhValue.Value   = std::numeric_limits<LREAL>::quiet_NaN();
	m_present.Value = std::numeric_limits<LREAL>::quiet_NaN();
	Current.Value   = std::numeric_limits<LREAL>::quiet_NaN();
	m_status        = Status::FAULT;
	Fault           = 1;

	return Fault;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//



///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
UDINT rAI::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".keypad"    , TYPE_LREAL, rVariable::Flags::___L, &KeypadValue.Value, m_present.Unit, ACCESS_KEYPAD);
	list.add(Alias + ".scales.min", TYPE_LREAL, rVariable::Flags::___L, &m_scale.Min.Value, m_present.Unit, ACCESS_SCALES);
	list.add(Alias + ".scales.max", TYPE_LREAL, rVariable::Flags::___L, &m_scale.Max.Value, m_present.Unit, ACCESS_SCALES);
	list.add(Alias + ".setup"     , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value    , U_DIMLESS     , ACCESS_SA);
	list.add(Alias + ".mode"      , TYPE_UINT , rVariable::Flags::___L, &m_mode           , U_DIMLESS     , ACCESS_KEYPAD);
	list.add(Alias + ".status"    , TYPE_UINT , rVariable::Flags::R___, &m_status         , U_DIMLESS     , 0);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault            , U_DIMLESS     , 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));
	UDINT  err      = 0;
	UDINT  result   = TRITONN_RESULT_OK;

	if ((result = rSource::LoadFromXML(element, cfg)) != TRITONN_RESULT_OK) {
		return result;
	}

	tinyxml2::XMLElement* module = element->FirstChildElement(XmlName::IOLINK);
	tinyxml2::XMLElement* limits = element->FirstChildElement(XmlName::LIMITS); // Limits считываем только для проверки
	tinyxml2::XMLElement* unit   = element->FirstChildElement(XmlName::UNIT);
	tinyxml2::XMLElement* scale  = element->FirstChildElement(XmlName::SCALE);

	// Если аналоговый сигнал не привязан к каналу, то разрешаем менять его значение
	if (module) {
		if ((result = rDataModule::loadFromXML(module, cfg)) != TRITONN_RESULT_OK) {
			return result;
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	if(nullptr == limits || nullptr == unit || nullptr == scale)
	{
		return DATACFGERR_AI;
	}

	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, err));

	m_setup.Init(m_flagsSetup.getValue(strSetup, err));

	KeypadValue.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::KEYPAD), 0.0, err));
	m_scale.Min.Init  (XmlUtils::getTextLREAL(scale->FirstChildElement  (XmlName::MIN)   , 0.0, err));
	m_scale.Max.Init  (XmlUtils::getTextLREAL(scale->FirstChildElement  (XmlName::MAX)   , 0.0, err));

	STRID Unit = XmlUtils::getTextUDINT(element->FirstChildElement(XmlName::UNIT), U_any, err);

	if(err)
	{
		return DATACFGERR_AI;
	}

	// Подправляем единицы измерения, исходя из конфигурации AI
	m_present.Unit = Unit;
	PhValue.Unit   = Unit;

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


std::string rAI::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	m_present.Limit.m_setup.Init(rLimit::Setup::NONE);
	PhValue.Limit.m_setup.Init(rLimit::Setup::NONE);
	Current.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}






