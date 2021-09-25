﻿//=================================================================================================
//===
//=== data_source.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-интерфейс для всех объектов, которые могут выступать в роли источника данных
//===
//=================================================================================================

#pragma once

#include <vector>
#include "def.h"
#include "../units.h"
#include "../event/event.h"


class rVariableList;
class rStation;
class rLink;
class rTotal;
class rError;
class rGeneratorMD;

namespace tinyxml2 {
	class XMLElement;
}


const UDINT SOURCE_LE_OUTPUT = 0x40000000;
const UDINT SOURCE_LE_UNIT   = 0x80000000;

//-------------------------------------------------------------------------------------------------
//
class rSource
{
public:
	rSource(const rStation* owner = nullptr);
	virtual ~rSource();

	bool        checkOutput(const std::string& name);
	std::string getMarkDown();

protected:
	// Описание входов-выходов
	//NOTE На данные структуры нельзя делать ссылки при формировании Variables
	std::vector<rLink*> m_inputs;
	std::vector<rLink*> m_outputs;

	rEvent& reinitEvent(UDINT eid);
	rEvent& reinitEvent(rEvent &event, UDINT eid);
	UDINT   checkExpr(bool expr, UDINT flag, rEvent &event_fault, rEvent &event_success);
	UDINT   sendEventSetLE(UDINT flag, rEvent &event);
	UDINT   sendEventClearLE(UDINT flag, rEvent &event);
	rLink*  getOutputByName(const string &name);


public:
	virtual const char *getRTTI() const = 0;

	virtual LREAL       getValue(const std::string& name, UDINT unit, UDINT &err);
	virtual STRID       getValueUnit(const std::string& name, UDINT& err);
	virtual UDINT       getFault();
	virtual UDINT       loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual std::string saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal);
	virtual UDINT       generateMarkDown(rGeneratorMD& md);
	virtual std::string getXmlInput() const;
	virtual std::string getXmlLimits(const std::string& prefix) const;
	virtual UDINT       generateVars(rVariableList& list);
	virtual UDINT       preCalculate();
	virtual UDINT       calculate();
	virtual UDINT       postCalculate();
	virtual UDINT       check(rError& err);

	virtual const rTotal* getTotal(void) const     { return nullptr; }
	virtual rTotal* getTotalNoConst(void)          { return nullptr; }
	virtual std::string   getModuleAlias() const   { return std::string(); }
	virtual USINT         getModuleNumber() const  { return 0xFF; }
	virtual USINT         getChannelNumber() const { return 0xFF; }

	virtual const rStation* getOwner() const { return m_station; }

protected:
	virtual UDINT initLink(UINT setup, rLink& link, UDINT unit, STRID nameid, const std::string& name, const std::string& shadow);
	virtual UDINT initLimitEvent(rLink& link) = 0;
	virtual UDINT reinitLimitEvents();


public:
	std::string m_alias   = ""; // Полное имя объекта, формируется из префикса и xml-атрибута "name"
	STRID       m_descr   = 0;
	UDINT       m_ID      = 0;
	UDINT       m_lineNum = 0;  // Строка в конфигурации

protected:
	const rStation* m_station  = nullptr;
	bool        m_isCalc  = false;        // Флаг того, что вычисления в данный скан уже были сделаны
	UDINT       m_fault   = 0;            // Флаг ошибки вычислений/канала
	UDINT       m_lockErr = 0;            // Флаги, информация о ранее выданных ошибках
	rEvent      m_event;                  // Временная пременная, что бы постоянно не создавать
};


