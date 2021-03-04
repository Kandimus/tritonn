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
#include "bits_array.h"
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

	UINT       m_mark;    // Статус отчета

	// Время
	rReportTime StartTime;
	rReportTime FinalTime;

	std::vector<rReportTotal*> AverageItems;
	std::vector<rReportItem*>  SnapshotItems;

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

	enum Status : UINT
	{
		IDLE      = 0,
		RUNNING   = 1,
		WAITING   = 2,
		COMPLETED = 3,
	};

	enum Mark : UINT
	{
		UNDEF      = 0,
		ILLEGAL    = 1, // Отчет завершен после перезагрузки. Мы не можем гарантировать точность данных
		INCOMPLETE = 2, // Отчет начат после перезагрузки, период отчета не полный
		VALIDATE   = 3,
		INPROGRESS = 4, // Отчет в работе
	};

	enum Command : UINT
	{
		NONE    = 0,
		START   = 1,
		STOP    = 2,
		RESTART = 3,
	};

	rReport();
	virtual ~rReport();

	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const override { return "report"; }

	virtual UDINT       loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT       generateVars(rVariableList& list) override;
	virtual std::string saveKernel(UDINT isio, const std::string& objname, const std::string& comment, UDINT isglobal) override;
	virtual UDINT       calculate() override;
protected:
	virtual UDINT       initLimitEvent(rLink &link) override;

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
	rCmpUSINT      m_command;      // Команда управления партией
	Status         m_status;       // Статус партии
	UINT           ArchiveAccept;  // Флаг для загрузки отчета
	rReportTime    ArchiveTime;    // Время для загрузки отчета в переменную Archive

	rReportDataset Present;        //
	rReportDataset Completed;      //
	rReportDataset Archive;

	// Временные переменные
	Time64_T   PastUNIX;
	struct tm  PastTM;

	static rBitsArray m_flagsMark;

private:
	static rBitsArray m_flagsType;
	static rBitsArray m_flagsPeriod;
	static rBitsArray m_flagsStatus;
	static rBitsArray m_flagsCommand;
};

