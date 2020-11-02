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
#include "io_manager.h"
#include "io_ai_channel.h"
#include "xml_util.h"
#include "data_ai.h"


const DINT  AI_CODE_FAULT    = -1;

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
				.add("PHIS"  , AI_MODE_PHIS)
				.add("KEYPAD", AI_MODE_MKEYPAD);
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , rAI::Setup::OFF)
				.add("NOBUFFER" , rAI::Setup::NOBUFFER)
				.add("VIRTUAL"  , rAI::Setup::VIRTUAL)
				.add("NOICE"    , rAI::Setup::NOICE)
				.add("KEYPAD"   , rAI::Setup::ERR_KEYPAD)
				.add("LASTGOOD" , rAI::Setup::ERR_LASTGOOD);
	}

	LockErr  = 0;
	m_setup  = rAI::Setup::OFF;
	Mode     = AI_MODE_PHIS;
	Code     = AI_CODE_FAULT;
	m_status = rAI::Status::UNDEF;

	// Все элементы LastCode приравниваем к -1
	memset(Spline, 0xFF, sizeof(Spline[0]) * MAX_AI_SPLINE);

	//NOTE Единицы измерения добавим после загрузки сигнала
	InitLink(LINK_SETUP_OUTPUT, Value  , U_any, SID_PRESENT , XmlName::PRESENT , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT, PhValue, U_any, SID_PHYSICAL, XmlName::PHYSICAL, LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT, Current, U_mA , SID_CURRENT , XmlName::CURRENT , LINK_SHADOW_NONE);
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
	UDINT       ii        = 0;
	LREAL       Range     = 0; // Значение "чистого" диапазона от Min до Max
	rAI::Status oldStatus = m_status;
	rEvent      event_success;
	rEvent      event_fault;
	
	if(rSource::Calculate()) return 0;

	// Если аналоговый сигнал выключен, то выходим
	if(m_setup.Value & rAI::Setup::OFF)
	{
		//lStatusCh = OFAISTATUSCH_OK;		
		Code          = 0xFFFF;
		UsedCode      = AI_CODE_FAULT;
		PhValue.Value = std::numeric_limits<LREAL>::quiet_NaN();
		Value.Value   = std::numeric_limits<LREAL>::quiet_NaN();
		Mode          = AI_MODE_PHIS;
		m_status      = rAI::Status::UNDEF;

		memset(Spline, 0xFF, sizeof(Spline[0]) * MAX_AI_SPLINE);
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_scale.Min.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_MIN)      << Value.Unit);
	m_scale.Max.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_MAX)      << Value.Unit);
	KeypadValue.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_SIMULATE) << Value.Unit);
	m_setup.Compare(ReinitEvent(EID_AI_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	// Вычисляем диапазон по инж. величинам (для ускорения расчетов). Вычисляем тут, потому-что это значение еще пригодится при расчете шума
	Range = m_scale.Max.Value - m_scale.Min.Value;

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

		CheckExpr(channel->m_state, AI_LE_CODE_FAULT, event_fault.Reinit(EID_AI_CH_OK) << ID << Descr, event_success.Reinit(EID_AI_CH_OK) << ID << Descr);
			
		//TODO Если считали успешно, то обрабатываем полученный код АЦП
		if(!channel->m_state)
		{
			// Используем текущий код АЦП
			// UsedCode 32 битное, а Code 16 битное, это сделано для нормального усредения кода ацп ниже.
			UsedCode = channel->m_ADC;

			if(rAI::Status::FAULT == m_status)
			{
				m_status = rAI::Status::UNDEF;
			}

			//TODO Перенести в канал!
			if(!(m_setup.Value & rAI::Setup::NOBUFFER))
			{
				// Устредняем по последним "хорошим" значениям
				for(ii = 0; ii < MAX_AI_SPLINE; ++ii)
				{
					UsedCode  += ((Spline[ii] == 0xFFFF) ? Code : Spline[ii]);
				}
				UsedCode /= MAX_AI_SPLINE + 1;
			}

			PhValue.Value = m_scale.Min.Value + static_cast<LREAL>(Range / channel->getRange()) * static_cast<LREAL>(UsedCode - channel->getMinValue());
			Current.Value = channel->m_current; //(24.0 / 65535.0) * static_cast<LREAL>(UsedCode);
		}
		else
		{
			UsedCode = AI_CODE_FAULT;
			Fault    = 1;

			if(Mode == AI_MODE_PHIS)
			{
				// если симуляция разрешена, то симулируем этот сигнал
				if(m_setup.Value & rAI::Setup::ERR_KEYPAD)
				{
					Mode = AI_MODE_AKEYPAD;
				}
				else if(m_setup.Value & rAI::Setup::ERR_LASTGOOD)
				{
					Mode = AI_MODE_LASTGOOD;
				}

				m_status = rAI::Status::FAULT; // выставляем флаг ошибки
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
	
	if(Mode == AI_MODE_MKEYPAD && !(LockErr & AI_LE_SIM_MANUAL))
	{
		LockErr |= AI_LE_SIM_MANUAL;
		LockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_AUTO | AI_LE_SIM_LAST);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_MANUAL));
	}
	
	if(Mode == AI_MODE_AKEYPAD && !(LockErr & AI_LE_SIM_AUTO))
	{
		LockErr |= AI_LE_SIM_AUTO;
		LockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_MANUAL | AI_LE_SIM_LAST);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_AUTO));
	}
	
	if(Mode == AI_MODE_LASTGOOD && !(LockErr & AI_LE_SIM_LAST))
	{
		LockErr |= AI_LE_SIM_LAST;
		LockErr &= ~(AI_LE_SIM_OFF | AI_LE_SIM_AUTO | AI_LE_SIM_MANUAL);
		
		rEventManager::instance().Add(ReinitEvent(EID_AI_SIM_LAST));
	}
	
	if(Mode == AI_MODE_PHIS && !(LockErr & AI_LE_SIM_OFF))
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
	if(Mode == AI_MODE_MKEYPAD || Mode == AI_MODE_AKEYPAD)
	{
		Value.Value = KeypadValue.Value;
	}
	// Если используем последнее "хорошее" значение
	else if(Mode == AI_MODE_LASTGOOD)
	{
		Value.Value = LastGood;
	}
	// Если используем "физическое" значение, у НЕ виртуального сигнала
	else if(isSetModule())
	{
		// Получаем значение с датчика
		Value.Value = PhValue.Value;

		// Проверяем округление 4 и 20мА
		if(m_setup.Value & rAI::Setup::NOICE)
		{
			LREAL d = 2.0 * Range / 100.0; // 2% зона нечуствительности

			if(PhValue.Value < m_scale.Min.Value && Value.Value >= m_scale.Min.Value - d)
			{
				Value.Value = m_scale.Min.Value;
			}
			if(PhValue.Value > m_scale.Max.Value && Value.Value <= m_scale.Max.Value + d)
			{
				Value.Value = m_scale.Max.Value;
			}
		}
	}



	////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ПРЕДЕЛЬНЫЕ ЗНАЧЕНИЯ
	//
	////////////////////////////////////////////////////////////////////////////////////////////////
	if(m_status != rAI::Status::FAULT)
	{
		// Если значение больше чем инж. максимум
		// ИЛИ
		// Значение больше чем инж. максимум минус дельта И статус уже равен выходу за инж. максимум  (нужно что бы на гестерезисе попасть в эту ветку, а не поймать AMAX)
		if(Value.Value > m_scale.Max.Value)
		{
			if(oldStatus != rAI::Status::MAX)
			{
				rEventManager::instance().Add(ReinitEvent(EID_AI_MAX) << Value.Unit << Value.Value << m_scale.Max.Value);
			}

			m_status = rAI::Status::MAX;
		}
		// Инженерный минимум
		else if(Value.Value < m_scale.Min.Value)
		{
			if(oldStatus != rAI::Status::MIN)
			{
				rEventManager::instance().Add(ReinitEvent(EID_AI_MIN) << Value.Unit << Value.Value << m_scale.Min.Value);
			}

			m_status = rAI::Status::MIN;
		}
		else
		{
			m_status = rAI::Status::NORMAL;
		}
	}

	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ЗАВЕРШАЮЩИЕ РАСЧЕТЫ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
		
	//----------------------------------------------------------------------------------------------
	// Сохраняем последнее "хорошее" значение и массив последних "хороших" кодов АЦП
	// Получается, что если сигнал у нас в ручной симуляции, то "хорошие" значения мы не сохраняем... Хорошо это или плохо?
	//TODO Нужно еще проверять выход за инженерные пределы!
	if(m_status != rAI::Status::FAULT && Mode == AI_MODE_PHIS && UsedCode != AI_CODE_FAULT)
	{
		LastGood = Value.Value;
		
		for(ii = MAX_AI_SPLINE - 1; ii >= 1; --ii)
		{
			Spline[ii] = Spline[ii - 1];
		}
		Spline[0] = Code;
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
	PhValue.Value = std::numeric_limits<LREAL>::quiet_NaN();
	Value.Value   = std::numeric_limits<LREAL>::quiet_NaN();
	Current.Value = std::numeric_limits<LREAL>::quiet_NaN();
	UsedCode      = AI_CODE_FAULT;
	m_status      = rAI::Status::FAULT;
	Fault         = 1;

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
	list.add(Alias + ".code"      , TYPE_UINT , rVariable::Flags::R__L, &Code             , U_DIMLESS , 0);
	list.add(Alias + ".keypad"    , TYPE_LREAL, rVariable::Flags::___L, &KeypadValue.Value, Value.Unit, ACCESS_KEYPAD);
	list.add(Alias + ".scales.min", TYPE_LREAL, rVariable::Flags::___L, &m_scale.Min.Value, Value.Unit, ACCESS_SCALES);
	list.add(Alias + ".scales.max", TYPE_LREAL, rVariable::Flags::___L, &m_scale.Max.Value, Value.Unit, ACCESS_SCALES);
	list.add(Alias + ".setup"     , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value    , U_DIMLESS , ACCESS_SA);
	list.add(Alias + ".mode"      , TYPE_UINT , rVariable::Flags::___L, &Mode             , U_DIMLESS , ACCESS_KEYPAD);
	list.add(Alias + ".status"    , TYPE_UINT , rVariable::Flags::R___, &m_status         , U_DIMLESS , 0);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault            , U_DIMLESS , 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (AI_MODE_PHIS));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(rAI::Setup::OFF));
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
		Value.Setup |= LINK_SETUP_WRITABLE;
	}

	if(nullptr == limits || nullptr == unit || nullptr == scale)
	{
		return DATACFGERR_AI;
	}

	Mode = m_flagsMode.getValue(strMode, err);

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
	Value.Unit   = Unit;
	PhValue.Unit = Unit;

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


UDINT rAI::saveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Value.Limit.Setup.Init(0);
	PhValue.Limit.Setup.Init(0);
	Current.Limit.Setup.Init(0);

	return rSource::saveKernel(file, isio, objname, comment, isglobal);
}






