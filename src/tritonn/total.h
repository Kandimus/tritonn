//=================================================================================================
//===
//=== total.h
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

#pragma once


#include "def.h"
#include "event/event.h"

namespace tinyxml2 {
	class XMLElement;
}

class rObjUnit;

struct rBaseTotal
{
	LREAL Mass;
	LREAL Volume;
	LREAL Volume15;
	LREAL Volume20;
	UDINT Count;
};


class rTotal
{
public:
	rTotal(rEvent& mass, rEvent& volume, rEvent& volume15, rEvent& volume20);
	virtual ~rTotal() = default;

	void Calculate(const rObjUnit& unit);

	std::string toXml(const std::string& name) const;
	bool        fromXml(tinyxml2::XMLElement* root, const std::string& name);

	static LREAL Sub(LREAL sub1, LREAL sub2);
	static void  clear(rBaseTotal &total);

protected:
	void checkMax();
	std::string toXmlBase(const char* name, const rBaseTotal& total) const;
	bool        fromXmlBase(tinyxml2::XMLElement* root, rBaseTotal& total);

public:
	rBaseTotal m_past;    // Нарастающие на прошлом скане
	rBaseTotal m_raw;     // Сырые текущие нарастающие (обрезанные до 5 знаков)
	rBaseTotal m_inc;     // Инкремент нарастающих на текущем скане (обрезанные до 5 знаков)
	rBaseTotal m_present; // Текущие нарастающие

protected:
	rEvent m_eventMass;
	rEvent m_eventVolume;
	rEvent m_eventVolume15;
	rEvent m_eventVolume20;
};

