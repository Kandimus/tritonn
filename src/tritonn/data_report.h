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


//-------------------------------------------------------------------------------------------------
// Класс отчета
class rReport : public rSource
{
public:

	enum Type : UINT
	{
		PERIODIC = 0,
		BATCH,
	};

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

	enum Period : UINT
	{
		HOUR      = 0,
		HOUR_2    = 1,
		HOUR_3    = 2,
		HOUR_4    = 3,
		HOUR_6    = 4,
		HOUR_8    = 5,
		HOUR_12   = 6,
		DAYLY     = 7,
		WEEKLY    = 8,
		BIWEEKLY  = 9,
		MONTHLY   = 10,
		QUARTERLY = 11,
		ANNUAL    = 12,
		MIN_5     = 13,
		MIN_15    = 14,
	};

	rReport();
	virtual ~rReport() = default;

	struct rItem
	{
		rItem() = default;
		rItem(rItem& src);
		virtual ~rItem() = default;

		rLink  m_source;      // Источник данных
		string m_name  = "";  // Имя для переменных
		LREAL  m_value = 0.0; // Значение в текущем отчете

		void print(tinyxml2::XMLPrinter& printer);
	};

	class rTotal
	{
	public:
		rTotal() = default;
		rTotal(rTotal& src);
		virtual ~rTotal();

	public:
		const ::rTotal *m_source = nullptr;   // Источник нарастающих
		string     m_alias = "";            // Название источника данных
		string     m_name  = "";            // Имя для переменных

		rBaseTotal m_startTotal;           // Нарастающие на начало отчета
		rBaseTotal m_finalTotal;           // Текущие нарастающие (должны совпадать с источником)

		STRID      m_unitMass;             // Ед. измерения массы
		STRID      m_unitVolume;           // Ед. измерения объема

		std::vector<rReport::rItem*> m_items; // Список переменных для средневзвешивания

		void print(tinyxml2::XMLPrinter& printer);
		void printTotals(tinyxml2::XMLPrinter& printer, const char* name, const rBaseTotal& total);
	};


	class rDataset
	{
	public:
		rDataset();
		virtual ~rDataset();

		Mark m_mark;    // Статус отчета

		// Время
		rReportTime StartTime;
		rReportTime FinalTime;

		std::vector<rReport::rTotal*> m_averageItems;
		std::vector<rReport::rItem* > m_snapshotItems;

		void  clear();
		void  print(tinyxml2::XMLPrinter &printer);
		void  createFrom(rDataset &ds);
		void  generateVars(const std::string &prefix, rVariableList& list);
	};

	bool isPeriodic() const { return m_type == Type::PERIODIC; }

protected:
	UDINT Store();
	UDINT Start();
	UDINT SaveToXML(UDINT present = false);
	UDINT GetUNIXPeriod();
	UDINT CheckFinishPeriodic();

	tinyxml2::XMLElement *GetDataSetElement(tinyxml2::XMLElement *element, const char *dsname);


	// Виртуальные функции от rSource
public:
	virtual const char* RTTI() const override { return "report"; }

	virtual UDINT loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix) override;
	virtual UDINT generateVars(rVariableList& list) override;
	virtual UDINT generateMarkDown(rGeneratorMD& md) override;
	virtual UDINT calculate() override;
protected:
	virtual UDINT initLimitEvent(rLink &link) override;

public:
	Type           m_type;         // Тип отчета
	UINT           m_storage = 31; // Время хранения отчета в сутках
	// Для периодических очетов
	Period         m_period;       // Период отчета
	// Для партионных
	rCmpUSINT      m_command;      // Команда управления партией
	Status         m_status;       // Статус партии
	USINT          ArchiveAccept;  // Флаг для загрузки отчета
	rReportTime    ArchiveTime;    // Время для загрузки отчета в переменную Archive

	rDataset m_present;        //
	rDataset m_completed;      //
	rDataset m_archive;

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

