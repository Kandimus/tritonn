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
#include "data_variable.h"
#include "xml_util.h"
#include "data_ai.h"


/*
	Расчет кодов АПЦ для разных случаев
	Канал   Датчик    Min      Max
	0-22mA  4..20мА   5958		29788
	0-20mA  4..20мА   6553		32767
	±11V    0..10V    0			29788
	±11V    1..10V    2979		29788
	±11V    0..5V     0			14894
	±11V    1..5V     2979		14894
	±10V    0..10V    0			32767
	±10V    1..10V    3277		29788
	±10V    0..5V     0			16384
	±10V    1..5V     3277		16384
*/


const DINT  AI_CODE_FAULT    = -1;

const UDINT AI_LE_SIM_AUTO   = 0x00000002;
const UDINT AI_LE_SIM_MANUAL = 0x00000004;
const UDINT AI_LE_SIM_OFF    = 0x00000008;
const UDINT AI_LE_SIM_LAST   = 0x00000010;
const UDINT AI_LE_CODE_FAULT = 0x00000020;

rBitsArray rAI::m_flagsMode;
rBitsArray rAI::m_flagsSetup;


rAIScale::rAIScale()
{
	Min.Init(0);
	Max.Init(100);

	Code_4mA  = 10923;
	Code_20mA = 54613;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rAI::rAI() : rSource(), KeypadValue(0.0), Setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , AI_MODE_PHIS)
				.add("KEYPAD", AI_MODE_MKEYPAD);
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , AI_SETUP_OFF)
				.add("NOBUFFER" , AI_SETUP_NOBUFFER)
				.add("VIRTUAL"  , AI_SETUP_VIRTUAL)
				.add("NOICE"    , AI_SETUP_NOICE)
				.add("KEYPAD"   , AI_SETUP_ERR_KEYPAD)
				.add("LASTGOOD" , AI_SETUP_ERR_LASTGOOD);
	}

	LockErr  = 0;
	Setup    = AI_SETUP_OFF;
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
	if(Setup.Value & AI_SETUP_OFF)
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
	Scale.Min.Compare  (COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_MIN) << Value.Unit);
	Scale.Max.Compare  (COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_MAX) << Value.Unit);
	KeypadValue.Compare(COMPARE_LREAL_PREC, ReinitEvent(EID_AI_NEW_SIMULATE) << Value.Unit);
	Setup.Compare(ReinitEvent(EID_AI_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	// Вычисляем диапазон по инж. величинам (для ускорения расчетов). Вычисляем тут, потому-что это значение еще пригодится при расчете шума
	Range = Scale.Max.Value - Scale.Min.Value;

	//-------------------------------------------------------------------------------------------
	// Преобразуем код АЦП в значение
	if(!(Setup.Value & AI_SETUP_VIRTUAL))
	{
		if(SetCode)
		{
			Code    =  SetCode & 0x0000FFFF;
			ChFault = (SetCode & 0x00010000) > 0;
		}
		//TODO Получим код статуса канала, после сохраним его основной структуре
		//if(!aidata->pGetCh->status)
		//{
		//	aidata->StatusCh = (aidata->pGetCh->value != 0);
		//}

		CheckExpr(ChFault, AI_LE_CODE_FAULT, event_fault.Reinit(EID_AI_CH_OK) << ID << Descr, event_success.Reinit(EID_AI_CH_OK) << ID << Descr);
			
		//TODO Если считали успешно, то обрабатываем полученный код АЦП
		//if(!aidata->pGetVal->status)
		if(!ChFault)
		{
			// Используем текущий код АЦП
			// UsedCode 32 битное, а Code 16 битное, это сделано для нормального усредения кода ацп ниже.
			UsedCode = Code;

			if(rAI::Status::FAULT == m_status)
			{
				m_status = rAI::Status::UNDEF;
			}

			
			if(!(Setup.Value & AI_SETUP_NOBUFFER))
			{
				// Устредняем по последним "хорошим" значениям
				//TODO Нужно проверять код АЦП, на хорошее значение от 10к до 54к.
				for(ii = 0; ii < MAX_AI_SPLINE; ++ii)
				{
					UsedCode  += ((Spline[ii] == 0xFFFF) ? Code : Spline[ii]);
				}
				UsedCode /= MAX_AI_SPLINE + 1;
			}
		}
		else
		{
			UsedCode = AI_CODE_FAULT;
			Fault    = 1;
		}

		// Расчитываем физическое значение и ток, только в том случае, если код АЦП больше нуля
		if(UsedCode != AI_CODE_FAULT)
		{
			// Для тока 0 мА = 0, 24 мА = 65535
			// Для значения min = 10923, max = 54613
			PhValue.Value = Scale.Min.Value + static_cast<LREAL>(Range / (Scale.Code_20mA - Scale.Code_4mA)) * static_cast<LREAL>(UsedCode - Scale.Code_4mA);
			Current.Value = (24.0 / 65535.0) * static_cast<LREAL>(UsedCode);
		}
		else
		{
			if(Mode == AI_MODE_PHIS)
			{
				// если симуляция разрешена, то симулируем этот сигнал
				if(Setup.Value & AI_SETUP_ERR_KEYPAD)
				{
					Mode = AI_MODE_AKEYPAD;
				}
				else if(Setup.Value & AI_SETUP_ERR_LASTGOOD)
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
		SetFault();
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
	else if(!(Setup.Value & AI_SETUP_VIRTUAL))
	{
		// Получаем значение с датчика
		Value.Value = PhValue.Value;

		// Проверяем округление 4 и 20мА
		if(Setup.Value & AI_SETUP_NOICE)
		{
			LREAL d = 2.0 * Range / 100.0; // 2% зона нечуствительности

			if(PhValue.Value < Scale.Min.Value && Value.Value >= Scale.Min.Value - d)
			{
				Value.Value = Scale.Min.Value;
			}
			if(PhValue.Value > Scale.Max.Value && Value.Value <= Scale.Max.Value + d)
			{
				Value.Value = Scale.Max.Value;
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
		if(Value.Value > Scale.Max.Value)
		{
			if(oldStatus != rAI::Status::MAX)
			{
				rEventManager::instance().Add(ReinitEvent(EID_AI_MAX) << Value.Unit << Value.Value << Scale.Max.Value);
			}

			m_status = rAI::Status::MAX;
		}
		// Инженерный минимум
		else if(Value.Value < Scale.Min.Value)
		{
			if(oldStatus != rAI::Status::MIN)
			{
				rEventManager::instance().Add(ReinitEvent(EID_AI_MIN) << Value.Unit << Value.Value << Scale.Min.Value);
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
UDINT rAI::GenerateVars(vector<rVariable *> &list)
{
	rSource::GenerateVars(list);

	// Variables
	list.push_back(new rVariable(Alias + ".code"            , TYPE_UINT , VARF_R__L, &Code             , U_DIMLESS , 0));
	list.push_back(new rVariable(Alias + ".setcode"         , TYPE_UDINT, VARF_RSHL, &SetCode          , U_DIMLESS , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".keypad"          , TYPE_LREAL, VARF____L, &KeypadValue.Value, Value.Unit, ACCESS_KEYPAD));
	list.push_back(new rVariable(Alias + ".scales.min"      , TYPE_LREAL, VARF____L, &Scale.Min.Value  , Value.Unit, ACCESS_SCALES));
	list.push_back(new rVariable(Alias + ".scales.max"      , TYPE_LREAL, VARF____L, &Scale.Max.Value  , Value.Unit, ACCESS_SCALES));
	list.push_back(new rVariable(Alias + ".scales.code_4mA" , TYPE_UINT , VARF_RS_L, &Scale.Code_4mA   , U_DIMLESS , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".scales.code_20mA", TYPE_UINT , VARF_RS_L, &Scale.Code_20mA  , U_DIMLESS , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".setup"           , TYPE_UINT , VARF_RS_L, &Setup.Value      , U_DIMLESS , ACCESS_SA));
	list.push_back(new rVariable(Alias + ".mode"            , TYPE_UINT , VARF____L, &Mode             , U_DIMLESS , ACCESS_KEYPAD));
	list.push_back(new rVariable(Alias + ".status"          , TYPE_UINT , VARF_R___, &m_status         , U_DIMLESS , 0));

	list.push_back(new rVariable(Alias + ".fault"           , TYPE_UDINT, VARF_R___, &Fault            , U_DIMLESS , 0));

//	Limit.GenerateVars(list, Alias, SSPOINTER(IO.AI[ID].Limit), Unit);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (AI_MODE_PHIS));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(AI_SETUP_OFF));
	UDINT  err      = 0;

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, cfg)) {
		return DATACFGERR_AI;
	}

//	tinyxml2::XMLElement *module     = element->FirstChildElement("module");
	tinyxml2::XMLElement *limits = element->FirstChildElement(XmlName::LIMITS); // Limits считываем только для проверки
	tinyxml2::XMLElement *unit   = element->FirstChildElement(XmlName::UNIT);
	tinyxml2::XMLElement *scale  = element->FirstChildElement(XmlName::SCALE);

	if(nullptr == limits || nullptr == unit || nullptr == scale)
	{
		return DATACFGERR_AI;
	}

	Mode  =    m_flagsMode.getValue (strMode , err);
	Setup.Init(m_flagsSetup.getValue(strSetup, err));

	KeypadValue.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::KEYPAD), 0.0, err));
	Scale.Min.Init  (XmlUtils::getTextLREAL(scale->FirstChildElement  (XmlName::MIN)   , 0.0, err));
	Scale.Max.Init  (XmlUtils::getTextLREAL(scale->FirstChildElement  (XmlName::MAX)   , 0.0, err));

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


UDINT rAI::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Value.Limit.Setup.Init(0);
	PhValue.Limit.Setup.Init(0);
	Current.Limit.Setup.Init(0);

	return rSource::SaveKernel(file, isio, objname, comment, isglobal);
}






