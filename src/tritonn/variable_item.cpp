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

rVariable::rVariable(rVariable *var)
{
	if (!var) {
		return;
	}

	*this = *var;

	var->m_extVar = this;
	m_pointer     = nullptr;
	m_flags      |= Flags::EXTERNAL;
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

std::string rVariable::saveToCSV()
{
	if (isHide()) {
		return "";
	}

	return String_format("%s;%s;%#06x;%08X;%#010x;\n", m_name.c_str(), NAME_TYPE[m_type].c_str(), m_flags, m_access, m_hash);
}

bool operator < (const rVariable* left, const rVariable* right)
{
	return left->m_hash < right->m_hash;
}
