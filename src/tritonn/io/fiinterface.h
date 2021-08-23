/*
 *
 * io/fiinterface.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "baseinterface.h"

class rIOFIInterface : public rIOBaseInterface
{
public:
	rIOFIInterface() = default;
	virtual ~rIOFIInterface() = default;

	virtual LREAL getFreq(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UDINT setOut(USINT num) = 0;
};



