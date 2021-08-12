/*
 *
 * data/ao.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "tinyxml2.h"
#include "xml_util.h"
#include "../event/eid.h"
#include "../text_id.h"
#include "../event/manager.h"
#include "../data_manager.h"
#include "../error.h"
#include "../variable_item.h"
#include "../variable_list.h"
#include "../io/manager.h"
#include "../io/aointerface.h"
#include "../generator_md.h"
#include "../comment_defines.h"
#include "ao.h"

const UDINT AO_LE_SIM = 0x00000001;

rBitsArray rAO::m_flagsMode;
rBitsArray rAO::m_flagsSetup;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
rAO::rAO(const rStation* owner) : rSource(owner), m_setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS)  , COMMENT::MODE_PHYS)
				.add("KEYPAD", static_cast<UINT>(Mode::KEYPAD), COMMENT::MODE_KEYPAD);
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"      , static_cast<UINT>(Setup::OFF), COMMENT::SETUP_OFF);
	}

	m_lockErr = 0;
	m_setup   = Setup::OFF;
	m_mode    = Mode::PHIS;

	//NOTE Единицы измерения добавим после загрузки сигнала
	initLink(rLink::Setup::INPUT, m_present, U_any, SID::PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAO::initLimitEvent(rLink& link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_AO_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_AO_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_AO_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_AO_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_AO_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_AO_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_AO_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_AO_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_AO_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_AO_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_AO_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_AO_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}

UDINT rAO::calculate()
{
	rEvent event_s;
	rEvent event_f;
	
	if(rSource::calculate()) return TRITONN_RESULT_OK;

	// Если аналоговый сигнал выключен, то выходим
	if(m_setup.Value & rAO::Setup::OFF)
	{
		m_present.m_value = 0;
		m_mode            = Mode::PHIS;
		return 0;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_scale.m_min.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_AO_NEW_MIN) << m_present.m_unit);
	m_scale.m_max.Compare(COMPARE_LREAL_PREC, reinitEvent(EID_AO_NEW_MAX) << m_present.m_unit);
	m_setup.Compare(reinitEvent(EID_AO_NEW_SETUP));
	
	checkExpr(m_mode == Mode::KEYPAD, AO_LE_SIM,
			  event_f.reinit(EID_AO_SIM_ON)  << m_ID << m_descr,
			  event_s.reinit(EID_AI_SIM_OFF) << m_ID << m_descr);

	//-------------------------------------------------------------------------------------------
	// Преобразуем код АЦП в значение
	if (isSetModule() && m_mode == Mode::PHIS) {
		auto interface = dynamic_cast<rIOAOInterface*>(rIOManager::instance().getModuleInterface(m_module, rIOBaseModule::Type::UNDEF));

		if (!interface) {
			rEventManager::instance().add(reinitEvent(EID_AO_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		UDINT fault    = TRITONN_RESULT_OK;
		LREAL range    = m_scale.m_max.Value - m_scale.m_min.Value;
		UINT  minDAC   = interface->getMinValue(m_channel, rIOBaseChannel::Type::AO, fault);
		UINT  rangeDAC = interface->getRange   (m_channel, rIOBaseChannel::Type::AO, fault);

		if (fault != TRITONN_RESULT_OK) {
			rEventManager::instance().add(reinitEvent(EID_AO_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, fault);
			return fault;
		}

		m_phValue = minDAC + static_cast<UINT>((rangeDAC / range) * (m_present.m_value - m_scale.m_min.Value));
		fault = interface->setValue(m_channel, rIOBaseChannel::Type::AO, m_phValue);

		if (fault != TRITONN_RESULT_OK) {
			rEventManager::instance().add(reinitEvent(EID_AO_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HaltReason::RUNTIME, fault);
			return fault;
		}
	}	
	
	postCalculate();
		
	return TRITONN_RESULT_OK;
}

UDINT rAO::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	m_scale.generateVars(list, m_alias, m_present.m_unit);

	list.add(m_alias + ".phvalue",             rVariable::Flags::RSH_, &m_phValue    , m_present.m_unit, 0            , "Значение ЦАП");
	list.add(m_alias + ".setup"  , TYPE::UINT, rVariable::Flags::RS__, &m_setup.Value, U_DIMLESS       , ACCESS_SA    , COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".mode"   , TYPE::UINT, rVariable::Flags::___D, &m_mode       , U_DIMLESS       , ACCESS_KEYPAD, COMMENT::MODE + m_flagsMode.getInfo(true));

	list.add(m_alias + ".fault"  ,             rVariable::Flags::R___, &m_fault      , U_DIMLESS       , 0, COMMENT::FAULT);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rAO::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_module = element->FirstChildElement(XmlName::IOLINK);
	tinyxml2::XMLElement* xml_limits = element->FirstChildElement(XmlName::LIMITS);
	tinyxml2::XMLElement* xml_unit   = element->FirstChildElement(XmlName::UNIT);
	tinyxml2::XMLElement* xml_scale  = element->FirstChildElement(XmlName::SCALE);

	if (xml_module) {
		if (rDataModule::loadFromXML(xml_module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	}

	if (!xml_limits || !xml_unit || !xml_scale) {
		return err.set(DATACFGERR_AO, element->GetLineNum(), "cant found limits or unit or scale");
	}

	UDINT fault = 0;
	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));

	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));

	if (fault) {
		return err.set(DATACFGERR_AO, element->GetLineNum(), "fault load attributes");
	}

	m_scale.m_min.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement(XmlName::LOW) , 0.0, fault));
	m_scale.m_max.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement(XmlName::HIGH), 100.0, fault));

	if (fault) {
		return err.set(DATACFGERR_AO, element->GetLineNum(), "fault load scales");
	}

	STRID unit = XmlUtils::getTextUDINT(element->FirstChildElement(XmlName::UNIT), U_any, fault);

	if (fault) {
		return err.set(DATACFGERR_AO, element->GetLineNum(), "units");
	}

	m_present.m_unit = unit;

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}

UDINT rAO::generateMarkDown(rGeneratorMD& md)
{
	m_present.m_limit.m_setup.Init(LIMIT_SETUP_ALL);

	md.add(this, true, rGeneratorMD::Type::IOCHANNEL_OPT)
			.addProperty(XmlName::SETUP, &m_flagsSetup)
			.addProperty(XmlName::MODE , &m_flagsMode, true)
			.addXml(XmlName::UNIT  , static_cast<UDINT>(U_any))
			.addXml("<" + std::string(XmlName::SCALE) + ">")
			.addXml(XmlName::MIN, m_scale.m_min.Value, false, "\t")
			.addXml(XmlName::MAX, m_scale.m_max.Value, false, "\t")
			.addXml("</" + std::string(XmlName::SCALE) + ">");

	return TRITONN_RESULT_OK;
}

