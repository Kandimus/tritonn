//=================================================================================================
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
#include "tickcount.h"

class rAI;

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
		ERRORSTAB,
		VALVETOUP,
		WAITTOUP,
		VALVETODOWN,
		ERRORTOUP,
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
	void onNoFlow();
	void onStabilization();
	void onValveToUp();


	bool checkStab(const rAI& ai, LREAL val);

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
	rCmpUINT m_strIdx;
	LREAL    m_inTemp  = 0;
	LREAL    m_inPres  = 0;
	LREAL    m_inDens  = 0;
	LREAL    m_strTemp = 0;
	LREAL    m_strPres = 0;
	LREAL    m_strDens = 0;

	LREAL    m_maxStabTemp = 0.2;
	LREAL    m_maxStabPres = 0.01;
	LREAL    m_maxStabDens = 0.5;

	LREAL    m_curFreq = 0.0;
	UINT     m_curDetectors = 0;

	UDINT    m_tsStart = 1000;
	UDINT    m_tsStab  = 20000;
	UDINT    m_tsD1    = 20000;
	UDINT    m_tsD2    = 20000;
	UDINT    m_tsV     = 20000;

private:
	State m_state = State::IDLE;

	LREAL m_stabDens = 0;
	LREAL m_stabPres = 0;
	LREAL m_stabTemp = 0;

	rTickCount m_timerStart;
	rTickCount m_timerStab;
	rTickCount m_timerWaitUp;

	static rBitsArray m_flagsSetup;
};



