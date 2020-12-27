//=================================================================================================
//===
//=== fi_channel.h
//===
//=== Copyright (c) 2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс частотного канала
//===
//=================================================================================================

#pragma once

#include <list>
#include "basechannel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIOFIChannel : public rIOBaseChannel
{
public:
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
		UNDEF    = 0x0000,     // Статуст не определен
		OFF      = 0x0001,     // Канал выключен
		AVERAGE  = 0x0002,
	};

public:
	rIOFIChannel();
	virtual ~rIOFIChannel() {}

	UDINT getValue() const { return m_value; }

public:
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate();

public:
	UINT  m_setup        = 0;             // Настройка канала
	UDINT m_value        = 0;             // Текущий накопитель
	USINT m_state        = 0;             // Статус канала

	UINT m_simMax       = 10000;
	UINT m_simMin       = 0;
	UINT m_simValue     = 0;              // Значение в Герцах
	INT  m_simSpeed     = 0;              //

private:
	USINT m_hardState   = 0;             // Статус канала с модуля
	USINT m_simSinus    = 0;
	UDINT m_simTimer    = 0;
	UDINT m_simTimerRem = 0;
};

