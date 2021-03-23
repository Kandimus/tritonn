//=================================================================================================
//===
//=== data_sysvar.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для системных переменных rDataManager
//===
//=================================================================================================

#pragma once

#include "def.h"
#include "structures.h"
#include <map>

class rVariableList;
class rGeneratorMD;

// Системные переменные
class rSystemVariable
{
public:
	rSystemVariable() = default;
	~rSystemVariable() = default;

	void  generateMarkDown(rGeneratorMD& md);
	UDINT initVariables(rVariableList& list);

public:
	rVersion      m_version;
	rMetrologyVer m_metrologyVer;
	rState        m_state;
	Time64_T      UnixTime;
	STM           DateTime;
	STM           SetDateTime;
	USINT         SetDateTimeAccept;
	rConfigInfo   ConfigInfo;
	char          Lang[MAX_LANG_SIZE];

	std::map<std::string, UDINT> m_max;

	//TODO IP
};





