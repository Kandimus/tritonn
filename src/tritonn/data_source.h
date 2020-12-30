//=================================================================================================
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
#include "units.h"
#include "event_class.h"


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
	rSource();
	virtual ~rSource();

	std::string Alias     = ""; // Полное имя объекта, формируется из префикса и xml-атрибута "name"
	STRID       Descr     = 0;
	UDINT       ID        = 0;
	UDINT       m_lineNum = 0;

	rStation *Station;

protected:
	UDINT  Calculated;              // Флаг того, что вычисления в данный скан уже были сделаны
	UDINT  Fault;                   // Флаг ошибки вычислений/канала
	UDINT  LockErr;                 // Флаги, информация о ранее выданных ошибках
	rEvent Event;                   // Временная пременная, что бы постоянно не создавать

	// Описание входов-выходов
	//NOTE На данные структуры нельзя делать ссылки при формировании Variables
	std::vector<rLink*> m_inputs;
	std::vector<rLink*> m_outputs;

	rEvent& ReinitEvent(UDINT eid);
	rEvent& ReinitEvent(rEvent &event, UDINT eid);
	UDINT   CheckExpr(bool expr, UDINT flag, rEvent &event_fault, rEvent &event_success);
	UDINT   SendEventSetLE(UDINT flag, rEvent &event);
	UDINT   SendEventClearLE(UDINT flag, rEvent &event);
	rLink*  GetOutputByName(const string &name);

public:
	virtual const char *RTTI() const = 0;

	virtual LREAL GetValue(const string &name, UDINT unit, UDINT &err);
	virtual STRID GetValueUnit(const string &name, UDINT &err);
	virtual UDINT GetFault();
	virtual UDINT LoadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix);
	virtual std::string saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal);
	virtual UDINT       generateMarkDown(rGeneratorMD& md);
	virtual std::string getMarkDown() const;
	virtual std::string getXmlInput() const;
	virtual UDINT generateVars(rVariableList& list) const;
	virtual UDINT PreCalculate();
	virtual UDINT Calculate();
	virtual UDINT PostCalculate();
	virtual UDINT check(rError& err);

	virtual const rTotal *getTotal(void) const { return nullptr; }

protected:
	virtual UDINT InitLink(UINT setup, rLink &link, UDINT unit, UDINT nameid, const string &name, const string &shadow);
	virtual UDINT InitLimitEvent(rLink &link) = 0;
	virtual UDINT ReinitLimitEvents();


public:
	UDINT CheckOutput(const string &name);

};



