//=================================================================================================
//===
//=== variable_item.cpp
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

#include "variable_item.h"
#include <algorithm>
#include <string.h>


rVariable::rVariable(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access)
{
	m_name    = String_tolower(name);
	m_type    = type;
	m_flags   = flags;
	m_pointer = pointer;
	m_hash    = std::hash<std::string>{}(m_name);
	m_unit    = unit;
	m_access  = access;
}


//-------------------------------------------------------------------------------------------------
// Конструктор удаляет все дерево переменных, включая дочерние и соседние узлы
rVariable::~rVariable()
{
}

bool rVariable::getBuffer(void* buffer) const
{
	if(m_pointer == nullptr) {
		return false;
	}

	memcpy(buffer, m_pointer, EPT_SIZE[m_type]);
	return true;
}

bool rVariable::setBuffer(void* buffer) const
{
	if(m_pointer == nullptr) {
		return false;
	}

	memcpy(m_pointer, buffer, EPT_SIZE[m_type]);
	return true;
}
