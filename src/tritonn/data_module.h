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
	const USINT FAULT = 0xFF;

	rDataModule();
	rDataModule(bool nochannel);
	virtual ~rDataModule() = default;

	UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);

public:
	bool        isSetModule() const;
	USINT       getModule()   const { return m_module; }
	USINT       getChannel()  const { return m_channel; }
	std::string getAlias() const;
	
protected:
	USINT m_module  = FAULT;
	USINT m_channel = FAULT;

private:
	bool  m_nochannel = false;
};



