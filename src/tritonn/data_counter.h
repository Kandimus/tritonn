//=================================================================================================
//===
//=== data_counter.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс частотного входного сигнала (FI)
//===
//=================================================================================================

#pragma once

//#include "io.h"
#include "data_source.h"
#include "compared_values.h"
#include "tickcount.h"


// Внутренние флаги
const UDINT COUNTER_LE_CODE_FAULT = 0x00000001;

const UDINT COUNTER_SA_FAULT = 0x00010000;
const UDINT COUNTER_SA_IMP   = 0x00020000;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rCounter : public rSource
{
public:
	rCounter();
	virtual ~rCounter();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "fi"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT GenerateVars(vector<rVariable *> &list);
	virtual UDINT SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs

	//Outputs
	rLink      Freq;
	rLink      Period;
	rLink      Impulse;

	// Variable
	//rAI_io Channel;
	UDINT      SetCount;
	LREAL      CountTail;
	UINT       Count;                   // Текущее значение счетчика с модуля
	UINT       ChFault;                 // Ошибка канала или модуля
	UDINT      LastCount;               // Прошлое значение счетчика с модуля. Если LastCount == 0xFFFFFFFF, то это означает что нужно пропустить цикл

	rCmpUINT   Setup;                   // Настройка сигнала
	rTickCount Tick;                    // Прошлый системный Tick (мсек)
	LREAL      Spline[MAX_FI_SPLINE];   // Массив последних 4 "хороших" частот, для сглаживания
	UINT       Status;                  // Нах это???????

protected:

};



