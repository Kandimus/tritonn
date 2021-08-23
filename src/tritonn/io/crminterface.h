/*
 *
 * io/crminterface.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "baseinterface.h"

class rIOCRMInterface : public rIOBaseInterface
{
public:
	enum class State : UINT
	{
		IDLE = 0,
		WAIT_1,
		WAIT_2,
		FINISH,
		TIMEOUT,
	};

	rIOCRMInterface() = default;
	virtual ~rIOCRMInterface() = default;

	virtual LREAL getFreq() = 0;
	virtual UINT  getDetectors() = 0;
	virtual State getState(USINT idx) = 0;
	virtual LREAL getTime(USINT idx) = 0;
	virtual LREAL getImp(USINT idx) = 0;
	virtual bool  start() = 0;
	virtual bool  abort() = 0;
};



