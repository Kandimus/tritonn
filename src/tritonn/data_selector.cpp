//=================================================================================================
//===
//=== data_selector.h
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================

#include "data_selector.h"
#include <string.h>
#include "event/eid.h"
#include "event/manager.h"
#include "text_id.h"
#include "error.h"
#include "data_manager.h"
#include "data_config.h"
#include "data_link.h"
#include "variable_list.h"
#include "xml_util.h"
#include "generator_md.h"
#include "comment_defines.h"


const UDINT SELECTOR_LE_NOCHANGE = 0x00000001;

rBitsArray rSelector::m_flagsSetup;
rBitsArray rSelector::m_flagsMode;


//-------------------------------------------------------------------------------------------------
//
rSelector::rSelector(const rStation* owner) : rSource(owner), m_select(-1)
{
	if (m_flagsSetup.empty()) {
		m_flagsSetup
				.add("OFF"    , static_cast<UINT>(Setup::OFF)    , COMMENT::SETUP_OFF)
				.add("NOEVENT", static_cast<UINT>(Setup::NOEVENT), "Запретить выдачу сообщений");
	}

	if (m_flagsMode.empty()) {
		m_flagsMode
				.add("NOCHANGE", static_cast<UINT>(Mode::NOCHANGE)  , "При аварии не переходить на другой вход")
				.add("ERROR"   , static_cast<UINT>(Mode::TOERROR)   , "При аварии переходить на аварийное значение")
				.add("PREV"    , static_cast<UINT>(Mode::CHANGEPREV), "При аварии переключить на предыдущий вход")
				.add("NEXT"    , static_cast<UINT>(Mode::CHANGENEXT), "При аварии переключить на следующий вход");
	}

	//TODO Нужно ли очищать свойства класса?
	m_lockErr   = 0;
	CountGroups = MAX_GROUPS; //  По умолчанию доступен максимальный селектор
	CountInputs = MAX_INPUTS;

	for(UDINT grp = 0; grp < MAX_GROUPS; ++grp) {
		Keypad[grp]    = 0.0;
		KpUnit[grp]    = U_any;
		NameInput[grp] = String_format("#output_%u_name", grp + 1);
	}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//NOTE Таблицу выходов не заполняем, т.к. все будет зависить от загрузки, заполним там.
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::initLimitEvent(rLink &link)
{
	link.m_limit.EventChangeAMin  = reinitEvent(EID_SELECTOR_NEW_AMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMin  = reinitEvent(EID_SELECTOR_NEW_WMIN)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeWMax  = reinitEvent(EID_SELECTOR_NEW_WMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeAMax  = reinitEvent(EID_SELECTOR_NEW_AMAX)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeHyst  = reinitEvent(EID_SELECTOR_NEW_HYST)  << link.m_descr << link.m_unit;
	link.m_limit.EventChangeSetup = reinitEvent(EID_SELECTOR_NEW_SETUP) << link.m_descr << link.m_unit;
	link.m_limit.EventAMin        = reinitEvent(EID_SELECTOR_AMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMin        = reinitEvent(EID_SELECTOR_WMIN)      << link.m_descr << link.m_unit;
	link.m_limit.EventWMax        = reinitEvent(EID_SELECTOR_WMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventAMax        = reinitEvent(EID_SELECTOR_AMAX)      << link.m_descr << link.m_unit;
	link.m_limit.EventNan         = reinitEvent(EID_SELECTOR_NAN)       << link.m_descr << link.m_unit;
	link.m_limit.EventNormal      = reinitEvent(EID_SELECTOR_NORMAL)    << link.m_descr << link.m_unit;

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::calculate()
{
	UDINT faultGrp[MAX_INPUTS] = {0};

	if (rSource::calculate()) {
		return TRITONN_RESULT_OK;
	}

	if (m_setup.Value & Setup::OFF) {
		return TRITONN_RESULT_OK;
	}

	for (UDINT ii = 0; ii < CountInputs; ++ii) {
		for (UDINT grp = 0; grp < CountGroups; ++grp) {
			faultGrp[ii] += FaultIn[ii][grp].m_value != 0.0;
		}
	}

	// Проверка на изменение данных пользователем
	m_select.Compare(reinitEvent(EID_SELECTOR_SELECTED));
	m_mode.Compare(reinitEvent(EID_SELECTOR_MODE));

	// Если переменная переключатель находится в недопустимом режиме
	//TODO Какие значения будут в выходах?
	if (m_select.Value >= CountInputs || m_select.Value < -1) {
		rEventManager::instance().add(reinitEvent(EID_SELECTOR_ERROR) << m_select.Value);
		m_select.Init(-1);
		m_fault = 1;
		//return 0;
	}

	//----------------------------------------------------------------------------------------
	// Автоматические переходы, по статусу ошибки
	if (m_select.Value != -1) {
		if (faultGrp[m_select.Value]) {
			switch (m_mode.Value) {
				// Переходы запрещены
				case Mode::NOCHANGE: {
					sendEventSetLE(SELECTOR_LE_NOCHANGE, reinitEvent(EID_SELECTOR_NOCHANGE) << m_select.Value);
					break;
				}

				// Переход на значение ошибки
				case Mode::TOERROR: {
					rEventManager::instance().add(reinitEvent(EID_SELECTOR_TOFAULT) << m_select.Value);

					m_select.Value = -1;
					m_fault      = 1;
				}

				// Переходим на следующий канал
				default: {
					DINT count = 0;

					m_lockErr &= ~(SELECTOR_LE_NOCHANGE);

					do {
						m_select.Value += (m_mode.Value == Mode::CHANGENEXT) ? +1 : -1;
					
						if(m_select.Value < 0)           m_select.Value = CountInputs - 1;
						if(m_select.Value > CountInputs) m_select.Value = 0;
						
						++count;
					} while (faultGrp[m_select.Value] && count < CountInputs - 2);

					if (faultGrp[m_select.Value]) {
						rEventManager::instance().add(reinitEvent(EID_SELECTOR_NOTHINGNEXT) << m_select.Value);

						m_select.Value = -1;
					} else {
						rEventManager::instance().add(reinitEvent(EID_SELECTOR_TONEXT) << m_select.Value);
					}
				}
			} //switch
		} else {
			sendEventClearLE(SELECTOR_LE_NOCHANGE, reinitEvent(EID_SELECTOR_CLEARERROR) << m_select.Value);
		}
	}

	// Записываем в "выхода" требуемые значения входов, так же изменяем ед. измерения у "выходов"
	for (UDINT grp = 0; grp < CountGroups; ++grp) {
		ValueOut[grp].m_value = (m_select.Value == -1) ? Keypad[grp] : ValueIn[m_select.Value][grp].m_value;
		ValueOut[grp].m_unit  = (m_select.Value == -1) ? KpUnit[grp] : ValueIn[m_select.Value][grp].m_unit;
	}

	m_fault = (m_select.Value == -1) ? 1 : faultGrp[m_select.Value];

	postCalculate();

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Генерация Inputs/Outputs
void rSelector::generateIO()
{
	m_inputs.clear();
	m_outputs.clear();

	if (m_setup.Value & Setup::MULTI) {
		// Настройка выходов
		for (UDINT grp = 0; grp < CountGroups; ++grp) {
			ValueOut[grp].m_varName = NameInput[grp] + ".output";
			initLink(rLink::Setup::OUTPUT | rLink::Setup::VARNAME, ValueOut[grp], ValueIn[0][grp].m_unit, SID::SEL_GRP1_OUT + grp, NameInput[grp], rLink::SHADOW_NONE);
		}

		// Настройка входов
		for (UDINT grp = 0; grp < CountGroups; ++grp) {
			for (UDINT ii = 0; ii < CountInputs; ++ii) {
				std::string i_name = String_format("%s.input_%i"      , NameInput[grp].c_str(), ii + 1);
				std::string f_name = String_format("%s.input_%i.fault", NameInput[grp].c_str(), ii + 1);

				initLink(rLink::Setup::INPUT                       , ValueIn[ii][grp], ValueIn[ii][grp].m_unit, SID::SEL_GRP1_IN1  + grp * MAX_INPUTS + ii, i_name, rLink::SHADOW_NONE);
				initLink(rLink::Setup::INPUT | rLink::Setup::SIMPLE, FaultIn[ii][grp], U_DIMLESS              , SID::SEL_GRP1_FLT1 + grp * MAX_INPUTS + ii, f_name, i_name            );
			}
		}
	} else {
		initLink(rLink::Setup::OUTPUT, ValueOut[0], ValueIn[0][0].m_unit, SID::OUTPUT, XmlName::OUTPUT, rLink::SHADOW_NONE);

		for (UDINT ii = 0; ii < CountInputs; ++ii) {
			string i_name = String_format("input_%i"      , ii + 1);
			string f_name = String_format("input_%i.fault", ii + 1);

			initLink(rLink::Setup::INPUT                       , ValueIn[ii][0], ValueIn[ii][0].m_unit, SID::INPUT_1 + ii, i_name, rLink::SHADOW_NONE);
			initLink(rLink::Setup::INPUT | rLink::Setup::SIMPLE, FaultIn[ii][0], U_DIMLESS            , SID::FAULT_1 + ii, f_name, i_name            );
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

	list.add(m_alias + ".Select"    , rVariable::Flags::___D, &m_select.Value, U_DIMLESS, ACCESS_SELECT, "Выбор коммуцируемого входа");
	list.add(m_alias + ".inputcount", rVariable::Flags::R___, &CountInputs   , U_DIMLESS, 0            , "Количество подключенных входов");
	list.add(m_alias + ".Setup"     , rVariable::Flags::RS__, &m_setup.Value , U_DIMLESS, ACCESS_SA    , COMMENT::SETUP + m_flagsSetup.getInfo());
	list.add(m_alias + ".Mode"      , rVariable::Flags::___D, &m_mode.Value  , U_DIMLESS, ACCESS_SELECT, COMMENT::MODE + m_flagsMode.getInfo(true));

	list.add(m_alias + ".fault"     , rVariable::Flags::R___, &m_fault       , U_DIMLESS, 0            , COMMENT::FAULT);

	// Мультиселектор
	if (m_setup.Value & Setup::MULTI) {
		list.add(m_alias + ".selectorcount", TYPE::UINT, rVariable::Flags::R___, &CountGroups , U_DIMLESS, 0, "Количество групп");

		for (UDINT grp = 0; grp < CountGroups; ++grp) {
			alias_unit   = String_format("%s.%s.keypad.unit" , m_alias.c_str(), NameInput[grp].c_str());
			alias_keypad = String_format("%s.%s.keypad.value", m_alias.c_str(), NameInput[grp].c_str());

			list.add(alias_unit  , TYPE::STRID, rVariable::Flags::R___,  KpUnit[grp].getPtr(), U_DIMLESS  , 0            , String_format("Группа %u. ", grp) + COMMENT::KEYPAD + ". Единицы измерения");
			list.add(alias_keypad, TYPE::LREAL, rVariable::Flags::___D, &Keypad[grp]         , KpUnit[grp], ACCESS_KEYPAD, String_format("Группа %u. ", grp) + COMMENT::KEYPAD);
		}
	} else {
		list.add(m_alias + ".keypad.unit" , TYPE::STRID, rVariable::Flags::R___,  KpUnit[0].getPtr(), U_DIMLESS, 0            , COMMENT::KEYPAD + ". Единицы измерения");
		list.add(m_alias + ".Keypad.value", TYPE::LREAL, rVariable::Flags::___D, &Keypad[0]         , KpUnit[0], ACCESS_KEYPAD, COMMENT::KEYPAD);
	}

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rSelector::loadFromXML(tinyxml2::XMLElement *element, rError& err, const std::string& prefix)
{
	std::string strSetup = XmlUtils::getAttributeString(element, XmlName::SETUP, m_flagsSetup.getNameByBits(Setup::OFF));
	std::string strMode  = XmlUtils::getAttributeString(element, XmlName::MODE, m_flagsMode.getNameByBits(Mode::CHANGENEXT));

	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "");
	}

	UDINT fault = 0;
	m_setup.Init(m_flagsSetup.getValue(strSetup, fault));
	m_mode.Init (m_flagsMode.getValue(strMode, fault));
	m_select.Init(XmlUtils::getAttributeINT(element, XmlName::SELECT, m_select.Value));

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

			if(ii >= MAX_INPUTS) {
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
				FaultIn[ii][0].m_limit.m_setup.Init(rLimit::Setup::OFF);

				if (FaultIn[ii][0].m_param.empty()) {
					FaultIn[ii][0].m_param = XmlName::FAULT;
				}

				if (++ii >= MAX_INPUTS) {
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
		m_setup.Init(m_setup.Value | Setup::MULTI);

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

				if (grp >= MAX_GROUPS) {
					return err.set(DATACFGERR_SELECTOR, xml_link->GetLineNum(), "too much groups");
				}
			}

			if (grp != CountGroups) {
				return err.set(DATACFGERR_SELECTOR, xml_group->GetLineNum(), "fault count groups");
			}

			++ii;

			if (ii >= MAX_INPUTS) {
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

					FaultIn[ii][grp].m_limit.m_setup.Init(rLimit::Setup::OFF);

					// Если параметр не задан, то принудетельно выставляем в fault, для того чтобы
					// значение бралось из fault а не из выхода по умолчанию
					if (FaultIn[ii][grp].m_param.empty()) {
						FaultIn[ii][grp].m_param = XmlName::FAULT;
					}

					if (++grp >= MAX_GROUPS) {
						return err.set(DATACFGERR_SELECTOR, xml_link->GetLineNum(), "too much fault group");
					}
				}

				if (grp != CountGroups) {
					return err.set(DATACFGERR_SELECTOR, xml_group->GetLineNum(), "fault count groups");
				}

				if (++ii >= MAX_INPUTS) {
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
			if (grp >= MAX_GROUPS || fault) {
				return err.set(DATACFGERR_SELECTOR, xml_keypad->GetLineNum(), "too much keypads or error keypad");
			}
		}

		if (grp != CountGroups) {
			return err.set(DATACFGERR_SELECTOR, xml_keypads->GetLineNum(), "error count keypads");
		}
	} else {
		return err.set(DATACFGERR_SELECTOR, element->GetLineNum(), "empty keypads");
	}

	generateIO();

	return TRITONN_RESULT_OK;
}

UDINT rSelector::generateMarkDown(rGeneratorMD& md)
{
	if (m_setup.Value & Setup::MULTI) {
		rGeneratorMD::rItem& mdi = md.add(this, false, rGeneratorMD::Type::CALCULATE)
				.addProperty(XmlName::SETUP, &m_flagsSetup)
				.addProperty(XmlName::MODE, &m_flagsMode, true)
				.addProperty(XmlName::SELECT, static_cast<LREAL>(m_select.Value))
				.addXml("<" + std::string(XmlName::NAMES) + ">");

		for (auto grp = 0; grp < MAX_GROUPS; ++grp) {
			mdi.addXml(XmlName::NAME, String_format("valid output %u name", grp), false, "\t");
		}
		mdi.addXml("</" + std::string(XmlName::NAMES) + ">");

		mdi.addXml("<" + std::string(XmlName::INPUTS) + ">");
		for (auto ii = 0; ii < MAX_INPUTS; ++ii) {
			mdi.addXml("\t<" + std::string(XmlName::GROUP) + ">");

			for (auto grp = 0; grp < MAX_GROUPS; ++grp) {
				mdi.addXml("\t\t" + std::string(rGeneratorMD::rItem::XML_LINK));
			}
			mdi.addXml("\t</" + std::string(XmlName::GROUP) + ">");
		}
		mdi.addXml("</" + std::string(XmlName::INPUTS) + ">");

		mdi.addXml("<" + std::string(XmlName::FAULTS) + ">" + rGeneratorMD::rItem::XML_OPTIONAL);
		for (auto ii = 0; ii < MAX_INPUTS; ++ii) {
			mdi.addXml("\t<" + std::string(XmlName::GROUP) + ">");

			for (auto grp = 0; grp < MAX_GROUPS; ++grp) {
				mdi.addXml("\t\t" + std::string(rGeneratorMD::rItem::XML_LINK));
			}
			mdi.addXml("\t</" + std::string(XmlName::GROUP) + ">");
		}
		mdi.addXml("</" + std::string(XmlName::FAULTS) + ">");

		mdi.addXml("<" + std::string(XmlName::KEYPADS) + ">");
		for (auto grp = 0; grp < MAX_GROUPS; ++grp) {
			mdi.addXml("\t<" + std::string(XmlName::KEYPAD) + ">");
			mdi.addXml(XmlName::UNIT , static_cast<UDINT>(0), false, "\t\t");
			mdi.addXml(XmlName::VALUE, 0.0, false, "\t\t");
			mdi.addXml("\t</" + std::string(XmlName::KEYPAD) + ">");
		}
		mdi.addXml("</" + std::string(XmlName::KEYPADS) + ">")
				.addRemark("В каждой группе количество входов должно совпадать!");

	} else {
		md.add(this, false, rGeneratorMD::Type::CALCULATE)
				.addProperty(XmlName::SETUP, &m_flagsSetup)
				.addProperty(XmlName::MODE, &m_flagsMode, true)
				.addProperty(XmlName::SELECT, static_cast<LREAL>(m_select.Value))
				.addXml("<" + std::string(XmlName::INPUTS) + ">")
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK))
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK))
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK) + " " + rGeneratorMD::rItem::XML_OPTIONAL)
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK) + " " + rGeneratorMD::rItem::XML_OPTIONAL)
				.addXml("</" + std::string(XmlName::INPUTS) + ">")
				.addXml("<" + std::string(XmlName::FAULTS) + "> " + rGeneratorMD::rItem::XML_OPTIONAL)
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK))
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK))
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK) + " " + rGeneratorMD::rItem::XML_OPTIONAL)
				.addXml("\t" + std::string(rGeneratorMD::rItem::XML_LINK) + " " + rGeneratorMD::rItem::XML_OPTIONAL)
				.addXml("</" + std::string(XmlName::FAULTS) + ">")
				.addXml("<" + std::string(XmlName::KEYPAD) + ">")
				.addXml(XmlName::UNIT , KpUnit[0].toUDINT(), false, "\t")
				.addXml(XmlName::VALUE, Keypad[0], false, "\t")
				.addXml("</" + std::string(XmlName::KEYPAD) + ">")
				.addRemark("Количество входных значений не должно быть меньше чем количество флагов ошибки входа!");
	}

	return TRITONN_RESULT_OK;
}


