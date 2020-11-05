//=================================================================================================
//===
//=== data_limit.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пределов сигнала
//===
//=================================================================================================

#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "tinyxml2.h"
//#include "data_selector.h"
#include "event_eid.h"
#include "event_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_limit.h"
#include "xml_util.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rLimit::rLimit() :
	AMin(0), WMin(0), WMax(0), AMax(0), Hysteresis(0.0)
{
}


rLimit::~rLimit()
{
	;
}


//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
//
UINT rLimit::Calculate(LREAL val, UDINT check)
{
	UINT   oldStatus = Status;

	// Если аналоговый сигнал выключен, то выходим
	if(Setup.Value & LIMIT_SETUP_OFF)
	{
		Status = LIMIT_STATUS_UNDEF;
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	AMin.Compare      (COMPARE_LREAL_PREC, EventChangeAMin);
	WMin.Compare      (COMPARE_LREAL_PREC, EventChangeWMin);
	WMax.Compare      (COMPARE_LREAL_PREC, EventChangeWMax);
	AMax.Compare      (COMPARE_LREAL_PREC, EventChangeAMax);
	Hysteresis.Compare(COMPARE_LREAL_PREC, EventChangeHyst);
	Setup.Compare     (                    EventChangeSetup);

	// Очистим текущий статус
	Status &= ~LIMIT_STATUS_MASK;

	// Проверка на недопустимое число
	if(val == std::numeric_limits<LREAL>::infinity() || std::isnan(val))
	{
		SendEvent(EventNan, nullptr, nullptr, oldStatus & LIMIT_STATUS_NAN);
		Status |= LIMIT_STATUS_NAN;
	}
	else
	{
		if(!check) return Status;

		// Если значение больше чем аварийный максимум
		// ИЛИ
		// Значение больше чем инж. максимум минус дельта И статус уже равен выходу за инж. максимум  (нужно что бы на гестерезисе попасть в эту ветку, а не поймать AMAX)
		else if((Setup.Value & LIMIT_SETUP_AMAX) && ((val > AMax.Value) || ((val > AMax.Value - Hysteresis.Value) && (LIMIT_STATUS_AMAX & oldStatus))))
		{
			SendEvent(EventAMax, &val, &AMax.Value, oldStatus & LIMIT_STATUS_AMAX);
			Status |= LIMIT_STATUS_AMAX;
		}
		else if((Setup.Value & LIMIT_SETUP_AMIN) && ((val < AMin.Value) || ((val < AMin.Value + Hysteresis.Value) && (LIMIT_STATUS_AMIN & oldStatus))))
		{
			SendEvent(EventAMin, &val, &AMin.Value, oldStatus & LIMIT_STATUS_AMIN);
			Status = LIMIT_STATUS_AMIN;
		}
		else if((Setup.Value & LIMIT_SETUP_WMAX) && ((val > WMax.Value) || ((val > WMax.Value - Hysteresis.Value) && (LIMIT_STATUS_WMAX & oldStatus))))
		{
			SendEvent(EventWMax, &val, &WMax.Value, oldStatus & LIMIT_STATUS_WMAX);
			Status = LIMIT_STATUS_WMAX;
		}
		else if((Setup.Value & LIMIT_SETUP_WMIN) && ((val < WMin.Value) || ((val < WMin.Value + Hysteresis.Value) && (LIMIT_STATUS_WMIN & oldStatus))))
		{
			SendEvent(EventWMin, &val, &WMin.Value, oldStatus & LIMIT_STATUS_WMIN);
			Status = LIMIT_STATUS_WMIN;
		}
		else
		{
			SendEvent(EventNormal, &val, nullptr, oldStatus & LIMIT_STATUS_NORMAL);
			Status = LIMIT_STATUS_NORMAL;
		}
	}
	
	return Status;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
UDINT rLimit::generateVars(rVariableList& list, const string &owner_name, STRID owner_unit)
{
	if(Setup.Value & LIMIT_SETUP_OFF) return 0;

	list.add(owner_name + ".LoLo"      , TYPE_LREAL, rVariable::Flags::___L, &AMin.Value      , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".Lo"        , TYPE_LREAL, rVariable::Flags::___L, &WMin.Value      , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".Hi"        , TYPE_LREAL, rVariable::Flags::___L, &WMax.Value      , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".HiHi"      , TYPE_LREAL, rVariable::Flags::___L, &AMax.Value      , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".Hysteresis", TYPE_LREAL, rVariable::Flags::___L, &Hysteresis.Value, owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".status"    , TYPE_UINT , rVariable::Flags::R___, &Status          , U_DIMLESS , 0);
	list.add(owner_name + ".setup"     , TYPE_UINT , rVariable::Flags::RS_L, &Setup.Value     , U_DIMLESS , ACCESS_LIMITS);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLimit::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &/*cfg*/)
{
	string defSetup = rDataConfig::GetFlagNameByBit(rDataConfig::LimitSetupFlags, LIMIT_SETUP_OFF);
	string strSetup = (element->Attribute("setup")) ? element->Attribute("setup") : defSetup.c_str();
	UDINT  err      = 0;

	Setup.Init(rDataConfig::GetFlagFromStr(rDataConfig::LimitSetupFlags, strSetup, err));

	Hysteresis.Init(rDataConfig::GetTextLREAL(element->FirstChildElement(XmlName::HYSTER), 0.0, err));
	AMin.Init      (rDataConfig::GetTextLREAL(element->FirstChildElement(XmlName::LOLO)  , 0.0, err));
	WMin.Init      (rDataConfig::GetTextLREAL(element->FirstChildElement(XmlName::LO)    , 0.0, err));
	WMax.Init      (rDataConfig::GetTextLREAL(element->FirstChildElement(XmlName::HI)    , 0.0, err));
	AMax.Init      (rDataConfig::GetTextLREAL(element->FirstChildElement(XmlName::HIHI)  , 0.0, err));

	return tinyxml2::XML_SUCCESS;
}


//
void rLimit::SendEvent(rEvent &e, LREAL *val, LREAL *lim, UDINT dontsend)
{
	if(dontsend) return;

	rEvent event = e;

	if(val) event << *val;
	if(lim) event << *lim;

	rEventManager::instance().Add(event);
}



