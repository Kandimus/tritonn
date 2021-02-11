﻿//=================================================================================================
//===
//=== prove.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс поверки
//===
//=================================================================================================

#pragma once

#include "../data_source.h"
#include "../data_module.h"
#include "bits_array.h"
#include "../compared_values.h"
#include "../data_link.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rProve : public rSource, private rDataModule
{
public:
	enum class Command : UINT
	{
		NONE = 0,
		START,
		STOP,
		ABORT,
		RESET,
	};

	enum Setup : UINT
	{
		NONE          = 0x0000,
		VALVE_4WAY    = 0x0001,
		STABILIZATION = 0x0002,
		NOVALVE       = 0x0004,
	};

	enum class State : USINT
	{
		IDLE = 0,
		START,
		NOFLOW,
		STABILIZATION,
		STABERROR,
		VALVETOUP,
		WAITTOUP,
	};

	rProve(const rStation* owner = nullptr);
	virtual ~rProve() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "prove"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const std::string &objname, const std::string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

private:
	void onIdle();
	void onStart();

public:
	// Inputs
	rLink m_temp;
	rLink m_pres;
	rLink m_dens;
	rLink m_open;
	rLink m_close;
	rLink m_opened;
	rLink m_closed;

	// Inoutputs

	// Outputs

	// Внутренние переменные
	Command  m_command = Command::NONE;
	rCmpUINT m_setup   = static_cast<UINT>(Setup::NONE);
	LREAL    m_inTemp  = 0;
	LREAL    m_inPres  = 0;
	LREAL    m_inDens  = 0;
	LREAL    m_curFreq = 0.0;
	UINT     m_curDetectors = 0;
	UDINT    m_timerStab;

private:
	State m_state = State::IDLE;

	static rBitsArray m_flagsSetup;
};



