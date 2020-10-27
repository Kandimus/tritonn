//=================================================================================================
//===
//=== variable_class.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс доступа к объектам и переменным из внешних источников. Массив данных структур
//=== заполняется на старте приложения и остается не изменным.
//===
//=================================================================================================

#pragma once

#include "def.h"
#include "variable_list.h"


class rVariableClass
{
public:
	rVariableClass();
	virtual ~rVariableClass();

	virtual UDINT processing();

	rVariableClass* getVariableClass() { return this; }

	const rVariable* findVar(const std::string& name);

protected:
	rVariableList m_varList;
};

