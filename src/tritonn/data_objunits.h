//=================================================================================================
//===
//=== data_objunits.h
//===
//=== Copyright (c) 2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Набор ед. изм. для станции и линии
//===
//=================================================================================================

#pragma once

#include "def.h"
#include "units.h"

class rError;

namespace tinyxml2 {
	class XMLElement;
}


class rObjUnit
{
public:
	rObjUnit() = default;
	virtual ~rObjUnit() = default;

	UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err);

	STRID getTemperature();
	STRID getPressure();
	STRID getDensity();
	STRID getMass();
	STRID getVolume();
	STRID getFlowMass();
	STRID getFlowVolume();
	STRID getMassKF();
	STRID getVolumeKF();

private:
	STRID m_temperature = U_C;
	STRID m_pressure    = U_MPa;
	STRID m_density     = U_kg_m3;
	STRID m_mass        = U_t;
	STRID m_volume      = U_m3;
};
