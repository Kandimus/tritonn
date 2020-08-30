//=================================================================================================
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

#include <algorithm>
#include <cctype>
#include <clocale>
#include <limits>
#include "string.h"
#include "data_variable.h"
#include <stdint.h>
#include "data_source.h"
#include "structures.h"


vector<rVariable *> rVariable::ListVar;


rVariable::rVariable(string name, TT_TYPE type, UINT flags, void *pointer, STRID unit, UDINT access)
{
	Name    = String_tolower(name);
	Type    = type;
	Flags   = flags;
	Pointer = pointer;
	Hash    = std::hash<std::string>{}(Name);
	Unit    = unit;
	Access  = access;
}


//-------------------------------------------------------------------------------------------------
// Конструктор удаляет все дерево переменных, включая дочерние и соседние узлы
rVariable::~rVariable()
{
	Name     = "";
	Type     = TYPE_UNDEF;
	Flags    = 0;
	Pointer  = 0;
	Access   = 0;
}



//
const rVariable *rVariable::Find(const string &name)
{
	string namelower = String_tolower(name);
	UDINT  hash      = std::hash<std::string>{}(namelower);

	for(UDINT ii = 0; ii < ListVar.size(); ++ii)
	{
		if(hash == ListVar[ii]->Hash)
		{
			if(ListVar[ii]->Name == namelower)
			{
				return ListVar[ii];
			}
		}
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
// Сортируем список по hash, для более быстрого поиска в будущем
void rVariable::Sort()
{
	std::sort(ListVar.begin(), ListVar.end(), [](const rVariable *a, const rVariable *b){ return a->Hash < b->Hash;});
}


//
void rVariable::DeleteVariables()
{
	for(UDINT ii = 0; ii < ListVar.size(); ++ii)
	{
		delete ListVar[ii];
	}
	ListVar.clear();
}


UDINT rVariable::SaveToCSV(const string &path)
{
	FILE *file = fopen((path + ".variable.csv").c_str(), "wt");

	if(!file) return 1;

	fprintf(file, ":alias;type;flags;access;alias hash;\n");

	for(UDINT ii = 0; ii < ListVar.size(); ++ii)
	{
		const rVariable *var = ListVar[ii];

		if(var->Flags & VARF___H_) continue;

		fprintf(file, "%s;%s;%#06x;%08X;%#010x;\n", var->Name.c_str(), NAME_TYPE[var->Type].c_str(), var->Flags, var->Access, var->Hash);
	}

	fclose(file);

	return 0;
}


