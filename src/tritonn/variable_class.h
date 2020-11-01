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

	UDINT getAllVariables(rSnapshot& snapshot);
	const rVariable* findVar(const std::string& name);

protected:
	UDINT writeExt(rVariableList& varlist);
	UDINT readExt(rVariableList& varlist);
	UDINT addExternal(rVariableList& varlist);
	UDINT linkToExternal(rVariableClass* extlist);

protected:
	rVariableClass*  m_linkClass = nullptr;
	rVariableList    m_varList;
	rSnapshot*       m_snapshot;
	pthread_mutex_t* m_mutex = nullptr;
};

