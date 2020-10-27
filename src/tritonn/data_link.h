//=================================================================================================
//===
//=== data_link.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс линка на объект rSource, где требуется не дефолтные входа/выхода
//===
//=================================================================================================

#pragma once

#include "data_source.h"
#include "data_limit.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
class rLink : public rSource
{
public:
	virtual ~rLink();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "link"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT Calculate();
	virtual UDINT generateVars(rVariableList& list);

private:
	virtual UDINT InitLimitEvent(rLink &link);
	virtual LREAL GetValue();

public:
	void  Init(UINT setup, UDINT unit, rSource *owner, const std::string &ioname, STRID descr);
	void  CalculateLimit();
	STRID GetSourceUnit();

public:
	std::string FullTag = "";         // Полное имя тега-источника (пример "sikn.line.io.temp:present")
	std::string Param   = "";         // Имя параметра-источника (из примера "present")
	std::string IO_Name = "";         // Собственное имя источника данных
	rSource*    Source  = nullptr;    // Источник данных
	rSource*    Owner   = nullptr;    // Куда привязаны

	STRID       Unit    = U_any;      // Требуемые ед.измерения
	LREAL       Value   = 0.0;        // Полученное значение
	rLimit      Limit;

	UINT        Setup   = 0;
	std::string Shadow  = "";

//private:
	virtual LREAL GetValue(const string &name, UDINT unit, UDINT &err);
};



