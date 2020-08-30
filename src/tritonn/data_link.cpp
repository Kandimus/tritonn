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
#include "data_config.h"
#include "data_variable.h"
#include "log_manager.h"
#include "data_link.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rLink::rLink()
{
	Source  = nullptr;
	Owner   = nullptr;
	Param   = "";
	FullTag = "";
	Unit    = U_any;
	Value   = 0.0;
	Shadow  = "";

	Outputs.clear();
	Inputs.clear();
}


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
	Setup   = setup;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rLink::GenerateVars(vector<rVariable *> &list)
{
	string name  = "";
	UINT   flags = VARF_R___;

	if(nullptr == Owner)
	{
		TRACEERROR("The link '%s' has no owner.", Alias);
		return 0;
	}

	name = Owner->Alias;
	if(!(Setup & LINK_SETUP_NONAME))
	{
		name += "." + IO_Name;
	}

	if(Setup & LINK_SETUP_WRITEBLE)
	{
		flags &= ~VARF_READONLY;
	}

	if(Setup & LINK_SETUP_SIMPLE)
	{
		list.push_back(new rVariable(name, TYPE_LREAL, flags, &Value, Unit, 0));
	}
	else
	{
		list.push_back(new rVariable(name + ".value", TYPE_LREAL, flags    , &Value        , Unit     , 0));
		list.push_back(new rVariable(name + ".unit" , TYPE_STRID, VARF_R___,  Unit.GetPtr(), U_DIMLESS, 0));

		Limit.GenerateVars(list, name, Unit);
	}

	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rLink::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	string *curstr = &Alias;

	FullTag = (element->Attribute(CFGNAME_ALIAS) ) ? element->Attribute(CFGNAME_ALIAS)  : "";

	if(FullTag.empty()) return DATACFGERR_LINK;

	// Делим полное имя на имя объекта и имя параметра (разбираем строчку "xxx:yyy")
	for(UDINT ii = 0; ii < FullTag.size(); ++ii)
	{
		if(':' == FullTag[ii])
		{
			// Двоеточие встретилось повторно
			if(curstr == &Param)
			{
				return DATACFGERR_LINK;
			}

			curstr = &Param;
			continue;
		}

		*curstr += FullTag[ii];
	}

	// Загружаем пределы
	tinyxml2::XMLElement *limits = element->FirstChildElement(CFGNAME_LIMITS);

	if(limits)
	{
		if(tinyxml2::XML_SUCCESS != Limit.LoadFromXML(limits, cfg))
		{
			return DATACFGERR_AI;
		}
	}
	else
	{
		Limit.Setup.Init(LIMIT_SETUP_OFF);
	}

	return tinyxml2::XML_SUCCESS;
}




