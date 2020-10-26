//=================================================================================================
//===
//=== data_interface.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для всех интерфейсов
//===
//=================================================================================================

#pragma once

#include <vector>
#include "tinyxml2.h"
#include "def.h"


using std::vector;

class  rThreadClass;
class  rDataConfig;
class  rVariableList;


//-------------------------------------------------------------------------------------------------
//
class rInterface
{
public:
	rInterface();
	virtual ~rInterface();

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement *xml_root, rDataConfig &) = 0;
	virtual UDINT saveKernel(FILE *file, const string &objname, const string &comment);
	virtual UDINT generateVars(rVariableList& list) = 0; // Генерация собственных переменных
	virtual UDINT CheckVars(rDataConfig &cfg) = 0;             // Проверка переменных, поиск переменных, сгенерированных другими интерфейсами
	virtual UDINT StartServer() = 0;
	virtual rThreadClass *GetThreadClass() = 0;

public:
	std::string Alias;
};



