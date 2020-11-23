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
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "io/manager.h"
#include "io/do_channel.h"
#include "xml_util.h"

const UDINT DO_LE_KEYPAD_ON  = 0x00000001;
const UDINT DO_LE_KEYPAD_OFF = 0x00000002;
const UDINT DO_LE_CODE_FAULT = 0x00000004;

rBitsArray rDO::m_flagsMode;
rBitsArray rDO::m_flagsSetup;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rDO::rDO() : rSource(), m_keypadValue(0), m_setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS))
				.add("KEYPAD", static_cast<UINT>(Mode::MKEYPAD));
	}
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"   , Setup::OFF)
				.add("KEYPAD", Setup::ERR_KEYPAD);
	}

	LockErr  = 0;
	m_setup  = Setup::OFF;
	m_mode   = Mode::PHIS;
	m_status = Status::UNDEF;

	InitLink(LINK_SETUP_INOUTPUT, m_present , U_any, SID_PRESENT , XmlName::PRESENT , LINK_SHADOW_NONE);
	InitLink(LINK_SETUP_OUTPUT  , m_physical, U_any, SID_PHYSICAL, XmlName::PHYSICAL, LINK_SHADOW_NONE);
}


rDO::~rDO()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::InitLimitEvent(rLink& link)
{
	UNUSED(link);
	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rDO::Calculate()
{
	Status oldStatus = m_status;
	rEvent event_success;
	rEvent event_fault;
	
	if(rSource::Calculate()) return 0;

	// Если аналоговый сигнал выключен, то выходим
	if(m_setup.Value & Setup::OFF)
	{
		m_physical.Value = 0;
		m_present.Value  = 0;
		m_mode           = Mode::PHIS;
		m_status         = Status::UNDEF;
		return TRITONN_RESULT_OK;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_keypadValue.Compare(ReinitEvent(EID_DO_NEW_SIMULATE));
	m_setup.Compare(ReinitEvent(EID_DO_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	m_status = Status::UNDEF;

	if(isSetModule())
	{
		auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
		auto channel     = static_cast<rIODOChannel*>(channel_ptr.get());

		if (channel == nullptr)
		{
			rEventManager::instance().Add(ReinitEvent(EID_DO_MODULE) << m_module << m_channel);
			rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
			return DATACFGERR_REALTIME_MODULELINK;
		}

		CheckExpr(channel->m_state, DO_LE_CODE_FAULT, event_fault.Reinit(EID_DO_CH_FAULT) << ID << Descr, event_success.Reinit(EID_DO_CH_OK) << ID << Descr);

//		m_physical.Value = channel->getValue();
		m_status         = Status::NORMAL;

		if(channel->m_state)
		{
			Fault = true;

			if(m_mode == Mode::PHIS)
			{
				// если симуляция разрешена, то симулируем этот сигнал
				if(m_setup.Value & Setup::ERR_KEYPAD)
				{
					m_mode = Mode::KEYPAD;
				}

				m_status = Status::FAULT; // выставляем флаг ошибки
			}

			SetFault();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// РЕЖИМЫ РАБОТЫ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Через oldmode делать нельзя, так как нам нужно поймать и ручное переключение
	// можно сделать через m_oldMode, но это не красиво
	if(m_mode == Mode::KEYPAD && !(LockErr & DI_LE_KEYPAD_ON))
	{
		LockErr |= DI_LE_KEYPAD_ON;
		LockErr &= ~DI_LE_KEYPAD_OFF;
		
		rEventManager::instance().Add(ReinitEvent(EID_DI_KEYPAD_ON));
	}
	
	if(m_mode == Mode::PHIS && !(LockErr & DI_LE_KEYPAD_OFF))
	{
		LockErr |= DI_LE_KEYPAD_OFF;
		LockErr &= ~DI_LE_KEYPAD_ON;
		
		rEventManager::instance().Add(ReinitEvent(EID_DI_KEYPAD_OFF));
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// ВЫХОДНОЕ ЗНАЧЕНИЕ
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////
	
	// Если сигнал за засимулирован, то значение равно значению симуляции
	if(m_mode == Mode::KEYPAD)
	{
		m_present.Value = m_keypadValue.Value;
	}
	// Если используем "физическое" значение
	else if(isSetModule())
	{
		m_present.Value = m_physical.Value;
	}

	PostCalculate();
		
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rDI::SetFault()
{
	m_physical.Value = 0;
	m_present.Value  = 0;
	m_status         = Status::FAULT;
	Fault            = true;

	return Fault;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDI::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(Alias + ".keypad"    , TYPE_USINT, rVariable::Flags::___L, &m_keypadValue.Value, U_DIMLESS, ACCESS_KEYPAD);
	list.add(Alias + ".setup"     , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value      , U_DIMLESS, ACCESS_SA);
	list.add(Alias + ".mode"      , TYPE_UINT , rVariable::Flags::___L, &m_mode             , U_DIMLESS, ACCESS_KEYPAD);
	list.add(Alias + ".status"    , TYPE_UINT , rVariable::Flags::R___, &m_status           , U_DIMLESS, 0);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault              , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDI::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));
	UDINT  err      = 0;
	UDINT  result   = TRITONN_RESULT_OK;

	if ((result = rSource::LoadFromXML(element, cfg)) != TRITONN_RESULT_OK) {
		return result;
	}

	tinyxml2::XMLElement* module = element->FirstChildElement(XmlName::IOLINK);

	if (module) {
		if ((result = rDataModule::loadFromXML(module, cfg)) != TRITONN_RESULT_OK) {
			return result;
		}
	} else {
		m_present.Setup |= LINK_SETUP_WRITABLE;
	}

	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, err));
	m_setup.Init(m_flagsSetup.getValue(strSetup, err));
	m_keypadValue.Init(XmlUtils::getTextBOOL(element->FirstChildElement(XmlName::KEYPAD), false, err));

	if(err)
	{
		return DATACFGERR_AI;
	}

	m_physical.Limit.m_setup.Init(rLimit::Setup::OFF);
	m_present.Limit.m_setup.Init (rLimit::Setup::OFF);

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rDI::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	m_physical.Limit.m_setup.Init(rLimit::Setup::NONE);
	m_present.Limit.m_setup.Init (rLimit::Setup::NONE);

	return rSource::saveKernel(true, objname, comment, isglobal);
}






