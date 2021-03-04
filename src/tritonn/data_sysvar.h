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

class rVariableList;


// Системные переменные
class rSystemVariable
{
public:
	rSystemVariable() = default;
	~rSystemVariable() = default;

	std::string saveKernel();
	UDINT       initVariables(rVariableList& list);

public:
	rVersion      m_version;
	rMetrologyVer m_metrologyVer;
	rState        m_state;
	Time64_T      UnixTime;
	STM           DateTime;
	STM           SetDateTime;
	USINT         SetDateTimeAccept;
	rMaxCount     Max;
	rConfigInfo   ConfigInfo;
	char          Lang[MAX_LANG_SIZE];

	//TODO SetTime
	//TODO IP
};





