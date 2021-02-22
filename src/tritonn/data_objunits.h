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

	UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err);

	STRID getTemperature() const;
	STRID getPressure() const;
	STRID getDensity() const;
	STRID getMass() const;
	STRID getVolume() const;
	STRID getFlowMass() const;
	STRID getFlowVolume() const;
	STRID getMassKF() const;
	STRID getVolumeKF() const;

private:
	STRID m_temperature = U_C;
	STRID m_pressure    = U_MPa;
	STRID m_density     = U_kg_m3;
	STRID m_mass        = U_t;
	STRID m_volume      = U_m3;
};
