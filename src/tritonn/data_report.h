//=================================================================================================
//===
//=== data_report.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс отчетов
//===
//=================================================================================================

#pragma once

//#include "io.h"
#include <time.h>
#include "time64.h"
#include "data_source.h"
#include "total.h"
#include "data_link.h"
#include "variable_item.h"

namespace tinyxml2
{
	class XMLPrinter;
	class XMLElement;
}

struct rReportTime
{
	Time64_T   _UNIX;
	struct tm  _TM;

	void  SetCurTime();
	void  generateVars(const std::string &prefix, rVariable::Flags flags, UDINT access, rVariableList& list);
	void  Print(tinyxml2::XMLPrinter& printer, const char *name);
};


struct rReportItem
{
	rReportItem();
	rReportItem(rReportItem &src);
	virtual ~rReportItem();

	rLink  Source;   // Источник данных
	string Name;     // Имя для переменных
	LREAL  Value;    // Значение в текущем отчете

	void Print(tinyxml2::XMLPrinter& printer);
};


//
// Класс средневзвешанных по источнику значений
class rReportTotal
{
public:
	rReportTotal();
	rReportTotal(rReportTotal &src);
	virtual ~rReportTotal();

public:
	const rTotal *Source;               // Источник нарастающих
	string        Alias;                // Название источника данных
	string        Name;                 // Имя для переменных

	rBaseTotal    StartTotal;           // Нарастающие на начало отчета
	rBaseTotal    FinalTotal;           // Текущие нарастающие (должны совпадать с источником)

	STRID         UnitMass;             // Ед. измерения массы
	STRID         UnitVolume;           // Ед. измерения объема

	std::vector<rReportItem *> Items;        // Список переменных для средневзвешивания

	void Print(tinyxml2::XMLPrinter& printer);
	void PrintTotals(tinyxml2::XMLPrinter& printer, const char *name, rBaseTotal &total);
};



class rReportDataset
{
public:
	rReportDataset();
	virtual ~rReportDataset();

	UINT       Mark;    // Статус отчета

	// Время
	rReportTime StartTime;
	rReportTime FinalTime;

	vector<rReportTotal *> AverageItems;
	vector<rReportItem  *> SnapshotItems;

	void  Clear();
	void  Print(tinyxml2::XMLPrinter &printer);
	void  CreateFrom(rReportDataset &ds);
	void  generateVars(const std::string &prefix, rVariableList& list);
};


//-------------------------------------------------------------------------------------------------
// Класс отчета
class rReport : public rSource
{
public:
	rReport();
	virtual ~rReport();

	// Виртуальные функции от rSource
public:
	virtual const char *RTTI() { return "report"; }

	virtual UDINT LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg);
	virtual UDINT generateVars(rVariableList& list);
	virtual std::string saveKernel(UDINT isio, const std::string &objname, const std::string &comment, UDINT isglobal);
	virtual UDINT Calculate();
protected:
	virtual UDINT InitLimitEvent(rLink &link);

protected:
	UDINT Store();
	UDINT Start();
	UDINT SaveToXML(UDINT present = false);
	UDINT GetUNIXPeriod();
	UDINT CheckFinishPeriodic();

	tinyxml2::XMLElement *GetDataSetElement(tinyxml2::XMLElement *element, const char *dsname);


public:
	UINT           Type;           // Тип отчета
	UINT           Storage;        // Время хранения отчета в сутках
	// Для периодических очетов
	USINT          Period;         // Период отчета (см.REPORT_PERIOD_*)
	// Для партионных
	rCmpUSINT      Command;        // Команда управления партией
	UINT           Status;         // Статус партии
	UINT           ArchiveAccept;  // Флаг для загрузки отчета
	rReportTime    ArchiveTime;    // Время для загрузки отчета в переменную Archive

	rReportDataset Present;        //
	rReportDataset Completed;      //
	rReportDataset Archive;

	// Временные переменные
	Time64_T   PastUNIX;
	struct tm  PastTM;
};

