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
		ABORT,
		RESET,
	};

	enum Setup : UINT
	{
		NONE          = 0x0000,
		VALVE_4WAY    = 0x0001,
		STABILIZATION = 0x0002,
		NOVALVE       = 0x0004,
		ONEDETECTOR   = 0x0008,
		BOUNCE        = 0x0010,
		NOSELECTSTR   = 0x0020,
		SIMULATE      = 0x0040,
	};

	enum Way : UINT
	{
		FORWARD = 0,
		REVERSE = 1,
	};

	enum class State : USINT
	{
		IDLE = 0,
		START,
		STABILIZATION,
		VALVETOUP,
		WAITTOUP,
		VALVETODOWN,
		WAITD1,
		WAITD2,
		CALCULATE,
		RETURNBALL,
		FINISH,
		ABORT,
		WAITD2_REVERSE,
		WAITD1_REVERSE,
		ERRORFLOW = 65,
		ERRORSTAB,
		ERRORTOUP,
		ERRORTODOWN,
		ERRORD1,
		ERRORD2,
		ERRORDETECTOR,
		ERRORRETURN,
		ERRORSTREAMID,
		ERRORD2_REVERSE,
		ERRORD1_REVERSE,
	};

	rProve(const rStation* owner = nullptr);
	virtual ~rProve() = default;
	
	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const { return "prove"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual UDINT generateMarkDown(rGeneratorMD& md);
	virtual UDINT calculate();
protected:
	virtual UDINT initLimitEvent(rLink& link);

private:
	void onIdle();
	void onStart();
	void onNoFlow();
	void onStabilization();
	void onValveToUp();
	void onWaitToUp();
	void onValveToDown();
	void onWaitD1();
	void onWaitD2();
	void onCalculate();
	void onReturnBall();
	void onAbort();
	void onErrorState();

	bool checkCommand();
	DINT checkDetectors(bool first);
	bool checkStab(LREAL start, LREAL present, LREAL maxstab, UDINT eid);

	void clearAverage();
	void calcAverage();
	void detectorsProcessing();
	bool connectToLine();

	void moduleStart();
	void moduleStop();

	void setState(State state);

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
	Command   m_command = Command::NONE;
	rCmpUINT  m_setup   = static_cast<UINT>(Setup::NONE);
	rCmpUINT  m_strIdx;
	USINT     m_way = Way::FORWARD;
	LREAL     m_prvFreq = 0;
	LREAL     m_prvTemp = 0;
	LREAL     m_prvPres = 0;
	LREAL     m_prvDens = 0;
	LREAL     m_prvCount[2];
	LREAL     m_prvTime[2];
	LREAL     m_strTemp = 0;
	LREAL     m_strPres = 0;
	LREAL     m_strDens = 0;
	UINT      m_curDet  = 0;
	UINT      m_fixDet  = 0;

	LREAL    m_maxStabTemp = 0.2;
	LREAL    m_maxStabPres = 0.01;
	LREAL    m_maxStabDens = 0.5;
	LREAL    m_maxStabFreq = 10.0;

	UDINT    m_tStart  = 1000;
	UDINT    m_tStab   = 10000;
	UDINT    m_tD1     = 8000;  // Время от старта до 1(2) детектора
	UDINT    m_tD2     = 8000;  // Время от 3(4) детектора до попадания шара в корзину
	UDINT    m_tVolume = 20000; //
	UDINT    m_tValve  = 5000;  //
	UDINT    m_tAbort  = 5000;
	UDINT    m_tBounce = 1000;

private:
	State m_state    = State::IDLE;

	LREAL m_stabDens = 0;
	LREAL m_stabPres = 0;
	LREAL m_stabTemp = 0;
	LREAL m_stabFreq = 0;

	LREAL m_curStrTemp = 0;
	LREAL m_curStrPres = 0;
	LREAL m_curStrDens = 0;

	bool  m_enableAverage = false;
	UDINT m_averageCount  = 0;

	std::string m_moduleName  = "";
	LREAL       m_moduleFreq  = 0.0;
	UINT        m_moduleDet   = 0;
	UDINT       m_moduleCount = 0;

	rTickCount  m_timer;
	rTickCount  m_timerBounce;

	static rBitsArray m_flagsSetup;
	static rBitsArray m_flagsCommand;
	static rBitsArray m_flagsState;
	static rBitsArray m_flagsWay;
};



