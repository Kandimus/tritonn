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
#include "density.h"
#include "total.h"
#include "data_link.h"
#include "compared_values.h"
#include "bits_array.h"

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
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
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

	rDensity::Product m_product;
	rCmpUINT     Setup;
	rTotal       Total;
	UDINT        UnitVolume; // Единицы измерений объема станиции
	UDINT        UnitMass; // Единицы измерений массы станиции

	std::vector<rStream*> Stream;

private:
	static rBitsArray m_flagsProduct;
};



