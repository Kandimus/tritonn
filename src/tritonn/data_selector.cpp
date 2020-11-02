//=================================================================================================
//===
//=== data_selector.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== 
//===
//=================================================================================================


#include <vector>
#include <string.h>
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_link.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_selector.h"
#include "xml_util.h"


using std::vector;


const UDINT SELECTOR_LE_NOCHANGE = 0x00000001;

//-------------------------------------------------------------------------------------------------
//
rSelector::rSelector() : Select(-1)
{
	//TODO Нужно ли очищать свойства класса?
	LockErr     = 0;
	CountGroups = MAX_SELECTOR_GROUP; //  По умолчанию доступен максимальный селектор
	CountInputs = MAX_SELECTOR_INPUT;

	for(UDINT grp = 0; grp < MAX_SELECTOR_GROUP; ++grp)
	{
		Keypad[grp]    = 0.0;
		KpUnit[grp]    = U_any;
		NameInput[grp] = String_format("#username_%i", grp + 1);
	}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//NOTE Таблицу выходов не заполняем, т.к. все будет зависить от загрузки, заполним там.
}


//
rSelector::~rSelector()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::InitLimitEvent(rLink &link)
{
	link.Limit.EventChangeAMin  = ReinitEvent(EID_SELECTOR_NEW_AMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMin  = ReinitEvent(EID_SELECTOR_NEW_WMIN)  << link.Descr << link.Unit;
	link.Limit.EventChangeWMax  = ReinitEvent(EID_SELECTOR_NEW_WMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeAMax  = ReinitEvent(EID_SELECTOR_NEW_AMAX)  << link.Descr << link.Unit;
	link.Limit.EventChangeHyst  = ReinitEvent(EID_SELECTOR_NEW_HYST)  << link.Descr << link.Unit;
	link.Limit.EventChangeSetup = ReinitEvent(EID_SELECTOR_NEW_SETUP) << link.Descr << link.Unit;
	link.Limit.EventAMin        = ReinitEvent(EID_SELECTOR_AMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMin        = ReinitEvent(EID_SELECTOR_WMIN)      << link.Descr << link.Unit;
	link.Limit.EventWMax        = ReinitEvent(EID_SELECTOR_WMAX)      << link.Descr << link.Unit;
	link.Limit.EventAMax        = ReinitEvent(EID_SELECTOR_AMAX)      << link.Descr << link.Unit;
	link.Limit.EventNan         = ReinitEvent(EID_SELECTOR_NAN)       << link.Descr << link.Unit;
	link.Limit.EventNormal      = ReinitEvent(EID_SELECTOR_NORMAL)    << link.Descr << link.Unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::Calculate()
{
	UDINT faultGrp[MAX_SELECTOR_INPUT] = {0};

	if(rSource::Calculate()) return 0;

	if(Setup.Value & SELECTOR_SETUP_OFF) return 0;


	for(UDINT ii = 0; ii < CountInputs; ++ii)
	{
		for(UDINT grp = 0; grp < CountGroups; ++grp)
		{
			faultGrp[ii] += FaultIn[ii][grp].Value;
		}
	}

	// Проверка на изменение данных пользователем
	Select.Compare(ReinitEvent(EID_SELECTOR_SELECTED));
	Mode.Compare(ReinitEvent(EID_SELECTOR_MODE));

	// Если переменная переключатель находится в недопустимом режиме
	//TODO Какие значения будут в выходах?
	if(Select.Value >= CountInputs || Select.Value < -1)
	{
		rEventManager::instance().Add(ReinitEvent(EID_SELECTOR_ERROR) << Select.Value);
		Select.Init(-1);
		Fault = 1;
		//return 0;
	}

	//----------------------------------------------------------------------------------------
	// Автоматические переходы, по статусу ошибки
	if(Select.Value != -1)
	{
		if(faultGrp[Select.Value])
		{
			switch(Mode.Value)
			{
				// Переходы запрещены
				case SELECTOR_MODE_NOCHANGE:
				{
					SendEventSetLE(SELECTOR_LE_NOCHANGE, ReinitEvent(EID_SELECTOR_NOCHANGE) << Select.Value);
					break;
				}

				// Переход на значение ошибки
				case SELECTOR_MODE_TOERROR:
				{
					rEventManager::instance().Add(ReinitEvent(EID_SELECTOR_TOFAULT) << Select.Value);

					Select.Value = -1;
					Fault        = 1;
				}

				// Переходим на следующий канал
				default:
				{
					DINT count = 0;

					LockErr &= ~(SELECTOR_LE_NOCHANGE);

					do
					{
						Select.Value += (Mode.Value == SELECTOR_MODE_CHANGENEXT) ? +1 : -1;
					
						if(Select.Value < 0)           Select.Value = CountInputs - 1;
						if(Select.Value > CountInputs) Select.Value = 0;
						
						++count;
					}
					while(faultGrp[Select.Value] && count < CountInputs - 2);

					if(faultGrp[Select.Value])
					{
						rEventManager::instance().Add(ReinitEvent(EID_SELECTOR_NOTHINGNEXT) << Select.Value);

						Select.Value = -1;
					}
					else
					{
						rEventManager::instance().Add(ReinitEvent(EID_SELECTOR_TONEXT) << Select.Value);
					}
				}
			} //switch
		}
		else
		{
			SendEventClearLE(SELECTOR_LE_NOCHANGE, ReinitEvent(EID_SELECTOR_CLEARERROR) << Select.Value);
		}
	}

	// Записываем в "выхода" требуемые значения входов, так же изменяем ед. измерения у "выходов"
	for(UDINT grp = 0; grp < CountGroups; ++grp)
	{
		ValueOut[grp].Value = (Select.Value == -1) ? Keypad[grp] : ValueIn[Select.Value][grp].Value;
		ValueOut[grp].Unit  = (Select.Value == -1) ? KpUnit[grp] : ValueIn[Select.Value][grp].Unit;
	}

	Fault = (Select.Value == -1) ? 1 : faultGrp[Select.Value];

	PostCalculate();

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Генерация Inputs/Outputs
void rSelector::GenerateIO()
{
	m_inputs.clear();
	m_outputs.clear();

	if(Setup.Value & SELECTOR_SETUP_MULTI)
	{
		// Настройка выходов
		for(UDINT grp = 0; grp < CountGroups; ++grp)
		{
			const string aa = "";
			InitLink(LINK_SETUP_OUTPUT, ValueOut[grp], ValueIn[0][grp].Unit, SID_SEL_GRP1_OUT + grp, NameInput[grp] + ".output", LINK_SHADOW_NONE);
		}

		// Настройка входов
		for(UDINT grp = 0; grp < CountGroups; ++grp)
		{
			for(UDINT ii = 0; ii < CountInputs; ++ii)
			{
				string i_name = String_format("%s.input_%i"      , NameInput[grp].c_str(), ii + 1);
				string f_name = String_format("%s.input_%i.fault", NameInput[grp].c_str(), ii + 1);

				InitLink(LINK_SETUP_INPUT                    , ValueIn[ii][grp], ValueIn[ii][grp].Unit, SID_SEL_GRP1_IN1  + grp * MAX_SELECTOR_INPUT + ii, i_name, LINK_SHADOW_NONE);
				InitLink(LINK_SETUP_INPUT | LINK_SETUP_SIMPLE, FaultIn[ii][grp], U_DIMLESS            , SID_SEL_GRP1_FLT1 + grp * MAX_SELECTOR_INPUT + ii, f_name, i_name          );
			}
		}
	}
	else
	{
		InitLink(LINK_SETUP_OUTPUT, ValueOut[0], ValueIn[0][0].Unit, SID_SEL_OUT, "output", LINK_SHADOW_NONE);

		for(UDINT ii = 0; ii < CountInputs; ++ii)
		{
			string i_name = String_format("input_%i"      , ii + 1);
			string f_name = String_format("input_%i.fault", ii + 1);

			InitLink(LINK_SETUP_INPUT                    , ValueIn[ii][0], ValueIn[ii][0].Unit, SID_SEL_IN1  + ii, i_name, LINK_SHADOW_NONE);
			InitLink(LINK_SETUP_INPUT | LINK_SETUP_SIMPLE, FaultIn[ii][0], U_DIMLESS          , SID_SEL_FLT1 + ii, f_name, i_name          );
		}
	}
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::generateVars(rVariableList& list)
{
	string alias_unit   = "";
	string alias_keypad = "";

	rSource::generateVars(list);

	list.add(Alias + ".Select"    , TYPE_INT  , rVariable::Flags::___L, &Select.Value, U_DIMLESS, ACCESS_SELECT);
	list.add(Alias + ".inputcount", TYPE_UINT , rVariable::Flags::R___, &CountInputs , U_DIMLESS, 0);
	list.add(Alias + ".Setup"     , TYPE_UINT , rVariable::Flags::RS__, &Setup.Value , U_DIMLESS, ACCESS_SA);
	list.add(Alias + ".Mode"      , TYPE_UINT , rVariable::Flags::___L, &Mode.Value  , U_DIMLESS, ACCESS_SELECT);

	list.add(Alias + ".fault"     , TYPE_UDINT, rVariable::Flags::R___, &Fault       , U_DIMLESS, 0);

	// Мультиселектор
	if(Setup.Value & SELECTOR_SETUP_MULTI)
	{
		list.add(Alias + ".selectorcount", TYPE_UINT, rVariable::Flags::R___, &CountGroups , U_DIMLESS, 0);

		for(UDINT grp = 0; grp < CountGroups; ++grp)
		{
			alias_unit   = String_format("%s.%s.keypad.unit" , Alias.c_str(), NameInput[grp].c_str());
			alias_keypad = String_format("%s.%s.keypad.value", Alias.c_str(), NameInput[grp].c_str());

			list.add(alias_unit  , TYPE_UDINT, rVariable::Flags::R__L,  KpUnit[grp].GetPtr(), U_DIMLESS  , 0);
			list.add(alias_keypad, TYPE_LREAL, rVariable::Flags::___L, &Keypad[grp]         , KpUnit[grp], ACCESS_KEYPAD);
		}
	}
	else
	{
		list.add(Alias + ".keypad.unit" , TYPE_UDINT, rVariable::Flags::R__L,  KpUnit[0].GetPtr(), U_DIMLESS, 0);
		list.add(Alias + ".Keypad.value", TYPE_LREAL, rVariable::Flags::___L, &Keypad[0]         , KpUnit[0], ACCESS_KEYPAD);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
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
	if(string(XmlName::SELECTOR) == element->Name())
	{
		tinyxml2::XMLElement *inputs = element->FirstChildElement(XmlName::INPUTS);
		tinyxml2::XMLElement *faults = element->FirstChildElement(XmlName::FAULTS);
		tinyxml2::XMLElement *keypad = element->FirstChildElement(XmlName::KEYPAD);

		if(nullptr == inputs || nullptr == keypad)
		{
			return DATACFGERR_SELECTOR;
		}

		//------------------------------------
		// Входа
		for(tinyxml2::XMLElement *link = inputs->FirstChildElement(XmlName::LINK); link != nullptr; link = link->NextSiblingElement(XmlName::LINK))
		{
			if(tinyxml2::XML_SUCCESS != cfg.LoadLink(link, ValueIn[ii][0])) return cfg.ErrorID;
			++ii;

			if(ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
		}
		if(ii < 2) return DATACFGERR_SELECTOR;

		CountGroups = 1;
		CountInputs = ii;
		ii          = 0;

		//------------------------------------
		// Фаулты (ошибки)
		if(faults)
		{
			for(tinyxml2::XMLElement *link = faults->FirstChildElement(XmlName::LINK); link != nullptr; link = link->NextSiblingElement(XmlName::LINK))
			{
				if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(link, FaultIn[ii][0], ValueIn[ii][0], "fault")) return cfg.ErrorID;
				// Принудительно выключаем пределы
				FaultIn[ii][0].Limit.Setup.Init(LIMIT_SETUP_OFF);

				if(++ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
			}
			if(ii != CountInputs) return DATACFGERR_SELECTOR;
		}

		// Подстановочное значение
		Keypad[0] = rDataConfig::GetTextLREAL(keypad->FirstChildElement(XmlName::VALUE),   0.0, err);
		KpUnit[0] = rDataConfig::GetTextUDINT(keypad->FirstChildElement(XmlName::UNIT) , U_any, err);

		if(err) return DATACFGERR_SELECTOR;
	}

	// Мульти-селектор
	else if(string(XmlName::MSELECTOR) == element->Name())
	{
		Setup.Init(Setup.Value | SELECTOR_SETUP_MULTI);

		tinyxml2::XMLElement *names   = element->FirstChildElement(XmlName::NAMES);
		tinyxml2::XMLElement *inputs  = element->FirstChildElement(XmlName::INPUTS);
		tinyxml2::XMLElement *faults  = element->FirstChildElement(XmlName::FAULTS);
		tinyxml2::XMLElement *keypads = element->FirstChildElement(XmlName::KEYPADS);

		if(nullptr == names || nullptr == inputs || nullptr == keypads)
		{
			return DATACFGERR_SELECTOR;
		}

		// Загружаем имена выходов
		UDINT grp = 0;
		for(tinyxml2::XMLElement *name = names->FirstChildElement(XmlName::NAME); name != nullptr; name = name->NextSiblingElement(XmlName::NAME))
		{
			NameInput[grp] = name->GetText();

			if(NameInput[grp].empty()) return DATACFGERR_SELECTOR;
			++grp;
		}
		CountGroups = grp;

		// Загружаем входа
		ii = 0;
		for(tinyxml2::XMLElement *group = inputs->FirstChildElement(XmlName::GROUP); group != nullptr; group = group->NextSiblingElement(XmlName::GROUP))
		{
			// Линки
			grp = 0;
			for(tinyxml2::XMLElement *link = group->FirstChildElement(XmlName::LINK); link != nullptr; link = link->NextSiblingElement(XmlName::LINK))
			{
				if(tinyxml2::XML_SUCCESS != cfg.LoadLink(link, ValueIn[ii][grp])) return cfg.ErrorID;

				++grp;
				if(grp >= MAX_SELECTOR_GROUP) return DATACFGERR_SELECTOR;
			}
			if(grp != CountGroups) return DATACFGERR_SELECTOR;

			++ii;
			if(ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
		}
		if(ii < 2) return DATACFGERR_SELECTOR;

		CountInputs = ii;

		// Фаулты
		if(faults)
		{
			ii = 0;
			for(tinyxml2::XMLElement *group = faults->FirstChildElement(XmlName::GROUP); group != nullptr; group = group->NextSiblingElement(XmlName::GROUP))
			{
				// Линки
				grp = 0;
				for(tinyxml2::XMLElement *link = group->FirstChildElement(XmlName::LINK); link != nullptr; link = link->NextSiblingElement(XmlName::LINK))
				{
					if(tinyxml2::XML_SUCCESS != cfg.LoadShadowLink(link, FaultIn[ii][grp], ValueIn[ii][grp], XmlName::FAULT)) return cfg.ErrorID;

					FaultIn[ii][grp].Limit.Setup.Init(LIMIT_SETUP_OFF);

					if(++grp >= MAX_SELECTOR_GROUP) return DATACFGERR_SELECTOR;
				}
				if(grp != CountGroups) return DATACFGERR_SELECTOR;

				if(++ii >= MAX_SELECTOR_INPUT) return DATACFGERR_SELECTOR;
			}
			if(ii != CountInputs) return DATACFGERR_SELECTOR;
		}

		// Подстановочные значения
		grp = 0;
		for(tinyxml2::XMLElement *keypad = keypads->FirstChildElement(XmlName::KEYPAD); keypad != nullptr; keypad = keypad->NextSiblingElement(XmlName::KEYPAD))
		{
			Keypad[grp] = rDataConfig::GetTextLREAL(keypad->FirstChildElement(XmlName::VALUE),   0.0, err);
			KpUnit[grp] = rDataConfig::GetTextUDINT(keypad->FirstChildElement(XmlName::UNIT) , U_any, err);

			++grp;
			if(grp >= MAX_SELECTOR_GROUP || err) return DATACFGERR_SELECTOR;
		}
		if(grp != CountGroups) return DATACFGERR_SELECTOR;
	}
	else return DATACFGERR_SELECTOR;

	GenerateIO();

	return tinyxml2::XML_SUCCESS;
}


/*
UDINT rStream::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Counter.Limit.Setup.Init(LIMIT_SETUP_OFF);
	Freq.Limit.Setup.Init();
	Temp.Limit.Setup.Init(0);
	Pres.Limit.Setup.Init(0);
	Dens.Limit.Setup.Init(0);
	Dens15.Limit.Setup.Init(0);
	Dens20.Limit.Setup.Init(0);
	B15.Limit.Setup.Init(0);
	Y15.Limit.Setup.Init(0);


	return rSource::SaveKernel(file, isio, objname, comment, isglobal);
}
*/




