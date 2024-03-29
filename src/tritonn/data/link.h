﻿//=================================================================================================
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

#include "source.h"
#include "limit.h"


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
		MUSTVIRT = 0x0040,
		DISABLE  = 0x0080,
		INOUTPUT = INPUT | OUTPUT,
	};

	static const std::string SHADOW_NONE;

	rLink() = default;
	virtual ~rLink() = default;

	void  init(UINT setup, UDINT unit, rSource *owner, const std::string& ioname, STRID descr);
	void  calculateLimit();
	STRID getSourceUnit();
	bool  isValid() const;
	
	// Виртуальные функции от rSource
public:
	virtual const char* getRTTI() const override { return "link"; }

	virtual UDINT getFault() override;
	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT calculate() override;
	virtual UDINT generateVars(rVariableList& list) override;

private:
	virtual UDINT initLimitEvent(rLink &link) override;
	virtual LREAL getValue(const std::string& name, UDINT unit, UDINT& err) override;

private:
	LREAL getValue();

public:
	std::string m_fullTag = "";         // Полное имя тега-источника (пример "sikn.line.io.temp:present")
	std::string m_param   = "";         // Имя параметра-источника (из примера "present")
	std::string m_ioName  = "";          // Собственное имя источника данных
	std::string m_varName = "";         // Имя для генерации переменных
	std::string m_comment = "";
	rSource*    m_source  = nullptr;    // Источник данных

	STRID       m_unit    = U_any;      // Требуемые ед.измерения
	LREAL       m_value   = 0.0;        // Полученное значение
	rLimit      m_limit;

	UINT        m_setup   = 0;
	std::string m_shadow  = "";

//private:


protected:
	rSource* m_owner = nullptr;    // Куда привязаны
};



