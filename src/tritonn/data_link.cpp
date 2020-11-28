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

#include <limits>
#include <cmath>
#include "tinyxml2.h"
#include "error.h"
//#include "data_config.h"
#include "variable_item.h"
#include "variable_list.h"
#include "log_manager.h"
#include "data_link.h"
#include "xml_util.h"


const std::string rLink::SHADOW_NONE = "";

///////////////////////////////////////////////////////////////////////////////////////////////////
//

rLink::~rLink()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
LREAL rLink::GetValue()
{
	UDINT err    = 0;
	LREAL result = 0;

	if(nullptr == Source)
	{
		err = 1;
		return 0;
	}

	result = Source->GetValue(Param, Unit, err);

	//TODO Нужна обработка ошибки

	return result;
}


STRID rLink::GetSourceUnit()
{
	UDINT err = 0;

	if(nullptr == Source)
	{
		err = 1;
		return U_any;
	}

	return Source->GetValueUnit(Param, err);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::GetFault(void)
{
	if(Source == nullptr)
	{
		return 1;
	}

	return Source->GetFault();
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::Calculate()
{
	UDINT err = 0;

	rSource::Calculate();

	if(nullptr == Source)
	{
		return 1;
	}

	// Получаем значение линка
	Value = Source->GetValue(Param, Unit, err);

	// Вычисляем пределы
	Limit.Calculate(Value, true);

	if(Value == std::numeric_limits<LREAL>::infinity() || Value == -std::numeric_limits<LREAL>::infinity() || std::isnan(Value))
	{
		Value = 0;
	}

	return err;
}


void rLink::CalculateLimit()
{
	Limit.Calculate(Value, true);
}


// Заглушка
UDINT rLink::InitLimitEvent(rLink &/*link*/)
{
	return 1;
}


// Заглушка
LREAL rLink::GetValue(const string &/*name*/, UDINT /*unit*/, UDINT &err)
{
	err = 1;
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
void rLink::Init(UINT setup, UDINT unit, rSource *owner, const string &ioname, STRID descr)
{
	Unit    = unit;
	Owner   = owner;
	IO_Name = ioname;
	Descr   = descr;
	m_setup = setup;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::generateVars(rVariableList& list)
{
	string name  = "";
	UINT   flags = rVariable::Flags::READONLY;

	if(nullptr == Owner)
	{
		TRACEERROR("The link '%s' has no owner.", Alias.c_str());
		return 0;
	}

	name = Owner->Alias;
	if(!(m_setup & Setup::NONAME))
	{
		if (m_setup & Setup::VARNAME) {
			name += "." + m_varName;
		} else {
			name += "." + IO_Name;
		}
	}

	if (m_setup & Setup::WRITABLE) {
		flags &= ~rVariable::Flags::READONLY;
	}

	if (m_setup & Setup::SIMPLE) {
		list.add(name, TYPE_LREAL, static_cast<rVariable::Flags>(flags), &Value, Unit, 0);
	} else {
		list.add(name + ".value", TYPE_LREAL, static_cast<rVariable::Flags>(flags), &Value        , Unit     , 0);
		list.add(name + ".unit" , TYPE_STRID, rVariable::Flags::R___              ,  Unit.GetPtr(), U_DIMLESS, 0);

		Limit.generateVars(list, name, Unit);
	}

	return TRITONN_RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rLink::LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	UNUSED(prefix);

	std::string* curstr = &Alias;

	FullTag   = XmlUtils::getAttributeString(element, XmlName::ALIAS, "");
	m_lineNum = element->GetLineNum();

	if (FullTag.empty()) {
		return err.set(DATACFGERR_LINK, element->GetLineNum(), "tag is empty");
	}

	// Делим полное имя на имя объекта и имя параметра (разбираем строчку "xxx:yyy")
	for (auto ch : FullTag) {
		if (ch == ':') {
			// Двоеточие встретилось повторно
			if (curstr == &Param) {
				return err.set(DATACFGERR_LINK, element->GetLineNum(), "tag name is fault");
			}

			curstr = &Param;
			continue;
		}

		*curstr += ch;
	}

	// Загружаем пределы
	tinyxml2::XMLElement* limits = element->FirstChildElement(XmlName::LIMITS);

	if (limits) {
		if (TRITONN_RESULT_OK != Limit.LoadFromXML(limits, err, "")) {
			return err.getError();
		}
	} else {
		Limit.m_setup.Init(rLimit::Setup::OFF);
	}

	return TRITONN_RESULT_OK;
}




