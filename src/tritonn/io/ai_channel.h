//=================================================================================================
//===
//=== io_ai_channel.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс канала аналового входного модуля AI (CAN)
//===
//=================================================================================================

#pragma once

#include <list>
#include "bits_array.h"
#include "basechannel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIOAIChannel : public rIOBaseChannel
{
public:
	enum class Type : USINT
	{
		mA_0_20 = 0,     //
		mA_4_20,         //
		V_m10_10,		  //
		V_0_10,          //
	};

	enum class Scale_mA_0_20 : UDINT
	{
		Min = 0,
		Max = 54612,
	};

	enum class Scale_mA_4_20 : UDINT
	{
		Min = 10922,
		Max = 54612,
	};

	enum class Scale_V_m10_10 : UDINT
	{
		Min = 2979,
		Max = 62556,
	};

	enum class Scale_V_0_10 : UDINT
	{
		Min = 32767,
		Max = 62556,
	};

	enum SimType
	{
		NONE = 0,
		CONST,
		LINEAR,
		SINUS,
		RANDOM,
	};

	enum Setup : UINT
	{
		UNDEF   = 0x0000,     // Статуст не определен
		OFF     = 0x0001,     // Канал выключен
//		NOICE   = 0x0010,     //
		AVERAGE = 0x0020,     //
	};

	const UDINT MAX_AVERAGE = 3;

public:
	rIOAIChannel(USINT index, const std::string& comment = "");
	virtual ~rIOAIChannel() = default;

	UINT getMinValue() const;
	UINT getMaxValue() const;
	UINT getRange() const;
	REAL getCurrent() const { return m_current; }

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate();
	virtual rBitsArray& getFlagsSetup() { return m_flagsSetup; }

public:
	UINT    m_setup        = 0;             // Настройка канала
	USINT   m_state        = 0;             // Статус канала (0 - норма)

	// hardware
	Type    m_type         = Type::mA_4_20; //
	UINT    m_ADC          = 0;             // Текущий код ацп
	REAL    m_current      = 0;             // Текущие амперы/вольты
	USINT   m_hardState    = 0;             // Статус канала с модуля
	USINT   m_stateRedLED  = 0;             // Статус красного диода
	USINT   m_actionRedLED = 0;             // Управление касным диодом

	// simulate
	UINT    m_simMax       = 65535;
	UINT    m_simMin       = 0;
	UINT    m_simValue     = 0;
	INT     m_simSpeed     = 1;

	static rBitsArray m_flagsSimType;

private:

	std::list<UINT> m_average;

	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsType;
};

