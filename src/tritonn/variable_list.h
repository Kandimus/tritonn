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
#include "variable_item.h"

class rVariable;
class rVariableClass;

class rVariableList
{
	friend class rVariableClass;

public:
	rVariableList();
	virtual ~rVariableList();

	void  add(const std::string& name, TYPE type, rVariable::Flags flags, void* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags,  SINT* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags, USINT* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags,   INT* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags,  UINT* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags,  DINT* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags, UDINT* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags,  REAL* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags, LREAL* pointer, STRID unit, UDINT access, const std::string& comment);
	void  add(const std::string& name, rVariable::Flags flags, STRID* pointer, STRID unit, UDINT access, const std::string& comment);
	void  sort();
	void  deleteAll(void);
	UDINT saveToCSV(const std::string& path);
	rVariable* find(const std::string& name);

	size_t size() const { return m_list.size(); }
	std::vector<rVariable*>::iterator begin() { return m_list.begin(); }
	std::vector<rVariable*>::iterator end()   { return m_list.end();   }

	std::string getMarkDown() const;

private:
	std::vector<rVariable*> m_list;
};


