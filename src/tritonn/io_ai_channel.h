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
#include "io_basechannel.h"

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
		None = 0,
		Const,
		Linear,
		Sinus,
		Random,
	};

	enum Setup : UINT
	{
		UNDEF   = 0x0000,     // Статуст не определен
		OFF     = 0x0001,     // Канал выключен
		NOICE   = 0x0010,     //
		AVERAGE = 0x0020,     //
	};

	const UDINT MAX_AVERAGE = 3;

public:
	rIOAIChannel() {}
	virtual ~rIOAIChannel() {}

	UINT getMinValue() const;
	UINT getMaxValue() const;
	UINT getRange() const;

public:
	virtual UDINT generateVars(const std::string& name, rVariableList& list);
	virtual UDINT processing();
	virtual UDINT simulate();

public:
	UINT    m_setup        = 0;             // Настройка канала
	UINT    m_ADC          = 0;             // Текущий код ацп
	REAL    m_current      = 0;             // Текущий ток
	Type    m_type         = Type::mA_4_20; //
	USINT   m_actionRedLED = 0;             // Управление касным диодом
	USINT   m_state        = 0;             // Статус канала
	USINT   m_stateRedLED  = 0;             // Статус красного диода

	UINT    m_simMax       = 65535;
	UINT    m_simMin       = 0;
	UINT    m_simValue     = 0;
	INT     m_simSpeed     = 1;

private:
	USINT   m_hardState    = 0;             // Статус канала с модуля

	std::list<UINT> m_average;
};

