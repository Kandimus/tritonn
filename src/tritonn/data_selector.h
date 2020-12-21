﻿//=================================================================================================
//===
//=== data_selector.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== 
//===
//=================================================================================================

#pragma once

#include "data_link.h"
#include "bits_array.h"
#include "compared_values.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//

class rSelector : public rSource
{
public:
	rSelector();
	virtual ~rSelector();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return (Setup.Value & SELECTOR_SETUP_MULTI) ? "mselector" : "selector"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);
	
public:
	rLink    ValueIn  [MAX_SELECTOR_INPUT][MAX_SELECTOR_GROUP]; // Массив входных значений
	rLink    FaultIn  [MAX_SELECTOR_INPUT][MAX_SELECTOR_GROUP]; // Массив ошибок по каждому входу
	rLink    ValueOut [MAX_SELECTOR_GROUP];                     // Выходное значение

	LREAL    Keypad   [MAX_SELECTOR_GROUP];                     // Значение подстановки
	STRID    KpUnit   [MAX_SELECTOR_GROUP];                     //
	string   NameInput[MAX_SELECTOR_GROUP];                     // Имена входов
	rCmpUINT Setup;                                             // Настройка
	rCmpUINT Mode;
	rCmpINT  Select;                                            // Переменная для управлением выбора
	UINT     CountInputs;                                       // Количество используемых входных значений, не больше чем MAX_SELECTOR_DATA
	UINT     CountGroups;

public:
	void GenerateIO();

private:
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsMode;
};




