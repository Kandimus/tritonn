//=================================================================================================
//===
//=== data_sampler.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пробоотборника
//===
//=================================================================================================

#include "data_sampler.h"
#include <vector>
#include <string.h>
#include "error.h"
#include "event_eid.h"
#include "data_config.h"
#include "xml_util.h"

rBitsArray rSampler::m_flagsMode;
rBitsArray rSampler::m_flagsSetup;


//-------------------------------------------------------------------------------------------------
//
rSampler::rSampler()
{
	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("PERIOD", static_cast<UINT>(Mode::PERIOD))
				.add("MASS"  , static_cast<UINT>(Mode::MASS))
				.add("VOLUME", static_cast<UINT>(Mode::VOLUME));
	}

	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"       , static_cast<UINT>(Setup::OFF))
				.add("ERRRESERV" , static_cast<UINT>(Setup::ERR_RESERV))
				.add("FILLRESERV", static_cast<UINT>(Setup::FILL_RESERV))
				.add("SINGLECAN" , static_cast<UINT>(Setup::SINGLE_CAN))
				.add("DUAL_CAN"  , static_cast<UINT>(Setup::DUAL_CAN))
				.add("AUTOSWITCH", static_cast<UINT>(Setup::AUTOSWITCH));
	}

	for (auto ii = 0; ii < CAN_MAX; ++ii) {
		InitLink(rLink::Setup::INPUT , m_can[ii].m_filled, U_any, SID_CANFILLED, XmlName::CANFILLED, rLink::SHADOW_NONE);
		InitLink(rLink::Setup::INPUT , m_can[ii].m_rrror , U_any, SID_FAULT    , XmlName::FAULT    , rLink::SHADOW_NONE);
		InitLink(rLink::Setup::INPUT , m_cam[ii].m_mass  , U_g  , SID_CANMASS  , XmlName::MASS     , rLink::SHADOW_NONE);
	}
	InitLink(rLink::Setup::INPUT , m_ioStart , U_any, SID_CANIOSTART, XmlName::IO_START, rLink::SHADOW_NONE);
	InitLink(rLink::Setup::INPUT , m_ioStop  , U_any, SID_CANIOSTOP , XmlName::IO_STOP , rLink::SHADOW_NONE);

	InitLink(rLink::Setup::OUTPUT, m_grab    , U_any, SID_GRAB     , XmlName::GRAB     , rLink::SHADOW_NONE);
	InitLink(rLink::Setup::OUTPUT, m_selected, U_any, SID_CANSELECT, XmlName::SELECTED , rLink::SHADOW_NONE);
}


//
rSampler::~rSampler()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::InitLimitEvent(rLink &/*link*/)
{
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::Calculate()
{
/*
	USINT  ii        = 0;
	UINT   mode      = Setup & SELECTOR_MODE; // Забираем только текущий выбор
	LREAL  _valueout = 0.0;
	INT    _select   = Select.Value;
	UDINT  _lockerr  = LockErr;
	rEvent event;
	
	vector<rWriteValue> arr;

	
	if(Setup & SELECTOR_OFF) return 0;

	// Проверка на изменение данных пользователем
	event.Reinit(EID_SELECTOR_SELECTED) << id;
	Select.Compare(event);	

	
	// Если переменная переключатель находится в недопустимом режиме
	if(_select >= Count || _select < -1)
	{
		Event_ID_UINT(EID_SELECTOR_ERROR, _select, id);
		
		_select = -1;
	}
	else
	{
		_valueout = ValueIn[_select];
		
		//----------------------------------------------------------------------------------------
		// Автоматические переходы, по статусу ошибки
		if(Fault[_select])
		{
			switch(mode)
			{
				// Переходы запрещены
				case SELECTOR_NOCHANGE:
					{
						if(!(_lockerr & SELECTOR_LE_NOCHANGE))
						{
							Event_ID_UINT(EID_SELECTOR_NOCHANGE, _select, id);
						
							_lockerr |= SELECTOR_LE_NOCHANGE;
						}
						break;
					}

				// Переход на значение ошибки
				case SELECTOR_TOERROR:
					{
						Event_ID_UINT(EID_SELECTOR_TOFAULT, _select, id);
					
						_select   = -1;
						_valueout = ValueFault;
					}

				// Переходим на следующий канал
				default:
					{
						UINT oldSelect = _select;

						ii        = 0;
						_lockerr &= ~(SELECTOR_LE_NOCHANGE);
						do
						{
							_select += (Setup & SELECTOR_CHANGENEXT) ? +1 : -1;
					
							if(_select < 0)     _select = Count - 1;
							if(_select > Count) _select = 0;
						
							++ii;
						}
						while(Fault[_select] && ii < Count - 2);

						if(Fault[_select])
						{
							Event_ID_UINT(EID_SELECTOR_NOTHINGNEXT, _select, id);
						
							_select = -1;
						}
						else
						{
							Event_ID_UINT_UINT(EID_SELECTOR_TONEXT, _select, oldSelect, id);
						}
				}
			} //switch
		}
		else
		{
			if(_lockerr & SELECTOR_LE_NOCHANGE)
			{
				Event_ID_UINT(EID_SELECTOR_CLEARERROR, id, _select);
			}
			_lockerr &= ~(SELECTOR_LE_NOCHANGE);
		}

		_valueout = (_select >= 0) ? ValueIn[_select] : ValueFault;
	}// else

	// Формируем запись данных в DataManager
	arr.push_back(rWriteValue(SSPOINTER(Selector[id].Select.Value), _select));
	arr.push_back(rWriteValue(SSPOINTER(Selector[id].LockErr)     , _lockerr));
	arr.push_back(rWriteValue(SSPOINTER(Selector[id].ValueOut)    , _valueout));
	gDataManager.Set(arr);
*/
	return 0;
}

//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::generateVars(rVariableList& list)
{
	rSource::generateVars(list);

	// Variables
	list.add(Alias + ".mode"    , TYPE_UINT, rVariable::Flags::___L, &m_mode, U_UNDEF, ACCESS_SAMPLERS);

//	list.push_back(new rVariable(Alias + ".Status"    , TYPE_UINT , VARF_RO, SSPOINTER(IO.AI[ID].Status    )));
//	list.push_back(new rVariable(Alias + ".Mode"      , TYPE_UINT , VARF_RW, SSPOINTER(IO.AI[ID].Mode      )));
//	list.push_back(new rVariable(Alias + ".Unit"      , TYPE_STRID, VARF_RO, SSPOINTER(IO.AI[ID].Unit      )));

	return TRITONN_RESULT_OK;
}

UDINT rSampler::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE , m_flagsMode.getNameByBits (static_cast<UINT>(Mode::PERIOD)));
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));

	if (rSource::LoadFromXML(element, err, prefix) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	tinyxml2::XMLElement* xml_totals   = element->FirstChildElement(XmlName::TOTALS);
	tinyxml2::XMLElement* xml_iostart  = element->FirstChildElement(XmlName::IOSTART);
	tinyxml2::XMLElement* xml_iostop   = element->FirstChildElement(XmlName::IOSTOP);
	tinyxml2::XMLElement* xml_reserve  = element->FirstChildElement(XmlName::RESERVE);
	tinyxml2::XMLElement* xml_grabvol  = element->FirstChildElement(XmlName::GRABVOL);
	tinyxml2::XMLElement* xml_period   = element->FirstChildElement(XmlName::PERIOD);
	tinyxml2::XMLElement* xml_grabtest = element->FirstChildElement(XmlName::GRABTEST);
	tinyxml2::XMLElement* xml_can1     = element->FirstChildElement(XmlName::CAN1);
	tinyxml2::XMLElement* xml_can2     = element->FirstChildElement(XmlName::CAN2);

	if (!xml_totals) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, element->GetLineNum(), "");
	}

	UDINT failt = 0;
	m_mode = static_cast<Mode>(m_flagsMode.getValue(strMode, fault));
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_MODE, element->GetLineNum(), "");
	}

	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_SETUP, element->GetLineNum(), "");
	}

	m_totalsAlias = XmlUtils::getTextString(xml_totals, "", fault);
	if (fault) {
		return err.set(DATACFGERR_SAMPLER_TOTALS, element->GetLineNum(), "empty alias");
	}

	if (rDataConfig::instance().LoadLink(xml_iostart, m_ioStart, false) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	if (rDataConfig::instance().LoadLink(xml_iostop, m_ioStop, false) != TRITONN_RESULT_OK) {
		return err.getError();
	}

	return TRITONN_RESULT_OK;
}



