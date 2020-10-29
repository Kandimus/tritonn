//=================================================================================================
//===
//=== data_station.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== 
//===
//=================================================================================================

#pragma once

#include <vector>
#include "total.h"
#include "data_link.h"
#include "compared_values.h"

using std::vector;

class rStream;

class rStation : public rSource
{
public:
	rStation();
	virtual ~rStation();

	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "station"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list);
	virtual UDINT saveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();

	virtual const rTotal *GetTotal(void);

protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	UDINT GetUnitFlowVolume();
	UDINT GetUnitFlowMass();

	// Inputs
	rLink        Temp;
	rLink        Pres;
	rLink        Dens;
//TODO	rLink        Dens15; нужно ли добавлять?
//TODO	rLink        Dens20; нужно ли добавлять?
	// Outputs
	rLink        FlowMass;
	rLink        FlowVolume;
	rLink        FlowVolume15;
	rLink        FlowVolume20;

	TYPE_PRODUCT Product;
	rCmpUINT     Setup;
	rTotal       Total;
	UDINT        UnitVolume; // Единицы измерений объема станиции
	UDINT        UnitMass; // Единицы измерений массы станиции

	UDINT        LockErr;

	vector<rStream *> Stream;

};



