//=================================================================================================
//===
//=== di_channel.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс дискретного входного канала
//===
//=================================================================================================

#pragma once

#include <list>
#include "basechannel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIODIChannel : public rIOBaseChannel
{
public:
	enum SimType
	{
		None = 0,
		Const,
		Pulse,
		Random,
	};

	enum Setup : UINT
	{
		UNDEF    = 0x0000,     // Статуст не определен
		OFF      = 0x0001,     // Канал выключен
		BOUNCE   = 0x0002,     // Защита от дребезга канала
		INVERTED = 0x0004,
	};

public:
	rIODIChannel();
	virtual ~rIODIChannel() {}

public:
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate();

public:
	UINT  m_setup        = 0;             // Настройка канала
	USINT m_value        = 0;             // Текущий код ацп
	USINT m_state        = 0;             // Статус канала
	UDINT m_bounce       = 0;             // Задержка дребезга (мсек)
//	USINT m_actionRedLED = 0;             // Управление касным диодом
//	USINT m_stateRedLED  = 0;             // Статус красного диода

	USINT m_simValue    = 0;
	UDINT m_simBlink    = 1000;
	UDINT m_simTimer    = 0;

private:
	USINT m_hardState   = 0;             // Статус канала с модуля
	USINT m_hardValue   = 0;

	UDINT m_bounceTimer = 0;
	USINT m_oldValue    = 0;
};

