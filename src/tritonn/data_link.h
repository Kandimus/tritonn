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
	enum Setup
	{
		INPUT    = 0x0001,
		OUTPUT   = 0x0002,
		SIMPLE   = 0x0004,
		NONAME   = 0x0008,
		WRITABLE = 0x0010,
		VARNAME  = 0x0020,
		INOUTPUT = INPUT | OUTPUT,
	};

	static const std::string SHADOW_NONE;

	virtual ~rLink();
	
	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() const { return "link"; }

	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual UDINT Calculate();
	virtual UDINT generateVars(rVariableList& list);

private:
	virtual UDINT InitLimitEvent(rLink &link);
	virtual LREAL GetValue();

public:
	void  Init(UINT setup, UDINT unit, rSource *owner, const std::string &ioname, STRID descr);
	void  CalculateLimit();
	STRID GetSourceUnit();
	const rSource* getOwner() const;
	bool  isValid() const;

public:
	std::string FullTag = "";         // Полное имя тега-источника (пример "sikn.line.io.temp:present")
	std::string Param   = "";         // Имя параметра-источника (из примера "present")
	std::string IO_Name = "";         // Собственное имя источника данных
	std::string m_varName = "";       // Имя для генерации переменных
	rSource*    m_source = nullptr;    // Источник данных

	STRID       Unit    = U_any;      // Требуемые ед.измерения
	LREAL       Value   = 0.0;        // Полученное значение
	rLimit      Limit;

	UINT        m_setup = 0;
	std::string Shadow  = "";

//private:
	virtual LREAL GetValue(const string &name, UDINT unit, UDINT &err);

protected:
	rSource* m_owner = nullptr;    // Куда привязаны
};



