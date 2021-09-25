﻿/*
 *
 * data/stream.h
 *
 * Copyright (c) 2019-2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "bits_array.h"
#include "link.h"
#include "../total.h"
#include "../compared_values.h"

class rVariable;

struct rFactorPoint
{
	rFactorPoint() = default;
	rFactorPoint(UDINT id, LREAL hz, LREAL kf) : m_id(id), Hz(hz), Kf(kf) {}
	virtual ~rFactorPoint() = default;

	UDINT     m_id;
	rCmpLREAL Hz;
	rCmpLREAL Kf;
};

struct rFlowFactor
{
	enum {
		MAXPOINTS = 12
	};

	static std::string FOOTNOTE;

	rCmpLREAL KeypadKF;
	rCmpLREAL KeypadMF;

	std::vector<rFactorPoint> m_point;
};


//
class rStream : public rSource
{
public:
	enum class Type : USINT
	{
		CORIOLIS   = 1,
		TURBINE    = 2,
		ULTRASONIC = 3,
	};

	enum Setup : UINT
	{
		OFF = 0x0001,
	};

public:
	rStream(const rStation* owner = nullptr);
	virtual ~rStream() =  default;

	UDINT enableFreqOut() const;
	UDINT disableFreqOut() const;

	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "stream"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
	virtual const rTotal *getTotal(void) const override { return &m_total; }
	virtual rTotal* getTotalNoConst(void) override { return &m_total; }

protected:
	virtual UDINT initLimitEvent(rLink &link) override;

protected:
	LREAL calcualateKF();
	void  calcTotal();
	UDINT getUnitKF() const;

public:
	// Inputs
	rLink m_counter;
	rLink m_freq;
	// Inputs/Outputs
	rLink m_temp;
	rLink m_pres;
	rLink m_dens;
	rLink m_dens15;
	rLink m_dens20;
	rLink m_b15;
	rLink m_y15;
	//Outputs
	rLink m_flowMass;
	rLink m_flowVolume;
	rLink m_flowVolume15;
	rLink m_flowVolume20;

	rCmpUINT    m_setup;
	Type        m_flowmeter;     // Тип расходомера
	USINT       m_maintenance;   // 1 - Линия в ремонте
	USINT       m_linearization; // Флаг использования кусочно-линейной апроксимации, а не одного К-фактора
	USINT       m_resetTotals = 0;
	rFlowFactor m_curFactor;
	rFlowFactor m_setFactor;
	LREAL       m_curKF;
	USINT       m_acceptKF;
	rTotal      m_total;

private:
	static rBitsArray m_flagsFlowmeter;
	static rBitsArray m_flagsSetup;
};


