//=================================================================================================
//===
//=== do_channel.h
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
#include "basechannel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIODOChannel : public rIOBaseChannel
{
public:
	enum Setup : UINT
	{
		UNDEF    = 0x0000,     // Статуст не определен
		OFF      = 0x0001,     // Канал выключен
		INVERTED = 0x0002,
	};

public:
	rIODOChannel() {}
	virtual ~rIODOChannel() {}

public:
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate();

public:
	UINT    m_setup        = 0;             // Настройка канала
	UINT    m_value        = 0;             // Текущий код ацп
	USINT   m_actionRedLED = 0;             // Управление касным диодом
	USINT   m_state        = 0;             // Статус канала
	USINT   m_stateRedLED  = 0;             // Статус красного диода
};

