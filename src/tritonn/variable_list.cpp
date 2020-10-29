//=================================================================================================
//===
//=== variable_list.cpp
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

#include "variable_list.h"
#include "variable_item.h"
#include "simplefile.h"


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
	std::sort(m_list.begin(), m_list.end(), [](const rVariable *a, const rVariable *b){ return a < b; });
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
		text += var->saveToCSV();
	}

	return SimpleFileSave(path + ".variable.csv", text);
}


