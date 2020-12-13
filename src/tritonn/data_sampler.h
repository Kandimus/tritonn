//=================================================================================================
//===
//=== data_sampler.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пробоотборника
//===
//=================================================================================================

#pragma once

#include "data_source.h"
#include "bits_array.h"
#include "data_link.h"
#include "compared_values.h"


class rSampler : public rSource
{
public:
	enum class Mode : UINT
	{
		PERIOD = 0,
		MASS   = 1,
		VOLUME = 2,
	};

	enum Setup : UINT
	{
		OFF         = 0x0001,
		ERR_RESERV  = 0x0002,
		FILL_RESERV = 0x0004,
		SINGLE_CAN  = 0x0008,
		DUAL_CAN    = 0x0010,
		AUTOSWITCH  = 0x0020,
	};

	enum class Command : UINT
	{
		NONE = 0,
		START = 1,
		STOP = 2,
		TEST = 3,
	};

	struct Can
	{
		UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);

		// Inputs, Inoutputs
		rLink m_overflow;
		rLink m_fault;
		rLink m_weight;

		LREAL m_volume;
	};

	const static UDINT CAN_MAX = 2;

private:
	enum class State : UINT
	{
		IDLE = 0,
		TEST,
		WORKTIME,
		WORKVOLUME,
		WORKMASS,
	};

public:
	rSampler();
	virtual ~rSampler();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "sampler"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal);
	virtual UDINT Calculate();
	virtual UDINT check(rError& err);
protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs, Inoutputs
	rLink m_ioStart;
	rLink m_ioStop;

	// Outputs
	rLink m_grab;
	rLink m_selected;

	Can      m_can[CAN_MAX];
	Command  m_command;
	UINT     m_select = 0;
	rCmpUINT m_setup;
	Mode     m_mode;
	UDINT    m_probePeriod;        // Период времени для работы по таймеру
	LREAL    m_probeVolume;
	LREAL    m_probeMass;
	UDINT    m_grabTest;      // Кол-во тестовых доз
	LREAL    m_grabVol;       // Объем единичной дозы
	LREAL    m_volume;        // Требуемый объем емкости

	LREAL    m_interval;      // объем/масса между дозами или время, в мсек, между дозами, при отборе по времени
	LREAL    m_volRemain;     // Оставшийся объем пробы
	UDINT    m_grabCount;     // Кол-во отобранных проб
	UDINT    m_grabRemain;    // Кол-во оставшихся проб
	State    m_state = State::IDLE;

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;

	const rTotal*   m_totals  = nullptr;
	const rSampler* m_reserve = nullptr;

	std::string m_totalsAlias  = "";
	std::string m_reserveAlias = "";

private:
	void onStart();
};
