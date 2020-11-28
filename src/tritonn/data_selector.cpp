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
#include "error.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_config.h"
#include "data_link.h"
#include "variable_item.h"
#include "variable_list.h"
#include "data_selector.h"
#include "xml_util.h"


const UDINT SELECTOR_LE_NOCHANGE = 0x00000001;

rBitsArray rSelector::m_flagsSetup;
rBitsArray rSelector::m_flagsMode;


//-------------------------------------------------------------------------------------------------
//
rSelector::rSelector() : Select(-1)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , SELECTOR_SETUP_OFF)
				.add("NOEVENT", SELECTOR_SETUP_NOEVENT);
	}

	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("NEXT"    , SELECTOR_MODE_CHANGENEXT)
				.add("PREV"    , SELECTOR_MODE_CHANGEPREV)
				.add("NOCHANGE", SELECTOR_MODE_NOCHANGE)
				.add("ERROR"   , SELECTOR_MODE_TOERROR);
	}

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
			faultGrp[ii] += FaultIn[ii][grp].Value != 0.0;
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
			ValueOut[grp].m_varName = NameInput[grp] + ".output";
			InitLink(rLink::Setup::OUTPUT | rLink::Setup::VARNAME, ValueOut[grp], ValueIn[0][grp].Unit, SID_SEL_GRP1_OUT + grp, NameInput[grp], rLink::SHADOW_NONE);
		}

		// Настройка входов
		for(UDINT grp = 0; grp < CountGroups; ++grp)
		{
			for(UDINT ii = 0; ii < CountInputs; ++ii)
			{
				std::string i_name = String_format("%s.input_%i"      , NameInput[grp].c_str(), ii + 1);
				std::string f_name = String_format("%s.input_%i.fault", NameInput[grp].c_str(), ii + 1);

				InitLink(rLink::Setup::INPUT                       , ValueIn[ii][grp], ValueIn[ii][grp].Unit, SID_SEL_GRP1_IN1  + grp * MAX_SELECTOR_INPUT + ii, i_name, rLink::SHADOW_NONE);
				InitLink(rLink::Setup::INPUT | rLink::Setup::SIMPLE, FaultIn[ii][grp], U_DIMLESS            , SID_SEL_GRP1_FLT1 + grp * MAX_SELECTOR_INPUT + ii, f_name, i_name            );
			}
		}
	}
	else
	{
		InitLink(rLink::Setup::OUTPUT, ValueOut[0], ValueIn[0][0].Unit, SID_SEL_OUT, "output", rLink::SHADOW_NONE);

		for(UDINT ii = 0; ii < CountInputs; ++ii)
		{
			string i_name = String_format("input_%i"      , ii + 1);
			string f_name = String_format("input_%i.fault", ii + 1);

			InitLink(rLink::Setup::INPUT                       , ValueIn[ii][0], ValueIn[ii][0].Unit, SID_SEL_IN1  + ii, i_name, rLink::SHADOW_NONE);
			InitLink(rLink::Setup::INPUT | rLink::Setup::SIMPLE, FaultIn[ii][0], U_DIMLESS          , SID_SEL_FLT1 + ii, f_name, i_name            );
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
UDINT rSelector::LoadFromXML(tinyxml2::XMLElement *element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(SELECTOR_SETUP_OFF));
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE, m_flagsMode.getNameByBits(SELECTOR_MODE_CHANGENEXT));

	if (TRITONN_RESULT_OK != rSource::LoadFromXML(element, err, prefix)) {
		return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "");
	}

	UDINT fault = 0;
	Setup.Init(m_flagsSetup.getValue(strSetup, fault));
	Mode.Init (m_flagsMode.getValue(strMode, fault));

	if (fault) {
		return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "");
	}

	// Простой селектор
	if(std::string(XmlName::SELECTOR) == element->Name())
	{
		tinyxml2::XMLElement* xml_inputs = element->FirstChildElement(XmlName::INPUTS);
		tinyxml2::XMLElement* xml_faults = element->FirstChildElement(XmlName::FAULTS);
		tinyxml2::XMLElement* xml_keypad = element->FirstChildElement(XmlName::KEYPAD);

		if(!xml_inputs || !xml_keypad) {
			return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "error inputs or keypad");
		}

		//------------------------------------
		// Входа
		UDINT ii = 0;
		XML_FOR(xml_link, xml_inputs, XmlName::LINK) {
			if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_link, ValueIn[ii][0])) {
				return err.getError();
			}
			++ii;

			if(ii >= MAX_SELECTOR_INPUT) {
				return err.set(DATACFGERR_SELECTOR, xml_link->GetLineNum(), "too much inputs");
			}
		}
		if (ii < 2) {
			return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "too few inputs");
		}

		CountGroups = 1;
		CountInputs = ii;
		ii          = 0;

		//------------------------------------
		// Фаулты (ошибки)
		if (xml_faults) {
			XML_FOR(xml_link, xml_faults, XmlName::LINK) {
				if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_link, FaultIn[ii][0], ValueIn[ii][0], "fault")) {
					return err.getError();
				}

				// Принудительно выключаем пределы
				FaultIn[ii][0].Limit.m_setup.Init(rLimit::Setup::OFF);

				if (++ii >= MAX_SELECTOR_INPUT) {
					return err.set(DATACFGERR_SELECTOR, xml_link->GetLineNum(), "too much faults");
				}
			}

			if (ii != CountInputs) {
				return err.set(DATACFGERR_SELECTOR, xml_faults->GetLineNum(), "error count faults");
			}
		}

		// Подстановочное значение
		Keypad[0] = XmlUtils::getTextLREAL(xml_keypad->FirstChildElement(XmlName::VALUE),   0.0, fault);
		KpUnit[0] = XmlUtils::getTextUDINT(xml_keypad->FirstChildElement(XmlName::UNIT) , U_any, fault);

		if (fault) {
			return err.set(DATACFGERR_SELECTOR, xml_faults->GetLineNum(), "");
		}
	}

	// Мульти-селектор
	else if (string(XmlName::MSELECTOR) == element->Name()) {
		Setup.Init(Setup.Value | SELECTOR_SETUP_MULTI);

		tinyxml2::XMLElement *xml_names   = element->FirstChildElement(XmlName::NAMES);
		tinyxml2::XMLElement *xml_inputs  = element->FirstChildElement(XmlName::INPUTS);
		tinyxml2::XMLElement *xml_faults  = element->FirstChildElement(XmlName::FAULTS);
		tinyxml2::XMLElement *xml_keypads = element->FirstChildElement(XmlName::KEYPADS);

		if (!xml_names || !xml_inputs || !xml_keypads) {
			return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "fault names or inputs or keypad");
		}

		// Загружаем имена выходов
		UDINT grp = 0;
		XML_FOR(xml_name, xml_names, XmlName::NAME) {
			NameInput[grp] = String_tolower(xml_name->GetText());

			if (NameInput[grp].empty()) {
				return err.set(DATACFGERR_SELECTOR, xml_name->GetLineNum(), "empty name");
			}

			++grp;
		}
		CountGroups = grp;

		// Загружаем входа
		UDINT ii = 0;
		XML_FOR(xml_group, xml_inputs, XmlName::GROUP) {
			// Линки
			grp = 0;
			XML_FOR(xml_link, xml_group, XmlName::LINK) {
				if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(xml_link, ValueIn[ii][grp])) {
					return err.getError();
				}

				++grp;

				if (grp >= MAX_SELECTOR_GROUP) {
					return err.set(DATACFGERR_SELECTOR, xml_link->GetLineNum(), "too much groups");
				}
			}

			if (grp != CountGroups) {
				return err.set(DATACFGERR_SELECTOR, xml_group->GetLineNum(), "fault count groups");
			}

			++ii;

			if (ii >= MAX_SELECTOR_INPUT) {
				return err.set(DATACFGERR_SELECTOR, xml_group->GetLineNum(), "too much inputs");
			}
		}
		if (ii < 2) {
			return err.set(DATACFGERR_SELECTOR, xml_inputs->GetLineNum(), "too few inputs");
		}

		CountInputs = ii;

		// Фаулты
		if (xml_faults) {
			ii = 0;
			XML_FOR(xml_group, xml_faults, XmlName::GROUP) {
				// Линки
				grp = 0;
				XML_FOR(xml_link, xml_group, XmlName::LINK) {
					if (TRITONN_RESULT_OK != rDataConfig::instance().LoadShadowLink(xml_link, FaultIn[ii][grp], ValueIn[ii][grp], XmlName::FAULT)) {
						return err.getError();
					}

					FaultIn[ii][grp].Limit.m_setup.Init(rLimit::Setup::OFF);

					if (++grp >= MAX_SELECTOR_GROUP) {
						return err.set(DATACFGERR_SELECTOR, xml_link->GetLineNum(), "too much fault group");
					}
				}

				if (grp != CountGroups) {
					return err.set(DATACFGERR_SELECTOR, xml_group->GetLineNum(), "fault count groups");
				}

				if (++ii >= MAX_SELECTOR_INPUT) {
					return err.set(DATACFGERR_SELECTOR, xml_group->GetLineNum(), "too much groups");
				}
			}

			if (ii != CountInputs) {
				return err.set(DATACFGERR_SELECTOR, xml_faults->GetLineNum(), "error count fault groups");
			}
		}

		// Подстановочные значения
		grp = 0;
		XML_FOR(xml_keypad, xml_keypads, XmlName::KEYPAD) {
			UDINT fault = 0;
			Keypad[grp] = XmlUtils::getTextLREAL(xml_keypad->FirstChildElement(XmlName::VALUE),   0.0, fault);
			KpUnit[grp] = XmlUtils::getTextUDINT(xml_keypad->FirstChildElement(XmlName::UNIT) , U_any, fault);

			++grp;
			if (grp >= MAX_SELECTOR_GROUP || fault) {
				return err.set(DATACFGERR_SELECTOR, xml_keypad->GetLineNum(), "too much keypads or error keypad");
			}
		}

		if (grp != CountGroups) {
			return err.set(DATACFGERR_SELECTOR, xml_keypads->GetLineNum(), "error count keypads");
		}
	} else {
		return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "empty keypads");
	}

	GenerateIO();

	return TRITONN_RESULT_OK;
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




