/*
 *
 * data/ao.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "scale.h"
#include "../data_source.h"
#include "../data_module.h"
#include "../compared_values.h"
#include "../data_link.h"
#include "bits_array.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rAO : public rSource, private rDataModule
{
public:
	enum Setup : UINT
	{
		OFF          = 0x0001,     // Cигнал выключен из обработки
	};

	enum class Mode : UINT
	{
		PHIS     = 0,          // Симуляции нет, используется физический сигнал
		KEYPAD   = 1,          // Ручное переключение на симуляцию
	};

	rAO(const rStation* owner = nullptr);
	virtual ~rAO() = default;
	
// rSource
public:
	virtual const char* getRTTI() const override { return "ao"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;

	virtual std::string getModuleAlias()   const override { return rDataModule::getAlias();   }
	virtual USINT       getModuleNumber()  const override { return rDataModule::getModule();  }
	virtual USINT       getChannelNumber() const override { return rDataModule::getChannel(); }
protected:
	virtual UDINT       initLimitEvent(rLink& link) override;

public:
	// Inputs, Inoutputs
	rLink       m_present;

	// Outputs

	// Внутренние переменные
	UINT        m_phValue;
	rScale      m_scale;
	Mode        m_mode;
	rCmpUINT    m_setup;

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;
};



