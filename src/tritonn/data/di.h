/*
 *
 * data/do.h
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "../data_module.h"
#include "../compared_values.h"
#include "link.h"
#include "bits_array.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rDI : public rSource, private rDataModule
{
public:
	// Статус
	enum class Status : UINT
	{
		UNDEF  = 0x0000,     // Статуст не определен
		OFF    = 0x0001,     // Канал выключен
		NORMAL = 0x0002,     // Показания канала в норме
		FAULT  = 0x0004,     // Ошибка. Выход из строя канала или модуля
	};

	// Настройка
	enum Setup
	{
		OFF         = 0x0001,     // Cигнал выключен из обработки
		ERR_KEYPAD  = 0x0002,     // Разрешение при обрыве переводить сигнал в KEYPAD
		SUCCESS_ON  = 0x0004,
		SUCCESS_OFF = 0x0008,
		WARNING_ON  = 0x0010,
		WARNING_OFF = 0x0020,
		ALARM_ON    = 0x0040,
		ALARM_OFF   = 0x0080,
	};

	// Режимы
	enum class Mode : UINT
	{
		PHIS   = 0,          // Симуляции нет, используется физический сигнал
		KEYPAD = 1,          // Ручное переключение на симуляцию
	};

	rDI(const rStation* owner = nullptr);
	virtual ~rDI() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "di"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;

	virtual std::string getModuleAlias()   const override { return rDataModule::getAlias();   }
	virtual USINT       getModuleNumber()  const override { return rDataModule::getModule();  }
	virtual USINT       getChannelNumber() const override { return rDataModule::getChannel(); }
protected:
	virtual UDINT       initLimitEvent(rLink &link) override;

public:
	UDINT setFault();

public:
	// Inputs, Inoutputs

	// Outputs
	rLink       m_physical;              // Текущее физическое значение
	rLink       m_present;               // Результирующие значение

	// Внутренние переменные
	rCmpUSINT   m_keypadValue;           // Значение ручного ввода
	Mode        m_mode;                  // Режим работы
	rCmpUINT    m_setup;                 // Настройка сигнала
	Status      m_status;                //
//	UDINT       Security;                //

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsStatus;
};



