﻿//=================================================================================================
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

	std::string toXml(const char* name) const;

	static LREAL Sub(LREAL sub1, LREAL sub2);
	static void  clear(rBaseTotal &total);

protected:
	void checkMax();
	std::string toXmlBase(const char* name, const rBaseTotal& total) const;

public:
	rBaseTotal Past;    // Нарастающие на прошлом скане
	rBaseTotal Raw;     // Сырые текущие нарастающие (обрезанные до 5 знаков)
	rBaseTotal Inc;     // Инкремент нарастающих на текущем скане (обрезанные до 5 знаков)
	rBaseTotal Present; // Текущие нарастающие

protected:
	rEvent m_eventMass;
	rEvent m_eventVolume;
	rEvent m_eventVolume15;
	rEvent m_eventVolume20;
};

