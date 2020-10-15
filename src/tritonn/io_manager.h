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
#include "thread_class.h"

class rDataConfig;
class rIOBaseChannel;

//-------------------------------------------------------------------------------------------------
//
class rIOManager : public rThreadClass
{
	SINGLETON(rIOManager)

public:
		USINT setSimulateMode(USINT issim) { USINT result = m_isSimulate.Get(); m_isSimulate.Set(issim); return result; }

// Методы
public:
	UDINT LoadFromXML(tinyxml2::XMLElement* element, rDataConfig &cfg);
	UDINT GenerateVars(vector<rVariable* > &list);
	UDINT SaveKernel(FILE* file);

	rIOBaseChannel* getChannel(USINT module, USINT channel);

protected:
	virtual rThreadStatus Proccesing();

private:
	rSafityValue<USINT> m_isSimulate;
	std::vector<rIOBaseModule* > m_modules; //
};



