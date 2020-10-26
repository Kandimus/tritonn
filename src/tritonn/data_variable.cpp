﻿//=================================================================================================
//===
//=== data_variable.cpp
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

#include "data_variable.h"
#include <algorithm>
#include "simplefile.h"
//#include <cctype>
//#include <clocale>
//#include <limits>
//#include "string.h"
//#include <stdint.h>
//#include "data_source.h"
//#include "structures.h"


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



rVariableList::rVariableList()
{
}

rVariableList::~rVariableList()
{
	for (auto var : m_list) {
		if (var) {
			delete var;
		}
	}
	m_list.clear();
}

void rVariableList::add(const std::string& name, TT_TYPE type, UINT flags, void* pointer, STRID unit, UDINT access)
{
	const rVariable* var = find(name);

	if (var) {
		//TODO Выдать сообщение
		return;
	}

	m_list.push_back(new rVariable(name, type, flags, pointer, unit, access));
}

void rVariableList::addExternal(const rVariableList& varlist)
{
	for (auto var : varlist.m_list) {
		if (find(var->m_name)) {
			//TODO Выдать сообщение
			continue;
		}

		add(var->m_name, var->m_type, var->m_flags | rVariable::Flags::EXTERNAL, nullptr, var->m_unit, var->m_access);
	}
}


//
const rVariable* rVariableList::find(const string &name)
{
	std::string namelower = String_tolower(name);
	UDINT  hash = std::hash<std::string>{}(namelower);

	for(auto var : m_list) {
		if(hash == var->m_hash)
		{
			if(var->m_name == namelower)
			{
				return var;
			}
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Сортируем список по hash, для более быстрого поиска в будущем
void rVariableList::sort()
{
	std::sort(m_list.begin(), m_list.end(), [](const rVariable *a, const rVariable *b){ return a->m_hash < b->m_hash;});
}


//
void rVariableList::deleteAll()
{
	for(auto var : m_list)
	{
		if (var) {
			delete var;
		}
	}
	m_list.clear();
}


UDINT rVariableList::saveToCSV(const std::string& path)
{
	std::string text = ":alias;type;flags;access;alias hash;\n";

	for (auto var : m_list) {
		if(var->m_flags & rVariable::Flags::HIDE) {
			continue;
		}

		text += String_format("%s;%s;%#06x;%08X;%#010x;\n", var->m_name.c_str(), NAME_TYPE[var->m_type].c_str(), var->m_flags, var->m_access, var->m_hash);
	}

	return SimpleFileSave(path + ".variable.csv", text);
}


