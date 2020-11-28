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
	virtual ~rDataModule();

	bool  isSetModule() const;
	UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	
public:
	USINT m_module;
	USINT m_channel;
};



