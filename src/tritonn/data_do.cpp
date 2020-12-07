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
rDO::rDO() : rSource(), m_setup(0)
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PHIS"  , static_cast<UINT>(Mode::PHIS))
				.add("KEYPAD", static_cast<UINT>(Mode::KEYPAD));
	}

	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"   , static_cast<UINT>(Setup::OFF));
	}

	LockErr  = 0;
	m_setup  = Setup::OFF;
	m_mode   = Mode::PHIS;
	m_status = Status::UNDEF;

	InitLink(rLink::Setup::INOUTPUT, m_present , U_any, SID::PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
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
	rEvent event_success;
	rEvent event_fault;
	
	if(rSource::Calculate()) return 0;

	// Если аналоговый сигнал выключен, то выходим
	if(m_setup.Value & Setup::OFF)
	{
		m_physical      = 0;
		m_present.Value = 0;
		m_mode          = Mode::PHIS;
		m_status        = Status::UNDEF;
		return TRITONN_RESULT_OK;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_setup.Compare(ReinitEvent(EID_DO_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	m_status = Status::NORMAL;

	if(m_mode == Mode::PHIS)
	{
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

			if(channel->m_state)
			{
				Fault    = true;
				m_status = Status::FAULT; // выставляем флаг ошибки
			} else {
				m_physical = channel->getValue();
				//TODO Установить значение
			}
		}
	}

	// Через oldmode делать нельзя, так как нам нужно поймать и ручное переключение
	// можно сделать через m_oldMode, но это не красиво
	if(m_mode == Mode::KEYPAD && !(LockErr & DO_LE_KEYPAD_ON))
	{
		LockErr |=  DO_LE_KEYPAD_ON;
		LockErr &= ~DO_LE_KEYPAD_OFF;
		
		rEventManager::instance().Add(ReinitEvent(EID_DO_KEYPAD_ON));
	}
	
	if(m_mode == Mode::PHIS && !(LockErr & DO_LE_KEYPAD_OFF))
	{
		LockErr |=  DO_LE_KEYPAD_OFF;
		LockErr &= ~DO_LE_KEYPAD_ON;
		
		rEventManager::instance().Add(ReinitEvent(EID_DO_KEYPAD_OFF));
	}
	
	
	PostCalculate();
		
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(Alias + ".setup"     , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value, U_DIMLESS, ACCESS_SA);
	list.add(Alias + ".mode"      , TYPE_UINT , rVariable::Flags::___L, &m_mode       , U_DIMLESS, ACCESS_KEYPAD);
	list.add(Alias + ".status"    , TYPE_UINT , rVariable::Flags::R___, &m_status     , U_DIMLESS, 0);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault        , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::LoadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* module = element->FirstChildElement(XmlName::IOLINK);

	if (module) {
		if (rDataModule::loadFromXML(module, err) != TRITONN_RESULT_OK) {
			return err.getError();
		}
	} else {
		m_present.m_setup |= rLink::Setup::WRITABLE;
	}

	UDINT fault = 0;
	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));
	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));

	if (fault) {
		return err.set(DATACFGERR_DO, element->GetLineNum(), "");
	}

	m_present.Limit.m_setup.Init (rLimit::Setup::OFF);

	ReinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rDO::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	UNUSED(isio);

	m_present.Limit.m_setup.Init (rLimit::Setup::OFF);

	return rSource::saveKernel(true, objname, comment, isglobal);
}






