//=================================================================================================
//===
//=== data_stream.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс измерительной линии
//===
//=================================================================================================

#pragma once

#include "total.h"
#include "compared_values.h"
#include "data_source.h"
#include "data_link.h"
#include "bits_array.h"

class rVariable;


const UINT STR_SETUP_OFF      = 0x0001;


struct rFactorPoint
{
	rCmpLREAL Hz;
	rCmpLREAL Kf;
};

struct rFlowFactor
{
	rCmpLREAL KeypadKF;
	rCmpLREAL KeypadMF;

	rFactorPoint Point[MAX_FACTOR_POINT];
};


//
class rStream : public rSource
{
public:
	enum class Type : USINT
	{
		CORIOLIS   = 1,
		TURBINE    = 2,
		ULTRASONIC = 3,
	};

public:
	rStream();
	virtual ~rStream();

	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "stream"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();

	virtual const rTotal *getTotal(void) const;

protected:
	virtual UDINT InitLimitEvent(rLink &link);

protected:
	LREAL CalcualateKF();
	UDINT CalcTotal();
	UDINT GetUnitKF();

public:
	// Inputs
	rLink       m_counter;
	rLink       m_freq;
	// Inputs/Outputs
	rLink       m_temp;
	rLink       m_pres;
	rLink       m_dens;
	rLink       m_dens15;
	rLink       m_dens20;
	rLink       m_b15;
	rLink       m_y15;
	//Outputs
	rLink       m_flowMass;
	rLink       m_flowVolume;
	rLink       m_flowVolume15;
	rLink       m_flowVolume20;


//	UDINT       Unit; //
	rCmpUINT    Setup;
	Type        m_flowmeter;     // Тип расходомера
	USINT       Maintenance;   // 1 - Линия в ремонте
	USINT       Linearization; // Флаг использования кусочно-линейной апроксимации, а не одного К-фактора
	rFlowFactor Factor;
	rFlowFactor SetFactor;
	LREAL       CurKF;
	UDINT       AcceptKF;
	rTotal      Total;

private:
	static rBitsArray m_flagsFlowmeter;
};



