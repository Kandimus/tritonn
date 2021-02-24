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
rDO::rDO(const rStation* owner) : rSource(owner)
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

	m_lockErr = 0;
	m_setup   = Setup::OFF;
	m_mode    = Mode::PHIS;
	m_status  = Status::UNDEF;

	initLink(rLink::Setup::INOUTPUT, m_present , U_discrete, SID::PRESENT , XmlName::PRESENT , rLink::SHADOW_NONE);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::initLimitEvent(rLink& link)
{
	UNUSED(link);
	return TRITONN_RESULT_OK;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rDO::calculate()
{	
	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	// Если аналоговый сигнал выключен, то выходим
	if (m_setup.Value & Setup::OFF) {
		m_physical        = 0;
		m_present.m_value = 0;
		m_mode            = Mode::PHIS;
		m_status          = Status::UNDEF;
		return TRITONN_RESULT_OK;
	}

	//-------------------------------------------------------------------------------------------
	// Обработка ввода пользователя
	m_setup.Compare(reinitEvent(EID_DO_NEW_SETUP));
	// Сообщения об изменении Mode формируем в ручную
	
	m_status = Status::NORMAL;

	if (m_mode == Mode::PHIS) {
		if (isSetModule()) {
			auto channel_ptr = rIOManager::instance().getChannel(m_module, m_channel);
			auto channel     = static_cast<rIODOChannel*>(channel_ptr.get());

			if (channel == nullptr) {
				rEventManager::instance().Add(reinitEvent(EID_DO_MODULE) << m_module << m_channel);
				rDataManager::instance().DoHalt(HALT_REASON_RUNTIME | DATACFGERR_REALTIME_MODULELINK);
				return DATACFGERR_REALTIME_MODULELINK;
			}

			rEvent event_s;
			rEvent event_f;
			checkExpr(channel->m_state, DO_LE_CODE_FAULT,
					  event_f.Reinit(EID_DO_CH_FAULT) << m_ID << m_descr,
					  event_s.Reinit(EID_DO_CH_OK)    << m_ID << m_descr);

			if (channel->m_state) {
				m_fault  = true;
				m_status = Status::FAULT; // выставляем флаг ошибки
			} else {
				m_physical = channel->getValue();
				//TODO Установить значение
			}
		}
	}

	// Через oldmode делать нельзя, так как нам нужно поймать и ручное переключение
	// можно сделать через m_oldMode, но это не красиво
	if (m_mode == Mode::KEYPAD && !(m_lockErr & DO_LE_KEYPAD_ON)) {
		m_lockErr |=  DO_LE_KEYPAD_ON;
		m_lockErr &= ~DO_LE_KEYPAD_OFF;
		
		rEventManager::instance().Add(reinitEvent(EID_DO_KEYPAD_ON));
	}
	
	if (m_mode == Mode::PHIS && !(m_lockErr & DO_LE_KEYPAD_OFF)) {
		m_lockErr |=  DO_LE_KEYPAD_OFF;
		m_lockErr &= ~DO_LE_KEYPAD_ON;
		
		rEventManager::instance().Add(reinitEvent(EID_DO_KEYPAD_OFF));
	}
	
	postCalculate();
		
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	list.add(m_alias + ".setup" , TYPE_UINT , rVariable::Flags::RS_L, &m_setup.Value, U_DIMLESS, ACCESS_SA);
	list.add(m_alias + ".mode"  , TYPE_UINT , rVariable::Flags::___L, &m_mode       , U_DIMLESS, ACCESS_KEYPAD);
	list.add(m_alias + ".status", TYPE_UINT , rVariable::Flags::R___, &m_status     , U_DIMLESS, 0);

	list.add(m_alias + ".fault" , TYPE_UDINT, rVariable::Flags::R___, &m_fault      , U_DIMLESS, 0);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDO::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PHIS)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::loadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
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

	m_present.m_limit.m_setup.Init (rLimit::Setup::OFF);

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


std::string rDO::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	UNUSED(isio);

	m_present.m_limit.m_setup.Init (rLimit::Setup::OFF);

	return rSource::saveKernel(true, objname, comment, isglobal);
}

