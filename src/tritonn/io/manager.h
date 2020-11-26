﻿//=================================================================================================
//===
//=== io_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить для получения данных с модулей ввода-вывода
//===
//=================================================================================================

#pragma once

#include <vector>
#include <memory>
#include "safity.h"
#include "tinyxml2.h"
#include "../variable_class.h"
#include "thread_class.h"
#include "basemodule.h"

class rDataConfig;
class rIOBaseChannel;

//-------------------------------------------------------------------------------------------------
//
class rIOManager : public rThreadClass, public rVariableClass
{
	SINGLETON(rIOManager)

public:

// Методы
public:
	UDINT LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg);
	UDINT generateVars(rVariableClass* parent);
	std::string saveKernel();

	std::unique_ptr<rIOBaseChannel> getChannel(USINT module, USINT channel);

protected:
	virtual rThreadStatus Proccesing();

private:
	std::vector<rIOBaseModule*> m_modules; //
	USINT m_moduleCount;
};


