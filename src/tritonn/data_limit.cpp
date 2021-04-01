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

#include "data_limit.h"
#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "event/eid.h"
#include "event/manager.h"
#include "data_config.h"
#include "variable_list.h"
#include "xml_util.h"
#include "comment_defines.h"

rBitsArray rLimit::m_flagsSetup;
rBitsArray rLimit::m_flagsStatus;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rLimit::rLimit() :
	m_lolo(0), m_lo(0), m_hi(0), m_hihi(0), Hysteresis(0.0)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"  , static_cast<UINT>(Setup::OFF) , "Не выдавать сообщения")
				.add("LOLO" , static_cast<UINT>(Setup::LOLO), "Выдавать сообщение аварийного минимума")
				.add("LO"   , static_cast<UINT>(Setup::LO)  , "Выдавать сообщение предаварийного минимума")
				.add("HI"   , static_cast<UINT>(Setup::HI)  , "Выдавать сообщение предаварийного максимума")
				.add("HIHI" , static_cast<UINT>(Setup::HIHI), "Выдавать сообщение аварийного максимума");
	}

	if (m_flagsStatus.empty()) {
		m_flagsStatus
				.add("", static_cast<UINT>(Status::UNDEF) , "Неопределен")
				.add("", static_cast<UINT>(Status::ISNAN) , "Недействительное значение")
				.add("", static_cast<UINT>(Status::LOLO)  , "Значение ниже аварийного минимума")
				.add("", static_cast<UINT>(Status::LO)    , "Значение ниже предаварийного минимума")
				.add("", static_cast<UINT>(Status::NORMAL), "Значение в рабочем диапазоне")
				.add("", static_cast<UINT>(Status::HI)    , "Значение выше предаварийного максимума")
				.add("", static_cast<UINT>(Status::HIHI)  , "Значение выше аварийного максимума");
	}
}


//-------------------------------------------------------------------------------------------------
//
UINT rLimit::calculate(LREAL val, UDINT check)
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
		sendEvent(EventNan, nullptr, nullptr, oldStatus == Status::ISNAN);
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
			sendEvent(EventAMax, &val, &m_hihi.Value, oldStatus == Status::HIHI);
			m_status = Status::HIHI;
		}
		else if((m_setup.Value & Setup::LOLO) && ((val < m_lolo.Value) || ((val < m_lolo.Value + Hysteresis.Value) && (oldStatus == Status::LOLO))))
		{
			sendEvent(EventAMin, &val, &m_lolo.Value, oldStatus == Status::LOLO);
			m_status = Status::LOLO;
		}
		else if((m_setup.Value & Setup::HI) && ((val > m_hi.Value) || ((val > m_hi.Value - Hysteresis.Value) && (oldStatus == Status::HI))))
		{
			sendEvent(EventWMax, &val, &m_hi.Value, oldStatus == Status::HI);
			m_status = Status::HI;
		}
		else if((m_setup.Value & Setup::LO) && ((val < m_lo.Value) || ((val < m_lo.Value + Hysteresis.Value) && (oldStatus == Status::LO))))
		{
			sendEvent(EventWMin, &val, &m_lo.Value, oldStatus == Status::LO);
			m_status = Status::LO;
		}
		else
		{
			sendEvent(EventNormal, &val, nullptr, oldStatus == Status::NORMAL);
			m_status = Status::NORMAL;
		}
	}
	
	return static_cast<UINT>(m_status);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//

//-------------------------------------------------------------------------------------------------
//
UDINT rLimit::generateVars(rVariableList& list, const string &owner_name, STRID owner_unit, const std::string& owner_comment)
{
	if (m_setup.Value & Setup::OFF) {
		return TRITONN_RESULT_OK;
	}

	list.add(owner_name + ".lolo"      , TYPE_LREAL, rVariable::Flags::___, &m_lolo.Value    , owner_unit, ACCESS_LIMITS, owner_comment + ". Значение аварийного минимума");
	list.add(owner_name + ".lo"        , TYPE_LREAL, rVariable::Flags::___, &m_lo.Value      , owner_unit, ACCESS_LIMITS, owner_comment + ". Значение предаварийного минимума");
	list.add(owner_name + ".hi"        , TYPE_LREAL, rVariable::Flags::___, &m_hi.Value      , owner_unit, ACCESS_LIMITS, owner_comment + ". Значение предаварийного максимума");
	list.add(owner_name + ".hihi"      , TYPE_LREAL, rVariable::Flags::___, &m_hihi.Value    , owner_unit, ACCESS_LIMITS, owner_comment + ". Значение аварийного максимума");
	list.add(owner_name + ".hysteresis", TYPE_LREAL, rVariable::Flags::___, &Hysteresis.Value, owner_unit, ACCESS_LIMITS, owner_comment + ". Значение гистерезиса");
	list.add(owner_name + ".status"    , TYPE_UINT , rVariable::Flags::R__, &m_status        , U_DIMLESS , 0            , owner_comment + ". " + COMMENT::STATUS + m_flagsStatus.getInfo(true));
	list.add(owner_name + ".setup"     , TYPE_UINT , rVariable::Flags::RS_, &m_setup.Value   , U_DIMLESS , ACCESS_LIMITS, owner_comment + ". " + COMMENT::SETUP + m_flagsSetup.getInfo());

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLimit::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
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
void rLimit::sendEvent(rEvent &e, LREAL *val, LREAL *lim, UDINT dontsend)
{
	if (dontsend) {
		return;
	}

	rEvent event = e;

	if(val) event << *val;
	if(lim) event << *lim;

	rEventManager::instance().add(event);
}


std::string rLimit::getXML(const std::string& name, const std::string& prefix) const
{
	std::string result = "";

	if (m_setup.Value != Setup::OFF) {
		result += prefix + String_format("<%s name=\"%s\" setup=\"%s\">\n",
								 XmlName::LIMIT,
								 name.c_str(),
								 m_flagsSetup.getNameByBits(m_setup.Value).c_str());

		if (m_setup.Value & Setup::LOLO) {
			result += prefix + String_format("\t<lolo>%g</lolo>\n", m_lolo.Value);
		}

		if (m_setup.Value & Setup::LO) {
			result += prefix + String_format("\t<lo>%g</lo>\n", m_lo.Value);
		}

		if (m_setup.Value & Setup::HI) {
			result += prefix + String_format("\t<hi>%g</hi>\n", m_hi.Value);
		}

		if (m_setup.Value & Setup::HIHI) {
			result += prefix + String_format("\t<hihi>%g</hihi>\n", m_hihi.Value);
		}

		result += prefix + "</" + XmlName::LIMIT + ">\n";
	}

	return result;
}

