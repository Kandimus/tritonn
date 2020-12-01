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
		OFF = 0x0001,
		ERR_RESERV  = 0x0002,
		FILL_RESERV = 0x0004,
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
	rLink m_canFilled;
	rLink m_canError;
	rLink m_mass;

	// Outputs
	rLink       m_grab;                 // Текущее физическое значение
	rLink       m_canSelect;

	UINT  m_select = 0;
	UINT  m_setup;
	UINT  m_mode;
	LREAL m_CanVol[2];
	LREAL m_grabVol;

private:
	static rBitsArray m_flagsMode;
	static rBitsArray m_flagsSetup;

};




