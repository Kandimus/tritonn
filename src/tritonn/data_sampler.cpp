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


#include <vector>
#include <string.h>
#include "data_sampler.h"
#include "event_eid.h"
#include "data_manager.h"


//const UDINT SELECTOR_LE_NOCHANGE = 0x00000001;

//-------------------------------------------------------------------------------------------------
//
rSampler::rSampler()
{
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
//	link.Limit.EventChangeAMin  = ReinitEvent(EID_RDCDDENS_NEW_AMIN)  << link.Descr << link.Unit;
//	link.Limit.EventChangeWMin  = ReinitEvent(EID_RDCDDENS_NEW_WMIN)  << link.Descr << link.Unit;
//	link.Limit.EventChangeWMax  = ReinitEvent(EID_RDCDDENS_NEW_WMAX)  << link.Descr << link.Unit;
//	link.Limit.EventChangeAMax  = ReinitEvent(EID_RDCDDENS_NEW_AMAX)  << link.Descr << link.Unit;
//	link.Limit.EventChangeHyst  = ReinitEvent(EID_RDCDDENS_NEW_HYST)  << link.Descr << link.Unit;
//	link.Limit.EventChangeSetup = ReinitEvent(EID_RDCDDENS_NEW_SETUP) << link.Descr << link.Unit;
//	link.Limit.EventAMin        = ReinitEvent(EID_RDCDDENS_AMIN)      << link.Descr << link.Unit;
//	link.Limit.EventWMin        = ReinitEvent(EID_RDCDDENS_WMIN)      << link.Descr << link.Unit;
//	link.Limit.EventWMax        = ReinitEvent(EID_RDCDDENS_WMAX)      << link.Descr << link.Unit;
//	link.Limit.EventAMax        = ReinitEvent(EID_RDCDDENS_AMAX)      << link.Descr << link.Unit;
//	link.Limit.EventNan         = ReinitEvent(EID_RDCDDENS_NAN)       << link.Descr << link.Unit;
//	link.Limit.EventNormal      = ReinitEvent(EID_RDCDDENS_NORMAL)    << link.Descr << link.Unit;

	return 0;
}



//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::GetFault(void)
{
	UDINT result = 0;

	return result;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSampler::generateVars(rVariableList&/*list*/)
{
//	list.push_back(new rVariable(Alias + ".Status"    , TYPE_UINT , VARF_RO, SSPOINTER(IO.AI[ID].Status    )));
//	list.push_back(new rVariable(Alias + ".Mode"      , TYPE_UINT , VARF_RW, SSPOINTER(IO.AI[ID].Mode      )));
//	list.push_back(new rVariable(Alias + ".Unit"      , TYPE_STRID, VARF_RO, SSPOINTER(IO.AI[ID].Unit      )));

	return TRITONN_RESULT_OK;
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



UDINT rSampler::LoadFromXML(tinyxml2::XMLElement */*element*/, rDataConfig &/*cfg*/)
{
	return TRITONN_RESULT_OK;
/*
	string defSetup = rDataConfig::GetFlagNameByBit  (rDataConfig::SelectorSetupFlags, SELECTOR_SETUP_OFF);
	string defMode  = rDataConfig::GetFlagNameByValue(rDataConfig::SelectorModeFlags , SELECTOR_MODE_CHANGENEXT);
	string strSetup = (element->Attribute("setup")) ? element->Attribute("setup") : defSetup.c_str();
	string strMode  = (element->Attribute("mode") ) ? element->Attribute("mode")  : defMode.c_str();
	UDINT  err      = 0;
	UDINT  ii       = 0;

	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return DATACFGERR_SELECTOR;

	Setup.Init(rDataConfig::GetFlagFromStr(rDataConfig::SelectorSetupFlags, strSetup, err));
	Mode.Init (rDataConfig::GetFlagFromStr(rDataConfig::SelectorModeFlags , strMode , err));
	if(err) return DATACFGERR_SELECTOR;

	// Простой селектор
	if(string(CFGNAME_SELECTOR) == element->Name())
	{
		tinyxml2::XMLElement *inputs = element->FirstChildElement(CFGNAME_INPUTS);
		tinyxml2::XMLElement *faults = element->FirstChildElement(CFGNAME_FAULTS);

		if(nullptr == inputs)
		{
			return DATACFGERR_SELECTOR;
		}

		// Входа
		for(tinyxml2::XMLElement *link = inputs->FirstChildElement(CFGNAME_LINK); link != nullptr; link = link->NextSiblingElement(CFGNAME_LINK))
		{
			if(tinyxml2::XML_SUCCESS != cfg.LoadLink(link, &ValueLink[0][ii])) return cfg.ErrorID;
			++ii;

			if(ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
		}

		CountInputs = ii;
		ii          = 0;

		// Фаулты (ошибки)
		if(faults)
		{
			for(tinyxml2::XMLElement *link = faults->FirstChildElement(CFGNAME_LINK); link != nullptr; link = link->NextSiblingElement(CFGNAME_LINK))
			{
				if(tinyxml2::XML_SUCCESS != cfg.LoadLink(link, &FaultLink[0][ii])) return cfg.ErrorID;
				++ii;

				if(ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
			}
			if(ii != CountInputs) return DATACFGERR_SELECTOR;
		}

		// Подстановочное значение
		ValueFault[0] = rDataConfig::GetValueLREAL(element->FirstChildElement(CFGNAME_FAULTVAL), 0.0, err);
		if(err) return DATACFGERR_SELECTOR;
	}

	// Мульти-селектор
	else if(string(CFGNAME_MSELECTOR) == element->Name())
	{
		Setup.Init(Setup.Value | SELECTOR_SETUP_MULTI);

		tinyxml2::XMLElement *names     = element->FirstChildElement(CFGNAME_NAMES);
		tinyxml2::XMLElement *inputs    = element->FirstChildElement(CFGNAME_INPUTS);
		tinyxml2::XMLElement *faults    = element->FirstChildElement(CFGNAME_FAULTS);
		tinyxml2::XMLElement *faultvals = element->FirstChildElement(CFGNAME_FAULTVALS);

		if(nullptr == names || nullptr == inputs || nullptr == faultvals)
		{
			return DATACFGERR_SELECTOR;
		}

		// Загружаем имена выходов
		UDINT grp = 0;
		for(tinyxml2::XMLElement *name = names->FirstChildElement(CFGNAME_NAME); name != nullptr; name = name->NextSiblingElement(CFGNAME_NAME))
		{
			NameInput[grp] = name->GetText();

			if(NameInput[grp].empty()) return DATACFGERR_SELECTOR;
			++grp;
		}
		CountGroup = grp;

		// Загружаем входа
		ii = 0;
		for(tinyxml2::XMLElement *group = inputs->FirstChildElement(CFGNAME_GROUP); group != nullptr; group = group->NextSiblingElement(CFGNAME_GROUP))
		{
			// Линки
			grp = 0;
			for(tinyxml2::XMLElement *link = group->FirstChildElement(CFGNAME_LINK); link != nullptr; link = link->NextSiblingElement(CFGNAME_LINK))
			{
				if(tinyxml2::XML_SUCCESS != cfg.LoadLink(link, &ValueLink[ii][grp])) return cfg.ErrorID;

				++grp;
				if(grp >= MAX_SELECTOR_GROUP) return DATACFGERR_SELECTOR;
			}
			if(grp != CountGroup) return DATACFGERR_SELECTOR;

			++ii;
			if(ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
		}
		CountInputs = ii;

		// Фаулты
		if(faults)
		{
			ii = 0;
			for(tinyxml2::XMLElement *group = faults->FirstChildElement(CFGNAME_GROUP); group != nullptr; group = group->NextSiblingElement(CFGNAME_GROUP))
			{
				// Линки
				grp = 0;
				for(tinyxml2::XMLElement *link = group->FirstChildElement(CFGNAME_LINK); link != nullptr; link = link->NextSiblingElement(CFGNAME_LINK))
				{
					if(tinyxml2::XML_SUCCESS != cfg.LoadLink(link, &FaultLink[ii][grp])) return cfg.ErrorID;

					++grp;
					if(grp >= MAX_SELECTOR_GROUP) return DATACFGERR_SELECTOR;
				}
				if(grp != CountGroup) return DATACFGERR_SELECTOR;

				++ii;
				if(ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
			}
			if(ii != CountInputs) return DATACFGERR_SELECTOR;
		}

		// Подстановочные значения
		grp = 0;
		for(tinyxml2::XMLElement *faultval = faultvals->FirstChildElement(CFGNAME_FAULTVAL); faultval != nullptr; faultval = faultval->NextSiblingElement(CFGNAME_FAULTVAL))
		{
			ValueFault[grp] = rDataConfig::GetValueLREAL(faultval, 0.0, err);

			++grp;
			if(grp >= MAX_SELECTOR_GROUP || err) return DATACFGERR_SELECTOR;
		}
		if(grp != CountGroup) return DATACFGERR_SELECTOR;
	}
	else return DATACFGERR_SELECTOR;
*/
	return TRITONN_RESULT_OK;
}



