//=================================================================================================
//===
//=== interface.h
//===
//=== Copyright (c) 2020-2021 by RangeSoft.
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

#include "def.h"
#include "../variable_class.h"

class rThreadClass;
class rError;
class rGeneratorMD;

namespace tinyxml2
{
	class XMLElement;
}


//-------------------------------------------------------------------------------------------------
//
class rInterface : public rVariableClass
{
public:
	rInterface(pthread_mutex_t& mutex);
	virtual ~rInterface() = default;

public:
	virtual const char*   getRTTI() = 0;
	virtual UDINT         loadFromXML(tinyxml2::XMLElement* xml_root, rError& err);
	virtual UDINT         generateMarkDown(rGeneratorMD& md) = 0;
	virtual std::string   getMarkDown();
	virtual std::string   getAdditionalXml() const { return ""; }
	virtual UDINT         generateVars(rVariableClass* parent) = 0; // Генерация собственных переменных
	virtual UDINT         checkVars(rError& err) = 0; // Проверка переменных, поиск переменных, сгенерированных другими интерфейсами
	virtual UDINT         startServer() = 0;
	virtual rThreadClass* getThreadClass() = 0;

public:
	std::string m_alias;
};



