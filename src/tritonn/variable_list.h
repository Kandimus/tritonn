//=================================================================================================
//===
//=== variable_list.h
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
#include <vector>
#include <string>

class rVariable;

class rVariableList
{
public:
	rVariableList();
	virtual ~rVariableList();

	void  add(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access);
	void  add(const rVariable* var);
	void  add(const rVariable& var);
	bool  addExternal(const rVariableList& varlist);
	void  sort();
	void  deleteAll(void);
	UDINT saveToCSV(const std::string& path);
	const rVariable* find(const std::string& name);

	size_t size() const { return m_list.size(); }
	std::vector<const rVariable*>::iterator begin() { return m_list.begin(); }
	std::vector<const rVariable*>::iterator end()   { return m_list.end();   }

private:
	std::vector<const rVariable*> m_list;
};


