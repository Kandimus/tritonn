//=================================================================================================
//===
//=== total.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Нарастающие
//===
//=================================================================================================


#include "total.h"
#include "precision.h"
#include "data_objunits.h"
#include "event/manager.h"
#include "xml_util.h"

rTotal::rTotal(rEvent& mass, rEvent& volume, rEvent& volume15, rEvent& volume20)
{
	m_eventMass     = mass;
	m_eventVolume   = volume;
	m_eventVolume15 = volume15;
	m_eventVolume20 = volume20;

	m_eventMass     << MAX_TOTAL_LIMIT;
	m_eventVolume   << MAX_TOTAL_LIMIT;
	m_eventVolume15 << MAX_TOTAL_LIMIT;
	m_eventVolume20 << MAX_TOTAL_LIMIT;

}

void rTotal::checkMax()
{
	if(m_raw.Mass >= MAX_TOTAL_LIMIT)
	{
		if (m_eventMass.getEID()) {
			rEvent event = m_eventMass;
			rEventManager::instance().add(event);
		}
		m_raw.Mass -= MAX_TOTAL_LIMIT;
	}

	if(m_raw.Volume >= MAX_TOTAL_LIMIT)
	{
		if (m_eventVolume.getEID()) {
			rEvent event = m_eventVolume;
			rEventManager::instance().add(event);
		}
		m_raw.Volume -= MAX_TOTAL_LIMIT;
	}

	if(m_raw.Volume15 >= MAX_TOTAL_LIMIT)
	{
		if (m_eventVolume15.getEID()) {
			rEvent event = m_eventVolume15;
			rEventManager::instance().add(event);
		}
		m_raw.Volume15 -= MAX_TOTAL_LIMIT;
	}

	if(m_raw.Volume20 >= MAX_TOTAL_LIMIT)
	{
		if (m_eventVolume20.getEID()) {
			rEvent event = m_eventVolume20;
			rEventManager::instance().add(event);
		}
		m_raw.Volume20 -= MAX_TOTAL_LIMIT;
	}
}



void rTotal::Calculate(const rObjUnit& unit)
{
	m_past = m_present;

	m_raw.Mass     = Round(m_raw.Mass     + m_inc.Mass    , 5);
	m_raw.Volume   = Round(m_raw.Volume   + m_inc.Volume  , 5);
	m_raw.Volume15 = Round(m_raw.Volume15 + m_inc.Volume15, 5);
	m_raw.Volume20 = Round(m_raw.Volume20 + m_inc.Volume20, 5);
	m_raw.Count   += m_inc.Count;

	checkMax();

	m_present.Mass     = Round(m_raw.Mass    , rPrecision::instance().get(unit.getMass()));
	m_present.Volume   = Round(m_raw.Volume  , rPrecision::instance().get(unit.getVolume()));
	m_present.Volume15 = Round(m_raw.Volume15, rPrecision::instance().get(unit.getVolume()));
	m_present.Volume20 = Round(m_raw.Volume20, rPrecision::instance().get(unit.getVolume()));

	m_inc.Mass         = 0.0;
	m_inc.Volume       = 0.0;
	m_inc.Volume15     = 0.0;
	m_inc.Volume20     = 0.0;
	m_inc.Count        = 0;
}


// Корректная разница нарастающих, с учетом сброса по пределу MAX_TOTAL_LIMIT
LREAL rTotal::Sub(LREAL sub1, LREAL sub2)
{
	LREAL result = sub1 - sub2;

	if(result >= 0.0) return result;

	return (-result) + (MAX_TOTAL_LIMIT - sub2);
}


void rTotal::clear(rBaseTotal &total)
{
	total.Count    = 0;
	total.Mass     = 0.0;
	total.Volume   = 0.0;
	total.Volume15 = 0.0;
	total.Volume20 = 0.0;
}

std::string rTotal::toXmlBase(const char* name, const rBaseTotal& total) const
{
	std::string text = "";

	text += String_format("<%s>", name);
	text += String_format("<%s>%.15f</%s>", XmlName::MASS    , total.Mass    , XmlName::MASS);
	text += String_format("<%s>%.15f</%s>", XmlName::VOLUME  , total.Volume  , XmlName::VOLUME);
	text += String_format("<%s>%.15f</%s>", XmlName::VOLUME15, total.Volume15, XmlName::VOLUME15);
	text += String_format("<%s>%.15f</%s>", XmlName::VOLUME20, total.Volume20, XmlName::VOLUME20);
	text += String_format("<%s>%u</%s>"   , XmlName::COUNT   , total.Count   , XmlName::COUNT);
	text += String_format("</%s>", name);

	return text;
}

std::string rTotal::toXml(const std::string& name) const
{
	std::string text = "";

	text += "<" + name + ">";
	text += toXmlBase(XmlName::PAST   , m_past);
	text += toXmlBase(XmlName::RAW    , m_raw);
	text += toXmlBase(XmlName::INC    , m_inc);
	text += toXmlBase(XmlName::PRESENT, m_present);
	text += "</" + name + ">";

	return text;
}

bool rTotal::fromXmlBase(tinyxml2::XMLElement* root, rBaseTotal& total)
{
	rBaseTotal bt;

	if (!root) {
		return false;
	}

	UDINT err = 0;

	bt.Mass     = XmlUtils::getTextLREAL(root->FirstChildElement(XmlName::MASS)    , 0, err);
	bt.Volume   = XmlUtils::getTextLREAL(root->FirstChildElement(XmlName::VOLUME)  , 0, err);
	bt.Volume15 = XmlUtils::getTextLREAL(root->FirstChildElement(XmlName::VOLUME15), 0, err);
	bt.Volume20 = XmlUtils::getTextLREAL(root->FirstChildElement(XmlName::VOLUME20), 0, err);
	bt.Count    = XmlUtils::getTextUDINT(root->FirstChildElement(XmlName::COUNT)   , 0, err);

	if (err) {
		return false;
	}

	total = bt;

	return true;
}

bool rTotal::fromXml(tinyxml2::XMLElement* root, const std::string& name)
{
	auto xml_item = root->FirstChildElement(name.c_str());

	if (!xml_item) {
		return false;
	}

	bool result = true;

	result &= fromXmlBase(xml_item->FirstChildElement(XmlName::PAST   ), m_past);
	result &= fromXmlBase(xml_item->FirstChildElement(XmlName::RAW    ), m_raw);
	result &= fromXmlBase(xml_item->FirstChildElement(XmlName::INC    ), m_inc);
	result &= fromXmlBase(xml_item->FirstChildElement(XmlName::PRESENT), m_present);

	return result;
}
