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
#include "bits_array.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIOFIChannel : public rIOBaseChannel
{
public:
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
		UNDEF    = 0x0000,     // Статуст не определен
		OFF      = 0x0001,     // Канал выключен
		AVERAGE  = 0x0002,
	};

	const UDINT MAX_AVERAGE = 5;

public:
	rIOFIChannel(USINT index, const std::string& comment = "");
	virtual ~rIOFIChannel() = default;

	UDINT getValue() const { return m_value; }
	LREAL getFreq()  const { return m_freq;  }

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate) override;
	virtual UDINT processing() override;
	virtual UDINT simulate() override;
	virtual rBitsArray& getFlagsSetup() override { return m_flagsSetup; }

public:
	UINT  m_setup        = 0;             // Настройка канала
	UDINT m_value        = 0;             // Текущий накопитель
	LREAL m_freq         = 0.0;           // Частота
	USINT m_state        = 0;             // Статус канала

	UINT m_simMax       = 10000;
	UINT m_simMin       = 0;
	UINT m_simValue     = 0;              // Значение в Герцах
	INT  m_simSpeed     = 0;              //

	static rBitsArray m_flagsSimType;

private:
	USINT m_hardState   = 0;             // Статус канала с модуля
	UINT  m_simSinus    = 0;
	UDINT m_simTimer    = 0;
	UDINT m_simTimerRem = 0;
	LREAL m_simCountRem = 0.0;

	std::list<LREAL> m_average;

	static rBitsArray m_flagsSetup;
};

