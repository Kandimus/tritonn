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
#include "data_objunits.h"

class rStream;

class rStation : public rSource
{
public:
	rStation();
	virtual ~rStation();

	rDensity::Product getProduct() const;
	const rObjUnit& getUnit() const;

	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "station"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();

	virtual const rTotal *getTotal(void) const;

protected:
	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs/Outputs
	rLink m_temp;
	rLink m_pres;
	rLink m_dens;
//TODO	rLink        Dens15; нужно ли добавлять?
//TODO	rLink        Dens20; нужно ли добавлять?
	// Outputs
	rLink m_flowMass;
	rLink m_flowVolume;
	rLink m_flowVolume15;
	rLink m_flowVolume20;

	rDensity::Product m_product;
	rCmpUINT     Setup;

	std::vector<rStream*> Stream;

private:
	static rBitsArray m_flagsProduct;

	rTotal       m_total;
	rObjUnit     m_unit;
};



