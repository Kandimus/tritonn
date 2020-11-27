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

class rVariable;


const UINT STR_SETUP_OFF      = 0x0001;

const UINT STR_FLOWMETER_CARIOLIS   = 1;
const UINT STR_FLOWMETER_TURBINE    = 2;
const UINT STR_FLOWMETER_ULTRASONIC = 3;


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
	rStream();
	virtual ~rStream();

	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "stream"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();

	virtual const rTotal *GetTotal(void);

protected:
	virtual UDINT InitLimitEvent(rLink &link);


public:
	// Inputs
	rLink       Counter;
	rLink       Freq;
	rLink       Temp;
	rLink       Pres;
	rLink       Dens;
	rLink       Dens15;
	rLink       Dens20;
	rLink       B15;
	rLink       Y15;
	//Outputs
	rLink       FlowMass;
	rLink       FlowVolume;
	rLink       FlowVolume15;
	rLink       FlowVolume20;


//	UDINT       Unit; //
	rCmpUINT    Setup;
	USINT       FlowMeter;     // Тип расходомера
	USINT       Maintenance;   // 1 - Линия в ремонте
	USINT       Linearization; // Флаг использования кусочно-линейной апроксимации, а не одного К-фактора
	rFlowFactor Factor;
	rFlowFactor SetFactor;
	LREAL       CurKF;
	UDINT       AcceptKF;
	rTotal      Total;

protected:
	LREAL CalcualateKF();
	UDINT CalcTotal();
	UDINT GetUnitKF();
};



