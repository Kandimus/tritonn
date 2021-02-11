//=================================================================================================
//===
//=== prove.cpp
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс поверки
//===
//=================================================================================================

#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "tinyxml2.h"
#include "event_eid.h"
#include "text_id.h"
#include "../event_manager.h"
#include "../data_manager.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../io/manager.h"
#include "../io/ai_channel.h"
#include "xml_util.h"
#include "prove.h"

rBitsArray rProve::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rProve::rProve(const rStation* owner) : rSource(owner), m_setup(static_cast<UINT>(Setup::NONE))
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("NONE"         , static_cast<UINT>(Setup::NONE))
				.add("4WAY"         , static_cast<UINT>(Setup::VALVE_4WAY))
				.add("STABILIZATION", static_cast<UINT>(Setup::STABILIZATION));
	}

	//NOTE Единицы измерения добавим после загрузки сигнала
	InitLink(rLink::Setup::OUTPUT, m_temp, U_C  , SID::TEMPERATURE, XmlName::TEMP, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_pres, U_MPa, SID::PRESSURE   , XmlName::PRES, rLink::SHADOW_NONE);
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
UDINT rProve::Calculate()
{
	rEvent event_success;
	rEvent event_fault;
	
	if(rSource::Calculate()) return TRITONN_RESULT_OK;

	switch(m_state) {
		case State::IDLE: onIdle(); break;
		case State::START: onStart(); break;
		case State::NOFLOW: onNoFlow(); break;
	};

	PostCalculate();
		
	return TRITONN_RESULT_OK;
}

void rProve::onIdle()
{
	switch(m_command) {
		case Command::NONE: return;
		case Command::START: m_state = State::START; return;
	}
}


UDINT rProve::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".command"            , TYPE_UINT , rVariable::Flags::____, &m_command    , U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".setup"              , TYPE_UINT , rVariable::Flags::____, &m_setup.Value, U_DIMLESS, ACCESS_PROVE);
	list.add(Alias + ".state"              , TYPE_UINT , rVariable::Flags::R___, &m_state      , U_DIMLESS, 0);
	list.add(Alias + ".average.temperature", TYPE_LREAL, rVariable::Flags::R__L, &m_inTemp     , U_C      , 0);
	list.add(Alias + ".average.pressure"   , TYPE_LREAL, rVariable::Flags::R__L, &m_inPres     , U_MPa    , 0);
	list.add(Alias + ".timer.stabilization", TYPE_UDINT, rVariable::Flags::___L, &m_timerStab  , U_sec    , ACCESS_PROVE);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault            , U_DIMLESS     , 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAI::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::LoadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);
	tinyxml2::XMLElement* xml_limits = element->FirstChildElement(XmlName::LIMITS); // Limits считываем только для проверки
	tinyxml2::XMLElement* xml_unit   = element->FirstChildElement(XmlName::UNIT);
	tinyxml2::XMLElement* xml_scale  = element->FirstChildElement(XmlName::SCALE);

	// Если аналоговый сигнал не привязан к каналу, то разрешаем менять его значение
	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	if (!xml_limits || !xml_unit || !xml_scale) {
		return err.set(DATACFGERR_AI, element->GetLineNum(), "cant found limits or unit or scale");
	}

	UDINT fault = 0;
	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));

	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));

	KeypadValue.Init(XmlUtils::getTextLREAL(element->FirstChildElement(XmlName::KEYPAD) , 0.0, fault));
	m_scale.Min.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement  (XmlName::MIN), 0.0, fault));
	m_scale.Max.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement  (XmlName::MAX), 0.0, fault));

	STRID Unit = XmlUtils::getTextUDINT(element->FirstChildElement(XmlName::UNIT), U_any, fault);

	if (fault) {
		return err.set(DATACFGERR_AI, element->GetLineNum(), "");
	}

	// Подправляем единицы измерения, исходя из конфигурации AI
	m_present.Unit = Unit;
	PhValue.Unit   = Unit;

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rAI::saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal)
{
	m_present.Limit.m_setup.Init(rLimit::Setup::NONE);
	PhValue.Limit.m_setup.Init(rLimit::Setup::NONE);
	Current.Limit.m_setup.Init(rLimit::Setup::NONE);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}






