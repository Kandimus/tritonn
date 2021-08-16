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
#include "tickcount.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIODIChannel : public rIOBaseChannel
{
friend class rModuleDI16;
friend class rModuleDI8DO8;
friend class rModuleCRM;

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
		FILTER   = 0x0002,     // Защита от дребезга канала
		INVERSED = 0x0004,
	};

public:
	rIODIChannel(USINT index, const std::string& comment = "");
	virtual ~rIODIChannel() = default;

	USINT getValue()  const { return m_value; }
	UINT  getFilter() const { return m_filter; }
	void  setFilter();

	bool isOff()      const { return m_setup & Setup::OFF; }
	bool isFilter()   const { return m_setup & Setup::FILTER; }
	bool isInversed() const { return m_setup & Setup::INVERSED; }

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate) override;
	virtual UDINT processing() override;
	virtual UDINT simulate() override;
	virtual rBitsArray& getFlagsSetup() override { return m_flagsSetup; }

public:
	static rBitsArray m_flagsSimType;

protected:
	USINT m_phValue  = 0;             // значение с модуля
	UDINT m_filter   = 0;             // Задержка дребезга (мсек)

	UINT  m_setup    = 0;             // Настройка канала
	USINT m_value    = 0;             // Текущее значение
	USINT m_oldValue = 0;

	rTickCount m_timer;

	UINT  m_simValue = 0;
	UDINT m_simBlink = 1000;
	UDINT m_simTimer = 0;

	static rBitsArray m_flagsSetup;
};

