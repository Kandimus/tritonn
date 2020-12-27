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

#include "data_source.h"
#include "data_module.h"
#include <list>
#include "bits_array.h"
#include "compared_values.h"
#include "data_link.h"


// Внутренние флаги
const UDINT COUNTER_LE_CODE_FAULT = 0x00000001;

const UDINT COUNTER_SA_FAULT = 0x00010000;
const UDINT COUNTER_SA_IMP   = 0x00020000;


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rCounter : public rSource, private rDataModule
{
public:
	enum Setup : UINT
	{
		NONE    = 0x0000,
		OFF     = 0x0001,
		AVERAGE = 0x0002,
	};

	const UDINT AVERAGE_MAX = 5;

	rCounter();
	virtual ~rCounter();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "fi"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs

	//Outputs
	rLink      m_freq;
	rLink      m_period;
	rLink      m_impulse;

	// Variable
	UDINT      m_count;                   // Текущее значение счетчика с модуля
	rCmpUINT   m_setup;                   // Настройка сигнала
	UINT       m_status;                  // Нах это???????

protected:
	static rBitsArray m_flagsSetup;
	bool  m_isInit = false;
	UDINT m_countPrev = 0;
	UDINT m_tickPrev  = 0;

	std::list<LREAL> m_averageFreq;
};



