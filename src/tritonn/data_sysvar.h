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

#include <vector>
#include "def.h"
#include "structures.h"

using std::vector;

class rVariable;

// Системные переменные
class rSystemVariable
{
public:
	rSystemVariable() {}
	~rSystemVariable() {}

	UDINT SaveKernel(FILE *file);
	UDINT InitVariables(vector<rVariable *> &list);

public:
	rVersion    Ver;
	rState      State;
	Time64_T    UnixTime;
	STM         DateTime;
	STM         SetDateTime;
	USINT       SetDateTimeAccept;
	rMaxCount   Max;
	rConfigInfo ConfigInfo;
	char        Lang[MAX_LANG_SIZE];

	//TODO SetTime
	//TODO IP
};





