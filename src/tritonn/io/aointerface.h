/*
 *
 * io/aointerface.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "baseinterface.h"

class rIOAOInterface : public rIOBaseInterface
{
public:
	rIOAOInterface() = default;
	virtual ~rIOAOInterface() = default;

	virtual UINT  getMinValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UINT  getMaxValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UINT  getRange(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
};



