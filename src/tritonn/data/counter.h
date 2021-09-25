/*
 *
 * data/counter.h
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include <list>
#include "bits_array.h"
#include "link.h"
#include "../data_module.h"
#include "../compared_values.h"

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
	};

	const UDINT CALCULATE_TIMER = 1000;

	rCounter(const rStation* owner = nullptr);
	virtual ~rCounter() = default;

	
	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "counter"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;

	virtual std::string getModuleAlias()   const override { return rDataModule::getAlias();   }
	virtual USINT       getModuleNumber()  const override { return rDataModule::getModule();  }
	virtual USINT       getChannelNumber() const override { return rDataModule::getChannel(); }

protected:
	virtual UDINT       initLimitEvent(rLink& link) override;

private:
	LREAL getPeriod();
	void  clear();

public:
	// Inputs

	//Outputs
	rLink    m_freq;
	rLink    m_period;
	rLink    m_impulse;

	// Variable
	UDINT    m_count;                   // Текущее значение счетчика с модуля
	rCmpUINT m_setup;                   // Настройка сигнала
	UINT     m_status;                  // Нах это???????

protected:
	static rBitsArray m_flagsSetup;
	bool  m_isInit = false;
	UDINT m_countPrev = 0;
//	UDINT m_tickPrev  = 0;
	UDINT m_pullingCount = 0;
};

