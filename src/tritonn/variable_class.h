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

class rSnapshot;

class rVariableClass
{
public:
	rVariableClass(pthread_mutex_t& mutex);
	virtual ~rVariableClass();

	virtual UDINT processing();

	rVariableClass* getVariableClass() { return this; }

	UDINT get(rSnapshot& snapshot);
	UDINT set(rSnapshot& snapshot);
	UDINT addExternal(rVariableList& varlist);
	UDINT getAllVariables(rSnapshot& snapshot);
	const rVariable* findVar(const std::string& name);

protected:
	rVariableList    m_varList;
	pthread_mutex_t* m_mutex = nullptr;
};

