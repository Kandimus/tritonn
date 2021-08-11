/*
 *
 * io/aiinterface.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "baseinterface.h"

class rIOAIInterface : public rIOBaseInterface
{
public:
	rIOAIInterface() = default;
	virtual ~rIOAIInterface() = default;

	virtual REAL  getCurrent(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UINT  getMinValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UINT  getMaxValue(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual UINT  getRange(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
	virtual USINT getState(USINT num, rIOBaseChannel::Type type, UDINT& fault) = 0;
};



