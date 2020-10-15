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

//#include "io.h"
#include "data_source.h"
#include "compared_values.h"
#include "data_link.h"



struct rAIScale
{
	rAIScale();
	
	rCmpLREAL Min;               // Значение инж. минимума
	rCmpLREAL Max;               //

	UINT      Code_4mA;          // Максимальный код АЦП
	UINT      Code_20mA;         // Минимальный код АЦП
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rAI : public rSource
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

	rAI();
	virtual ~rAI();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "ai"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT GenerateVars(vector<rVariable *> &list);
	virtual UDINT SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	UDINT SetFault();

public:
	// Inputs, Inoutputs

	// Outputs
	rLink       PhValue;                 // Текущее физическое значение
	rLink       Value;                   // Результирующие значение
	rLink       Current;                 // Значение тока/напряжения, пересчитанное из кода АЦП

	// Внутренние переменные
	//rAI_io Channel;
	UDINT       SetCode;                 // Для теста
	UINT        ChFault;                 //TODO Временное решение
	UINT        Code;                    //TODO Временное решение, пока небудет реализован класс rAI_io
	DINT        UsedCode;                // Используемый код АЦП для расчета значения
	rCmpLREAL   KeypadValue;             // Значение ручного ввода
//	rLimit      Limit;                   // Технологические пределы
	rAIScale    Scale;                   // Инженерные пределы токового сигала
	UINT        Mode;                    // Режим работы
	rCmpUINT    Setup;                   // Настройка сигнала
	rAI::Status m_status;                //
//	STRID       Unit;                    // Номер строки, которая будет использоваться в качесте единиц измерения
	UDINT       Security;                //
	UINT        Spline[MAX_AI_SPLINE];   // Массив последних 4 "хороших" кодов АЦП, для сглаживания
	LREAL       LastGood;                // Последнее "хорошее" значение
};



