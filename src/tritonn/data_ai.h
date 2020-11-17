//=================================================================================================
//===
//=== data_ai.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс аналового входног сигнала (AI)
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
	enum Setup
	{
		OFF          = 0x0001,     // Cигнал выключен из обработки
		ERR_KEYPAD   = 0x0002,     // Разрешение при обрыве переводить сигнал в KEYPAD
		ERR_LASTGOOD = 0x0004,     // Разрешение при обрыве переводить сигнал в LASTGOOD
		NOBUFFER     = 0x8000,     // Отключение буфферизации значений (сглаживание)
		VIRTUAL      = 0x4000,     // "Виртуальный" аналоговый сигнал. Без обработки кода АЦП. При установке SimValue события не проиходит
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

	struct rScale
	{
		rScale() : Min(0), Max(100), Code_4mA(10923), Code_20mA(54613) {}

		rCmpLREAL Min;               // Значение инж. минимума
		rCmpLREAL Max;               //

		UINT      Code_4mA;          // Максимальный код АЦП
		UINT      Code_20mA;         // Минимальный код АЦП
	};

	rAI();
	virtual ~rAI();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "ai"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	UDINT SetFault();

public:
	// Inputs, Inoutputs

	// Outputs
	rLink       PhValue;                 // Текущее физическое значение
	rLink       m_present;               // Результирующие значение
	rLink       Current;                 // Значение тока/напряжения, пересчитанное из кода АЦП

	// Внутренние переменные
	UINT        ChFault;                 //TODO Временное решение
	rCmpLREAL   KeypadValue;             // Значение ручного ввода
	rScale      m_scale;                 // Инженерные пределы токового сигала
	Mode        m_mode;                  // Режим работы
	rCmpUINT    m_setup;                 // Настройка сигнала
	Status      m_status;                //
	UDINT       Security;                //
	LREAL       m_lastGood;              // Последнее "хорошее" значение

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;

};



