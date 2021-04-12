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
#include "bits_array.h"
#include "basechannel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIODIChannel : public rIOBaseChannel
{
public:
	enum SimType
	{
		NONE = 0,
		CONST,
		PULSE,
		RANDOM,
	};

	enum Setup : UINT
	{
		UNDEF    = 0x0000,     // Статуст не определен
		OFF      = 0x0001,     // Канал выключен
		BOUNCE   = 0x0002,     // Защита от дребезга канала
		INVERSED = 0x0004,
	};

public:
	rIODIChannel(USINT index, const std::string& comment = "");
	virtual ~rIODIChannel() = default;

	USINT getValue() const { return m_value; }

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate);
	virtual UDINT processing();
	virtual UDINT simulate();
	virtual rBitsArray& getFlagsSetup() { return m_flagsSetup; }

public:
	UINT  m_setup        = 0;             // Настройка канала
	USINT m_value        = 0;             // Текущий код ацп
	USINT m_state        = 0;             // Статус канала
	UDINT m_bounce       = 0;             // Задержка дребезга (мсек)
//	USINT m_actionRedLED = 0;             // Управление касным диодом
//	USINT m_stateRedLED  = 0;             // Статус красного диода

	UINT  m_simValue    = 0;
	UDINT m_simBlink    = 1000;
	UDINT m_simTimer    = 0;

	static rBitsArray m_flagsSimType;

private:
	USINT m_hardState   = 0;             // Статус канала с модуля
	USINT m_hardValue   = 0;

	UDINT m_bounceTimer = 0;
	USINT m_oldValue    = 0;

	static rBitsArray m_flagsSetup;
};

