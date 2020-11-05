//=================================================================================================
//===
//=== data_limit.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс пределов сигнала
//===
//=================================================================================================

#pragma once

#include "data_source.h"
#include "compared_values.h"

class rVariableList;

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rLimit //: public rSource
{
public:
	rLimit();
	virtual ~rLimit();

public:
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list, const string &owner_name, STRID owner_unit);

public:
	virtual UINT Calculate(LREAL val, UDINT check);

protected:
	void SendEvent(rEvent &e, LREAL *val, LREAL *lim, UDINT dontsend);

public:
	rCmpLREAL AMin;
	rCmpLREAL WMin;
	rCmpLREAL WMax;
	rCmpLREAL AMax;
	rCmpLREAL Hysteresis;              //

	rEvent    EventAMin;
	rEvent    EventWMin;
	rEvent    EventWMax;
	rEvent    EventAMax;
	rEvent    EventNormal;
	rEvent    EventNan;

	rEvent    EventChangeAMin;
	rEvent    EventChangeWMin;
	rEvent    EventChangeWMax;
	rEvent    EventChangeAMax;
	rEvent    EventChangeHyst;
	rEvent    EventChangeSetup;

	UINT      Status = LIMIT_STATUS_UNDEF;
	rCmpUINT  Setup  = LIMIT_SETUP_OFF;
};


