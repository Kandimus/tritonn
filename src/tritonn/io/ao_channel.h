/*
 *
 * ao_channel.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include <list>
#include "bits_array.h"
#include "basechannel.h"

class rModuleAO4;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rIOAOChannel : public rIOBaseChannel
{
friend rModuleAO4;
public:

	enum class Mode : UINT
	{
		ACTIVE = 0,
		PASSIVE,
	};

	enum Setup : UINT
	{
		UNDEF   = 0x0000,     // Статуст не определен
		OFF     = 0x0001,     // Канал выключен
	};

	enum class Regime : UINT
	{
		REDUCED_DAC = 0,
		TRUE_UA,
	};

public:
	rIOAOChannel(USINT index, const std::string& comment = "");
	virtual ~rIOAOChannel() = default;

public:
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err) override;
	virtual UDINT generateVars(const std::string& name, rVariableList& list, bool issimulate) override;
	virtual UDINT processing() override;
	virtual UDINT simulate() override;
	virtual rBitsArray& getFlagsSetup() override { return m_flagsSetup; }

protected:
	UINT getMinValue() const;
	UINT getMaxValue() const;
	UINT getRange() const;

public:
	UINT m_setup = 0;             // Настройка канала

	// hardware
	Mode   m_mode   = Mode::ACTIVE;
	UINT   m_ADC    = 0;             // Текущий код ацп
	Regime m_regime = Regime::REDUCED_DAC;

private:
	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsMode;
};

