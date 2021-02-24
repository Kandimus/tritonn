﻿//=================================================================================================
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
	virtual ~rStation() = default;

	rDensity::Product getProduct() const;
	const rObjUnit& getUnit() const;
	UDINT addStream(rStream* str);
	UDINT getStreamCount() const;
	UDINT setStreamFreqOut(UDINT strid) const;

	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const { return "station"; }

	virtual UDINT       loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT       generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT       calculate();
	virtual const rTotal *getTotal(void) const;

protected:
	virtual UDINT       initLimitEvent(rLink& link);

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
	rCmpUINT m_setup;

private:
	static rBitsArray m_flagsProduct;

	std::vector<rStream*> m_stream;

	rTotal       m_total;
	rObjUnit     m_unit;
};



