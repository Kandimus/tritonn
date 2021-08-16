/*
 *
 * data/scale.cpp
 *
 * Copyright (c) 2021 by RangeSoft.
 * All rights reserved.
 *
 * Litvinov "VeduN" Vitaliy O.
 *
 */

#include "scale.h"
#include "../variable_list.h"
#include "../error.h"
#include "xml_util.h"

void rScale::generateVars(rVariableList& list, const std::string& alias, STRID unit)
{
	list.add(alias + ".scales.low" , rVariable::Flags::___D, &m_min.Value, unit, ACCESS_SCALES, "Значение инженерного минимума");
	list.add(alias + ".scales.high", rVariable::Flags::___D, &m_max.Value, unit, ACCESS_SCALES, "Значение инженерного максимума");
}

void rScale::loadFromXml(tinyxml2::XMLElement* root, rError& err)
{
	if (!root) {
		return;
	}

	auto xml_scale = root->FirstChildElement(XmlName::SCALE);

	if (!xml_scale) {
		return;
	}

	UDINT fault = 0;
	m_min.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement(XmlName::LOW) , 0.0, fault));
	m_max.Init(XmlUtils::getTextLREAL(xml_scale->FirstChildElement(XmlName::HIGH), 100.0, fault));

	if (fault) {
		err.set(DATACFGERR_SCALE, xml_scale->GetLineNum(), "error load scale");
	}
}
