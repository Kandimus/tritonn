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
		mA_4_24,         //
		V_m10_10,		  //
		V_0_10,          //
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
		UNDEF  = 0x0000,     // Статуст не определен
		OFF    = 0x0001,     // Канал выключен
		NOICE  = 0x0010,     //
		SMOOTH = 0x0020,     //
	};

	rIOAIChannel() {};
	virtual ~rIOAIChannel() {}

	UINT    m_setup        = 0;
	UINT    m_ADC          = 0;
	REAL    m_current      = 0;
	Type    m_type         = Type::mA_0_20;
	USINT   m_actionRedLED = 0;
	USINT   m_state        = 0;
	USINT   m_stateRedLED  = 0;

	UINT    m_simMax       = 65535;
	UINT    m_simMin       = 0;
	UINT    m_simValue     = 0;
	INT     m_simSpeed     = 1;

public:
	virtual UDINT simulate();
};

