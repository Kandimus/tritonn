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
#include "bits_array.h"
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
		PULSE    = 0x0002,
		INVERSED = 0x0004,
	};

public:
	rIODOChannel(USINT index);
	virtual ~rIODOChannel() = default;

	USINT getValue() { return m_value; }

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate();

public:
	UINT  m_setup        = 0;             // Настройка канала
	UINT  m_value        = 0;             // Текущий код ацп
	USINT m_actionRedLED = 0;             // Управление касным диодом
	USINT m_state        = 0;             // Статус канала
	USINT m_stateRedLED  = 0;             // Статус красного диода

	UDINT m_pulse        = 1000;

private:
	USINT m_hardValue    = 0;
	USINT m_hardState    = 0;
	USINT m_oldValue     = 0;
	UDINT m_pulseTimer   = 0;

	static rBitsArray m_flagsSetup;
};

