﻿/*
 *
 * data/ai.h
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "bits_array.h"
#include "../data_source.h"
#include "../data_module.h"
#include "../compared_values.h"
#include "../data_link.h"
#include "scale.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rAI : public rSource, private rDataModule
{
public:
	// Статусы аналогового сигнала
	enum class Status : UINT
	{
		UNDEF  = 0x0000,     // Статуст не определен
		OFF    = 0x0001,     // Канал выключен
		NORMAL = 0x0010,     // Показания канала в норме
		MIN    = 0x0020,     // Значение ниже инженерного минимума
		MAX    = 0x0040,     // Значение выше инженерного максимума
		FAULT  = 0x0100,     // Ошибка. Выход из строя канала или модуля
	};

	// Настройка аналоговых сигналов
	enum Setup : UINT
	{
		OFF          = 0x0001,     // Cигнал выключен из обработки
		ERR_KEYPAD   = 0x0002,     // Разрешение при обрыве переводить сигнал в KEYPAD
		ERR_LASTGOOD = 0x0004,     // Разрешение при обрыве переводить сигнал в LASTGOOD
		NOICE        = 0x2000,     // Подавление шума около 4 и 20мА
	};

	// Режимы аналоговых сигналов
	enum class Mode : UINT
	{
		PHIS              = 0,          // Симуляции нет, используется физический сигнал
		MKEYPAD           = 1,          // Ручное переключение на симуляцию
		LASTGOOD          = 2,          // Используется последнее хорошее значение
		AKEYPAD           = 4,          // Автоматическое переключение на симуляцию
	};

	rAI(const rStation* owner = nullptr);
	virtual ~rAI() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "ai"; }

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
	UDINT setFault();

public:
	// Inputs, Inoutputs

	// Outputs
	rLink       m_phValue;               // Текущее физическое значение
	rLink       m_present;               // Результирующие значение
	rLink       m_current;               // Значение тока/напряжения, пересчитанное из кода АЦП

	// Внутренние переменные
	rCmpLREAL   m_keypad;                // Значение ручного ввода
	rScale      m_scale;                 // Инженерные пределы токового сигала
	Mode        m_mode;                  // Режим работы
	rCmpUINT    m_setup;                 // Настройка сигнала
	Status      m_status;                //
	UDINT       Security;                //
	LREAL       m_lastGood;              // Последнее "хорошее" значение

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsStatus;
};


