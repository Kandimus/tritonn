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

//#include "io.h"
#include "data_source.h"
#include "bits_array.h"
#include "data_link.h"
#include "compared_values.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//

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

	class rCan
	{
	public:
		// Inputs, Inoutputs
		rLink m_filled;
		rLink m_error;
		rLink m_mass;

		UDINT m_volume;
	};

	enum
	{
		CAN_MAX = 2,
	};

public:
	rSampler();
	virtual ~rSampler();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "sampler"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
//	virtual UDINT InitLimitEvent(rLink &link);

public:
	// Inputs, Inoutputs
	rLink m_ioStart;
	rLink m_ioStop;

	// Outputs
	rLink m_grab;
	rLink m_selected;

	std::string     m_totalsAlias;
	const rTotal*   m_totals;
	const rSampler* m_reserv;

	rCan     m_can[MAX_CAN];
	UINT     m_select = 0;
	rCmpUINT m_setup;
	UINT     m_mode;
	UDINT    m_period;
	UDINT    m_grabTest;
	LREAL    m_grabVol;

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;
};
