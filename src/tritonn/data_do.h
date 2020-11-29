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
		OFF        = 0x0001,     // Cигнал выключен из обработки
	};

	// Режимы
	enum class Mode : UINT
	{
		PHIS   = 0,          // Симуляции нет, используется физический сигнал
		KEYPAD = 1,          // Ручное переключение на симуляцию
	};

	rDO();
	virtual ~rDO();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "di"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs, Inoutputs
	rLink       m_present;               // Результирующие значение

	// Output

	// Внутренние переменные
	USINT       m_physical;              // Текущее физическое значение
	Mode        m_mode;                  // Режим работы
	rCmpUINT    m_setup;                 // Настройка сигнала
	Status      m_status;                //
//	UDINT       Security;                //

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;
};



