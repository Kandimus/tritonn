/*
 *
 * data/scale.h
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#pragma once

#include "def.h"
#include "../compared_values.h"

class rVariableList;
class rError;

namespace tinyxml2 {
	class XMLElement;
}

class rScale
{
public:
	rScale() : m_min(0), m_max(100) {}
	virtual ~rScale() = default;

	void  generateVars(rVariableList& list, const std::string& alias, STRID unit);
	LREAL getRange() const { return m_max.Value - m_min.Value; }
	void  loadFromXml(tinyxml2::XMLElement* root, rError& err);

public:
	rCmpLREAL m_min;
	rCmpLREAL m_max;
};
