//=================================================================================================
//===
//=== data_module.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Классы для описания модулей ввода-вывода
//===
//=================================================================================================

#pragma once

#include "def.h"

namespace tinyxml2
{
	class XMLElement;
};

class rError;


class rDataModule
{
public:
	rDataModule();
	rDataModule(bool nochannel);
	virtual ~rDataModule() = default;

	bool  isSetModule() const;
	UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	
public:
	USINT m_module  = 0xFF;
	USINT m_channel = 0xFF;

private:
	bool  m_nochannel = false;
};



