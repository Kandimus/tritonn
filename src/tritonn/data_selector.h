//=================================================================================================
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
	rSelector(const rStation* owner = nullptr);
	virtual ~rSelector() = default;

	void generateIO();
	
	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const override { return (m_setup.Value & SELECTOR_SETUP_MULTI) ? "mselector" : "selector"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement *element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT initLimitEvent(rLink &link);
	
public:
	rLink    ValueIn  [MAX_SELECTOR_INPUT][MAX_SELECTOR_GROUP]; // Массив входных значений
	rLink    FaultIn  [MAX_SELECTOR_INPUT][MAX_SELECTOR_GROUP]; // Массив ошибок по каждому входу
	rLink    ValueOut [MAX_SELECTOR_GROUP];                     // Выходное значение

	LREAL    Keypad   [MAX_SELECTOR_GROUP];                     // Значение подстановки
	STRID    KpUnit   [MAX_SELECTOR_GROUP];                     //
	string   NameInput[MAX_SELECTOR_GROUP];                     // Имена входов
	rCmpUINT m_setup;                                             // Настройка
	rCmpUINT Mode;
	rCmpINT  Select;                                            // Переменная для управлением выбора
	UINT     CountInputs;                                       // Количество используемых входных значений, не больше чем MAX_SELECTOR_DATA
	UINT     CountGroups;

private:
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsMode;
};




