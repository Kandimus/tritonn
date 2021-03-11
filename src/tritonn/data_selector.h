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
	enum Setup : UINT
	{
		OFF        = 0x0001,      // Селектор отключен
		NOEVENT    = 0x0004,      // Не выдавать сообщений
		MULTI      = 0x8000,      // Флаг мультиселектора (внутренний), не указывать в документации
	};

	enum Mode : UINT
	{
		NOCHANGE   = 1,           // Не переходить в случае аварии на другое входное значение
		TOERROR    = 2,           // В случае аварии переходить на ручной ввод (keypad)
		CHANGEPREV = 3,           // В случае аварии переходить на предыдущее входное значение
		CHANGENEXT = 4,           // В случае аварии переходить на следующее входное значение
	};

	rSelector(const rStation* owner = nullptr);
	virtual ~rSelector() = default;

	void generateIO();
	
	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const override { return (m_setup.Value & Setup::MULTI) ? "mselector" : "selector"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement *element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT initLimitEvent(rLink &link) override;
	
public:
	rLink    ValueIn  [MAX_SELECTOR_INPUT][MAX_SELECTOR_GROUP]; // Массив входных значений
	rLink    FaultIn  [MAX_SELECTOR_INPUT][MAX_SELECTOR_GROUP]; // Массив ошибок по каждому входу
	rLink    ValueOut [MAX_SELECTOR_GROUP];                     // Выходное значение

	LREAL    Keypad   [MAX_SELECTOR_GROUP];                     // Значение подстановки
	STRID    KpUnit   [MAX_SELECTOR_GROUP];                     //
	string   NameInput[MAX_SELECTOR_GROUP];                     // Имена входов
	rCmpUINT m_setup;                                           // Настройка
	rCmpUINT m_mode;
	rCmpINT  m_select;                                          // Переменная для управлением выбора
	UINT     CountInputs;                                       // Количество используемых входных значений, не больше чем MAX_SELECTOR_DATA
	UINT     CountGroups;

private:
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsMode;
};




