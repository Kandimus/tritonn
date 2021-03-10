//=================================================================================================
//===
//=== data_do.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс дискретного выходного сигнала (DO)
//===
//=================================================================================================

#pragma once

#include "data_source.h"
#include "data_module.h"
#include "bits_array.h"
#include "compared_values.h"
#include "data_link.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rDO : public rSource, private rDataModule
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
		SUCCESS_ON  = 0x0002,
		WARNING_ON  = 0x0004,
		ALARM_ON    = 0x0008,
		SUCCESS_OFF = 0x0010,
		WARNING_OFF = 0x0020,
		ALARM_OFF   = 0x0040,
	};

	// Режимы
	enum class Mode : UINT
	{
		PHIS   = 0,          // Симуляции нет, используется физический сигнал
		KEYPAD = 1,          // Ручное переключение на симуляцию
	};

	rDO(const rStation* owner = nullptr);
	virtual ~rDO() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const override { return "do"; }

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
	// Inputs, Inoutputs
	rLink    m_present;               // Результирующие значение

	// Output

	// Внутренние переменные
	USINT    m_physical;              // Текущее физическое значение
	Mode     m_mode;                  // Режим работы
	rCmpUINT m_setup;                 // Настройка сигнала
	Status   m_status;                //
//	UDINT       Security;                //

private:
	LREAL    m_oldvalue;

	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsStatus;
};



