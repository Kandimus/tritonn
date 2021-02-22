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
#include "variable_class.h"


using std::vector;

class  rThreadClass;
class  rError;
class  rVariableList;


//-------------------------------------------------------------------------------------------------
//
class rInterface : public rVariableClass
{
public:
	rInterface(pthread_mutex_t& mutex);
	virtual ~rInterface();

public:
	virtual UDINT       loadFromXML(tinyxml2::XMLElement* xml_root, rError& err);
	virtual std::string saveKernel(const std::string& objname, const std::string& comment);
	virtual UDINT       generateVars(rVariableClass* parent) = 0; // Генерация собственных переменных
	virtual UDINT       checkVars(rError& err) = 0; // Проверка переменных, поиск переменных, сгенерированных другими интерфейсами
	virtual UDINT       startServer() = 0;
	virtual rThreadClass* GetThreadClass() = 0;

public:
	std::string m_alias;
};



