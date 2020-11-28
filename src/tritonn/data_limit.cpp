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

rBitsArray rLimit::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rLimit::rLimit() :
	m_lolo(0), m_lo(0), m_hi(0), m_hihi(0), Hysteresis(0.0)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"  , static_cast<UINT>(Setup::OFF))
				.add("LOLO" , static_cast<UINT>(Setup::LOLO))
				.add("LO"   , static_cast<UINT>(Setup::LO))
				.add("HI"   , static_cast<UINT>(Setup::HI))
				.add("HIHI" , static_cast<UINT>(Setup::HIHI));
	}
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
	Status oldStatus = m_status;

	// Если лимиты выключены, то выходим
	if(m_setup.Value & Setup::OFF) {
		m_status = Status::UNDEF;
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_lolo.Compare    (COMPARE_LREAL_PREC, EventChangeAMin);
	m_lo.Compare      (COMPARE_LREAL_PREC, EventChangeWMin);
	m_hi.Compare      (COMPARE_LREAL_PREC, EventChangeWMax);
	m_hihi.Compare    (COMPARE_LREAL_PREC, EventChangeAMax);
	Hysteresis.Compare(COMPARE_LREAL_PREC, EventChangeHyst);
	m_setup.Compare   (                    EventChangeSetup);

	// Проверка на недопустимое число
	if (std::isinf(val) || std::isnan(val)) {
		SendEvent(EventNan, nullptr, nullptr, oldStatus == Status::ISNAN);
		m_status = Status::ISNAN;

	} else {
		if (!check) {
			return static_cast<UINT>(m_status);
		}

		// Если значение больше чем аварийный максимум
		// ИЛИ
		// Значение больше чем инж. максимум минус дельта И статус уже равен выходу за инж. максимум  (нужно что бы на гестерезисе попасть в эту ветку, а не поймать AMAX)
		else if((m_setup.Value & Setup::HIHI) && ((val > m_hihi.Value) || ((val > m_hihi.Value - Hysteresis.Value) && (oldStatus == Status::HIHI))))
		{
			SendEvent(EventAMax, &val, &m_hihi.Value, oldStatus == Status::HIHI);
			m_status = Status::HIHI;
		}
		else if((m_setup.Value & Setup::LOLO) && ((val < m_lolo.Value) || ((val < m_lolo.Value + Hysteresis.Value) && (oldStatus == Status::LOLO))))
		{
			SendEvent(EventAMin, &val, &m_lolo.Value, oldStatus == Status::LOLO);
			m_status = Status::LOLO;
		}
		else if((m_setup.Value & Setup::HI) && ((val > m_hi.Value) || ((val > m_hi.Value - Hysteresis.Value) && (oldStatus == Status::HI))))
		{
			SendEvent(EventWMax, &val, &m_hi.Value, oldStatus == Status::HI);
			m_status = Status::HI;
		}
		else if((m_setup.Value & Setup::LO) && ((val < m_lo.Value) || ((val < m_lo.Value + Hysteresis.Value) && (oldStatus == Status::LO))))
		{
			SendEvent(EventWMin, &val, &m_lo.Value, oldStatus == Status::LO);
			m_status = Status::LO;
		}
		else
		{
			SendEvent(EventNormal, &val, nullptr, oldStatus == Status::NORMAL);
			m_status = Status::NORMAL;
		}
	}
	
	return static_cast<UINT>(m_status);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
UDINT rLimit::generateVars(rVariableList& list, const string &owner_name, STRID owner_unit)
{
	if (m_setup.Value & Setup::OFF) {
		return TRITONN_RESULT_OK;
	}

	list.add(owner_name + ".lolo"      , TYPE_LREAL, rVariable::Flags::___L, &m_lolo.Value    , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".lo"        , TYPE_LREAL, rVariable::Flags::___L, &m_lo.Value      , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".hi"        , TYPE_LREAL, rVariable::Flags::___L, &m_hi.Value      , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".hihi"      , TYPE_LREAL, rVariable::Flags::___L, &m_hihi.Value    , owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".hysteresis", TYPE_LREAL, rVariable::Flags::___L, &Hysteresis.Value, owner_unit, ACCESS_LIMITS);
	list.add(owner_name + ".status"    , TYPE_UINT , rVariable::Flags::R___, &m_status        , U_DIMLESS , 0);
	list.add(owner_name + ".setup"     , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value   , U_DIMLESS , ACCESS_LIMITS);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLimit::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	UNUSED(err); UNUSED(prefix);

	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));
	UDINT       fault    = 0;

	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));

	Hysteresis.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::HYSTER), 0.0, fault));
	m_lolo.Init    (XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::LOLO)  , 0.0, fault));
	m_lo.Init      (XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::LO)    , 0.0, fault));
	m_hi.Init      (XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::HI)    , 0.0, fault));
	m_hihi.Init    (XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::HIHI)  , 0.0, fault));

	return TRITONN_RESULT_OK;
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



