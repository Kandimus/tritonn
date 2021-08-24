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
#include "tickcount.h"

class rIODOChannel : public rIOBaseChannel
{
friend class rModuleDO16;
friend class rModuleDI8DO8;

public:
	enum Setup : UINT
	{
		UNDEF    = 0x0000,     // Статуст не определен
		OFF      = 0x0001,     // Канал выключен
		PULSE    = 0x0002,
		INVERSED = 0x0004,
	};

public:
	rIODOChannel(USINT index, const std::string& comment = "");
	virtual ~rIODOChannel() = default;

	void setValue(bool value);

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate) override;
	virtual UDINT processing() override;
	virtual bool  simulate() override;
	virtual std::string getMarkDownFlags() const override;

protected:
	UINT  m_setup    = 0;             // Настройка канала
	UINT  m_value    = 0;             // Текущий код ацп
	UDINT m_pulse    = 1000;


	USINT m_phValue  = 0;
	USINT m_oldValue = 0;

	rTickCount m_timer;

	static rBitsArray m_flagsSetup;
};

