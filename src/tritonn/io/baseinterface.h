/*
 *
 * io/baseinterface.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "basechannel.h"

class rIOBaseInterface
{
public:
	rIOBaseInterface() = default;
	virtual ~rIOBaseInterface() = default;

	virtual UDINT getValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UDINT setValue(USINT num, rIOBaseChannel::Type type, UDINT  value) = 0;
};



