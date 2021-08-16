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
	rIOCRMInterface() = default;
	virtual ~rIOCRMInterface() = default;

	virtual UINT getDetectors() const = 0;
};



