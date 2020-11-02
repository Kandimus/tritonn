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
class rVariableClass;

class rVariableList
{
	friend class rVariableClass;

public:
	rVariableList();
	virtual ~rVariableList();

	void  add(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access);
	void  sort();
	void  deleteAll(void);
	UDINT saveToCSV(const std::string& path);
	rVariable* find(const std::string& name);

	size_t size() const { return m_list.size(); }
	std::vector<rVariable*>::iterator begin() { return m_list.begin(); }
	std::vector<rVariable*>::iterator end()   { return m_list.end();   }

private:
	std::vector<rVariable*> m_list;
};


