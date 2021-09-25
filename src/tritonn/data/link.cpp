//=================================================================================================
//===
//=== data_link.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс линка на объект rSource, где требуется не дефолтные входа/выхода
//===
//=================================================================================================

#include "link.h"
#include <limits>
#include <cmath>
#include "tinyxml2.h"
#include "log_manager.h"
#include "xml_util.h"
#include "../error.h"
#include "../variable_list.h"
#include "../text_manager.h"
#include "../comment_defines.h"

const std::string rLink::SHADOW_NONE = "";

///////////////////////////////////////////////////////////////////////////////////////////////////
//

bool rLink::isValid() const
{
	return /*m_owner &&*/ m_source;
}

//-------------------------------------------------------------------------------------------------
//
LREAL rLink::getValue()
{
	UDINT err    = 0;
	LREAL result = 0;

	if (!m_source) {
		err = 1;
		return 0;
	}

	result = m_source->getValue(m_param, m_unit, err);

	//TODO Нужна обработка ошибки
	return result;
}


STRID rLink::getSourceUnit()
{
	UDINT err = 0;

	if (!isValid()) {
		err = 1;
		return U_any;
	}

	return m_source->getValueUnit(m_param, err);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::getFault(void)
{
	if (!isValid()) {
		return 1;
	}

	return m_source->getFault();
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::calculate()
{
	UDINT err = 0;

	rSource::calculate();

	if (!isValid()) {
		return 1;
	}

	if (m_setup & Setup::DISABLE) {
		return TRITONN_RESULT_OK;
	}

	// Получаем значение линка
	m_value = m_source->getValue(m_param, m_unit, err);

	// Вычисляем пределы
	m_limit.calculate(m_value, true);

	if(m_value == std::numeric_limits<LREAL>::infinity() || m_value == -std::numeric_limits<LREAL>::infinity() || std::isnan(m_value)) {
		m_value = 0;
	}

	return err;
}


void rLink::calculateLimit()
{
	m_limit.calculate(m_value, true);
}


// Заглушка
UDINT rLink::initLimitEvent(rLink &/*link*/)
{
	return 1;
}


// Заглушка
LREAL rLink::getValue(const string &/*name*/, UDINT /*unit*/, UDINT &err)
{
	err = 1;
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
void rLink::init(UINT setup, UDINT unit, rSource *owner, const std::string& ioname, STRID descr)
{
	auto comment = rTextManager::instance().getPtr(descr, LANG_RU);

	m_unit    = unit;
	m_owner   = owner;
	m_ioName  = ioname;
	m_descr   = descr;
	m_setup   = setup;
	m_comment = comment ? *comment : COMMENT::FAULT_STRID;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::generateVars(rVariableList& list)
{
	string name  = "";
	UINT   flags = rVariable::Flags::READONLY;

	if (!m_owner) {
		TRACEP(LOG::LINK, "The link '%s' has no owner.", m_alias.c_str());
		return 0;
	}

	name = m_owner->m_alias;
	if (!(m_setup & Setup::NONAME)) {
		if (m_setup & Setup::VARNAME) {
			name += "." + m_varName;
		} else {
			name += "." + m_ioName;
		}
	}

	if (m_setup & Setup::WRITABLE) {
		flags &= ~rVariable::Flags::READONLY;
	}

	if (m_setup & Setup::MUSTVIRT) {
		flags |= rVariable::Flags::MUTABLE;
	}

	if (m_setup & Setup::SIMPLE) {
		list.add(name, TYPE::LREAL, static_cast<rVariable::Flags>(flags), &m_value, m_unit, 0, m_comment);
	} else {
		list.add(name + ".value", TYPE::LREAL, static_cast<rVariable::Flags>(flags), &m_value        , m_unit   , 0, m_comment + ". Текущее значение");
		list.add(name + ".unit" , TYPE::STRID, rVariable::Flags::R___              ,  m_unit.getPtr(), U_DIMLESS, 0, m_comment + ". Единицы измерения");

		m_limit.generateVars(list, name, m_unit, m_comment);
	}

	return TRITONN_RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rLink::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	UNUSED(prefix);

	std::string* curstr = &m_alias;

	m_fullTag = XmlUtils::getAttributeString(element, XmlName::ALIAS, "");
	m_lineNum = element->GetLineNum();

	if (m_fullTag.empty()) {
		return err.set(DATACFGERR_LINK, element->GetLineNum(), "tag is empty");
	}

	m_fullTag = String_tolower(m_fullTag);

	// Делим полное имя на имя объекта и имя параметра (разбираем строчку "xxx:yyy")
	for (auto ch : m_fullTag) {
		if (ch == ':') {
			// Двоеточие встретилось повторно
			if (curstr == &m_param) {
				return err.set(DATACFGERR_LINK, element->GetLineNum(), "tag name is fault");
			}

			curstr = &m_param;
			continue;
		}

		*curstr += ch;
	}

	// Загружаем пределы
	tinyxml2::XMLElement* limits = element->FirstChildElement(XmlName::LIMITS);

	if (limits) {
		if (TRITONN_RESULT_OK != m_limit.loadFromXML(limits, err, "")) {
			return err.getError();
		}
	} else {
		m_limit.m_setup.Init(rLimit::Setup::OFF);
	}

	return TRITONN_RESULT_OK;
}

