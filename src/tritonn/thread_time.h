//=================================================================================================
//===
//=== total.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Нарастающие
//===
//=================================================================================================

#pragma once


#include "def.h"


struct rThreadTimeInfo
{
	rThreadTimeInfo() : Work(0), Idle(0) {}
	rThreadTimeInfo(UDINT work, UDINT idle) : Work(work), Idle(idle) {}
	rThreadTimeInfo(rThreadTimeInfo *tti) { if(nullptr != tti) { Work = tti->Work; Idle = tti->Idle;} }

	UDINT Work;
	UDINT Idle;
};


struct rThreadTimeAvr
{
	UDINT WorkMax;        // Абсолютное максимальное время выполнения (между rThreadClass::Processing и rThreadClass::EndProcessing)
	UDINT WorkMin;        // Абсолютное минимальное время выполнения
	UDINT WorkAverage;    // Абсолютно среднее время выполения
	UDINT WorkAvgMax;     // Среденее время, если текущее время выполнения попадает между WorkMax и WorkAverage
	UDINT IdleAverage;    // Среднее время простоя нити
	UDINT IdleMax;        // Абсолютное максимальное время простоя (между rThreadClass::EndProcessing и rThreadClass::Processing)
	UDINT IdleMin;        // Абсолютное минимальное время простоя
};

