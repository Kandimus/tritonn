//=================================================================================================
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
#include "safity.h"
#include "tinyxml2.h"
#include "data_variable.h"
#include "thread_class.h"
#include "io_basemodule.h"

class rDataConfig;
class rIOBaseChannel;

//-------------------------------------------------------------------------------------------------
//
class rIOManager : public rThreadClass
{
	SINGLETON(rIOManager)

public:

// Методы
public:
	UDINT LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg);
	UDINT GenerateVars(vector<rVariable* > &list);
	UDINT SaveKernel(FILE* file);

	rIOBaseChannel* getChannel(USINT module, USINT channel);

protected:
	virtual rThreadStatus Proccesing();

private:
	std::vector<rIOBaseModule* > m_modules; //
};



