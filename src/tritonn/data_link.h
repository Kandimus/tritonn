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
	rLink();
	virtual ~rLink();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "link"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT Calculate();
	virtual UDINT GenerateVars(vector<rVariable *> &list);

private:
	virtual UDINT InitLimitEvent(rLink &link);
	virtual LREAL GetValue();

public:
	void  Init(UINT setup, UDINT unit, rSource *owner, const string &ioname, STRID descr);
	void  CalculateLimit();
	STRID GetSourceUnit();

public:
	string    FullTag;                 // Полное имя тега-источника (пример "sikn.line.io.temp:present")
	string    Param;                   // Имя параметра-источника (из примера "present")
	string    IO_Name;                 // Собственное имя источника данных
	rSource  *Source;                  // Источник данных
	rSource  *Owner;                   // Куда привязаны

	STRID     Unit;                    // Требуемые ед.измерения
	LREAL     Value;                   // Полученное значение
	rLimit    Limit;

	UINT      Setup;
	string    Shadow;

//private:
	virtual LREAL GetValue(const string &name, UDINT unit, UDINT &err);
};



