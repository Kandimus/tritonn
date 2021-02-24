//=================================================================================================
//===
//=== data_report.cpp
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

#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "tinyxml2.h"
#include "event_eid.h"
#include "text_id.h"
#include "event_manager.h"
#include "data_manager.h"
#include "data_config.h"
#include "error.h"
#include "variable_item.h"
#include "variable_list.h"
#include "simplefile.h"
#include "data_report.h"
#include "xml_util.h"

rBitsArray rReport::m_flagsType;
rBitsArray rReport::m_flagsPeriod;

void rReportTime::SetCurTime()
{
	GetCurrentTime(_UNIX, &_TM);
}


//
void rReportTime::generateVars(const string &prefix, rVariable::Flags flags, UDINT access, rVariableList& list)
{
	list.add(prefix + "sec"   , TYPE_USINT, flags, &_TM.tm_sec , U_DIMLESS, access);
	list.add(prefix + "min"   , TYPE_USINT, flags, &_TM.tm_min , U_DIMLESS, access);
	list.add(prefix + "hour"  , TYPE_USINT, flags, &_TM.tm_hour, U_DIMLESS, access);
	list.add(prefix + "day"   , TYPE_USINT, flags, &_TM.tm_mday, U_DIMLESS, access);
	list.add(prefix + "month" , TYPE_USINT, flags, &_TM.tm_mon , U_DIMLESS, access);
	list.add(prefix + "year"  , TYPE_UINT , flags, &_TM.tm_year, U_DIMLESS, access);
}


rReportItem::rReportItem()
{
	Value = 0.0;
	Name  = "";
}

rReportItem::rReportItem(rReportItem &src)
{
	Name = src.Name;
}

rReportItem::~rReportItem()
{
	Name = "";
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
rReportTotal::rReportTotal()
{
	Source = nullptr;
}

rReportTotal::rReportTotal(rReportTotal &src)
{
	Source     = nullptr;
	Alias      = "";
	Name       = src.Name;
	UnitMass   = src.UnitMass;
	UnitVolume = src.UnitVolume;

	for(UDINT ii = 0; ii < src.Items.size(); ++ii)
	{
		Items.push_back(new rReportItem(*src.Items[ii]));
	}
}

rReportTotal::~rReportTotal()
{
	Source = nullptr;
	for(UDINT ii = 0; ii < Items.size(); ++ii)
	{
		if(Items[ii]) delete Items[ii];
	}
	Items.clear();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
rReportDataset::rReportDataset()
{
	Mark = REPORT_MARK_UNDEF;
}


rReportDataset::~rReportDataset()
{
	Clear();
}


void rReportDataset::Clear()
{
	for(UDINT ii = 0; ii < AverageItems.size(); ++ii)
	{
		if(AverageItems[ii]) delete AverageItems[ii];
	}

	for(UDINT ii = 0; ii < SnapshotItems.size(); ++ii)
	{
		if(SnapshotItems[ii]) delete SnapshotItems[ii];
	}

	AverageItems.clear();
	SnapshotItems.clear();
}


void rReportDataset::CreateFrom(rReportDataset &ds)
{
	Clear();

	for(UDINT ii = 0; ii < ds.AverageItems.size(); ++ii)
	{
		AverageItems.push_back(new rReportTotal(*ds.AverageItems[ii]));
	}

	for(UDINT ii = 0; ii < ds.SnapshotItems.size(); ++ii)
	{
		SnapshotItems.push_back(new rReportItem(*ds.SnapshotItems[ii]));
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rReport::rReport() : rSource()
{
	if (m_flagsType.empty()) {
		m_flagsType
				.add("PERIODIC", REPORT_PERIODIC)
				.add("BATCH"   , REPORT_BATCH);
	}

	if (m_flagsPeriod.empty()) {
		m_flagsPeriod
				.add("HOUR"     , REPORT_PERIOD_HOUR)
				.add("2HOUR"    , REPORT_PERIOD_2HOUR)
				.add("3HOUR"    , REPORT_PERIOD_3HOUR)
				.add("4HOUR"    , REPORT_PERIOD_4HOUR)
				.add("6HOUR"    , REPORT_PERIOD_6HOUR)
				.add("8HOUR"    , REPORT_PERIOD_8HOUR)
				.add("12HOUR"   , REPORT_PERIOD_12HOUR)
				.add("DAYLY"    , REPORT_PERIOD_DAYLY)
				.add("WEEKLY"   , REPORT_PERIOD_WEEKLY)
				.add("BIWEEKLY" , REPORT_PERIOD_BIWEEKLY)
				.add("MONTHLY"  , REPORT_PERIOD_MONTHLY)
				.add("QUARTERLY", REPORT_PERIOD_QUARTERLY)
				.add("ANNUAL"   , REPORT_PERIOD_ANNUAL)
				.add("5MIN"     , REPORT_PERIOD_5MIN)
				.add("15MIN"    , REPORT_PERIOD_15MIN);

	}

	Status = REPORT_STATUS_IDLE;
}


rReport::~rReport()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::initLimitEvent(rLink& /*link*/)
{
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::calculate()
{
	if (rSource::calculate()) {
		return 0;
	}

	// Обработка периодических отчетов
	if (Type == REPORT_PERIODIC) {
		if (Status == REPORT_STATUS_RUNNING) {
			// Проверка на завершение отчета
			if (CheckFinishPeriodic()) {
				Store();
				Start();
			}

		// Проверка на незавершенный отчет при WarmStart
		} else if (Status == REPORT_STATUS_IDLE) {
			if (Present.StartTime._UNIX) {
				Time64_T curtime = timegm64(NULL);

				// Если время отчета уже вышло, то сохраняем его с меткой "недействительный"
				if(curtime - Present.StartTime._UNIX >= GetUNIXPeriod())
				{
					Present.Mark = REPORT_MARK_ILLEGAL;
					Store();
					Start();
					Present.Mark = REPORT_MARK_INCOMPLETE;
				}
				// Если время еще не вышло, то сохраняем с меткой "неполный"
				else
				{
					Present.Mark = REPORT_MARK_INCOMPLETE;
				}
			}
		}
	}
	// Обработка партионных отчетов
	else if (Type == REPORT_BATCH) {
		;
	} else {
		return 1;
	}


	for(UDINT ii = 0; ii < Present.AverageItems.size(); ++ii) {
		rReportTotal *total    = Present.AverageItems[ii];
		LREAL         oldmass = 0.0;
		LREAL         curmass = 0.0;
		LREAL         inc     = 0.0;

		// Прирост нарастающих за этот цикл
		inc     = rTotal::Sub(total->Source->Present.Mass, total->FinalTotal.Mass);
		// Общая масса на прошлом цикле (сколько прокачали до этого цикла)
		oldmass = rTotal::Sub(total->FinalTotal.Mass     , total->StartTotal.Mass);
		// Общая масса на текущем цикле (сколько прокачали на этом цикле)
		curmass = rTotal::Sub(total->Source->Present.Mass, total->StartTotal.Mass);

		// Сохраняем нарастающие
		total->FinalTotal = total->Source->Present;

		// Устредняем
		for(UDINT jj = 0; jj < total->Items.size(); ++jj)
		{
			rReportItem *itm = total->Items[ii];

			itm->Source.calculate();

			if(curmass <= 0.0) continue;

			itm->Value = ((itm->Value * inc) + (itm->Source.m_value * oldmass)) / curmass;
		}
	}

	for(auto item : Present.SnapshotItems) {
		item->Source.calculate();
		item->Value = item->Source.m_value;
	}

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
void rReportDataset::generateVars(const string &prefix, rVariableList& list)
{
	string name = "";

	list.add(prefix + "status", TYPE_UINT, rVariable::Flags::RS__, &Mark, U_DIMLESS, ACCESS_SA);

	StartTime.generateVars(prefix + "datetime.begin.", rVariable::Flags::RS__, ACCESS_SA, list);
	FinalTime.generateVars(prefix + "datetime.end."  , rVariable::Flags::RS__, ACCESS_SA, list);

	// Формируем переменые
	for(UDINT ii = 0; ii < AverageItems.size(); ++ii)
	{
		rReportTotal *tot = AverageItems[ii];

		name = prefix + tot->Name + ".total.";

		list.add(name + "begin.mass"    , TYPE_LREAL, rVariable::Flags::RS__, &tot->StartTotal.Mass    , tot->UnitMass  , ACCESS_SA);
		list.add(name + "begin.volume"  , TYPE_LREAL, rVariable::Flags::RS__, &tot->StartTotal.Volume  , tot->UnitVolume, ACCESS_SA);
		list.add(name + "begin.volume15", TYPE_LREAL, rVariable::Flags::RS__, &tot->StartTotal.Volume15, tot->UnitVolume, ACCESS_SA);
		list.add(name + "begin.volume20", TYPE_LREAL, rVariable::Flags::RS__, &tot->StartTotal.Volume20, tot->UnitVolume, ACCESS_SA);
		list.add(name + "end.mass"      , TYPE_LREAL, rVariable::Flags::RS__, &tot->FinalTotal.Mass    , tot->UnitMass  , ACCESS_SA);
		list.add(name + "end.volume"    , TYPE_LREAL, rVariable::Flags::RS__, &tot->FinalTotal.Volume  , tot->UnitVolume, ACCESS_SA);
		list.add(name + "end.volume15"  , TYPE_LREAL, rVariable::Flags::RS__, &tot->FinalTotal.Volume15, tot->UnitVolume, ACCESS_SA);
		list.add(name + "end.volume20"  , TYPE_LREAL, rVariable::Flags::RS__, &tot->FinalTotal.Volume20, tot->UnitVolume, ACCESS_SA);

		name = prefix + tot->Name + ".";

		for(UDINT jj = 0; jj < tot->Items.size(); ++jj)
		{
			rReportItem *itm = tot->Items[jj];

			list.add(name + itm->Name, TYPE_LREAL, rVariable::Flags::RS__, &itm->Value, itm->Source.getSourceUnit(), ACCESS_SA);
		}
	}

	name = prefix + "snapshot.";

	for(UDINT ii = 0; ii < SnapshotItems.size(); ++ii)
	{
		rReportItem *itm = SnapshotItems[ii];

		list.add(name + itm->Name, TYPE_LREAL, rVariable::Flags::RS__, &itm->Value, itm->Source.getSourceUnit(), ACCESS_SA);
	}
}



//-------------------------------------------------------------------------------------------------
//
UDINT rReport::generateVars(rVariableList& list)
{
	string name = m_alias + ".";

	rSource::generateVars(list);

	// Общие переменные для всех типов отчетов
	list.add(name + "type"               , TYPE_UINT , rVariable::Flags::R___, &Type         , U_DIMLESS, 0);
	list.add(name + "archive.load.accept", TYPE_UINT , rVariable::Flags::____, &ArchiveAccept, U_DIMLESS, ACCESS_REPORT);

	ArchiveTime.generateVars(name + "archive.load.", rVariable::Flags::____, ACCESS_REPORT, list);

	if(REPORT_PERIODIC == Type)
	{
		list.add(name + "period", TYPE_UINT, rVariable::Flags::____, &Period, U_DIMLESS, 0);
	}
	else
	{
		list.add(name + "command", TYPE_USINT, rVariable::Flags::____, &Command, U_DIMLESS, ACCESS_BATCH);
		list.add(name + "status" , TYPE_UINT , rVariable::Flags::R___, &Status , U_DIMLESS, 0);
	}

	Present.generateVars  (name + "present."  , list);
	Completed.generateVars(name + "completed.", list);
	Archive.generateVars  (name + "archive."  , list);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Поиск требуемого dataset в дереве конфигурации
tinyxml2::XMLElement *rReport::GetDataSetElement(tinyxml2::XMLElement *element, const char *dsname)
{
	tinyxml2::XMLNode *reports = element->Parent();
	tinyxml2::XMLNode *repsys  = nullptr;
	tinyxml2::XMLElement *datasets = nullptr;

	if(nullptr == reports) return nullptr;

	repsys = reports->Parent();

	if(nullptr == repsys) return nullptr;

	datasets = repsys->FirstChildElement(XmlName::DATASETS);

	if(nullptr == datasets) return nullptr;

	for(tinyxml2::XMLElement *ds = datasets->FirstChildElement(XmlName::DATASET); ds != nullptr; ds = ds->NextSiblingElement(XmlName::DATASET))
	{
		if(!strcmp(ds->Attribute(XmlName::NAME), dsname)) return ds;
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strType = XmlUtils::getAttributeString(element, XmlName::TYPE, m_flagsType.getNameByBits(REPORT_PERIODIC));
	UDINT fault = 0;

	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* period  = element->FirstChildElement(XmlName::PERIOD);
	tinyxml2::XMLElement* storage = element->FirstChildElement(XmlName::STORAGE);
	tinyxml2::XMLElement* dsname  = element->FirstChildElement(XmlName::DATASET);

	// Тип отчета
	Type = m_flagsType.getValue(strType, fault);

	if ((Type == REPORT_PERIODIC && !period) || !dsname || fault) {
		return err.set(DATACFGERR_REPORT, element->GetLineNum());
	}

	// Время хранения отчета
	Storage = XmlUtils::getTextUINT(storage, REPORT_DEFAULT_STORAGE, fault);
	if (Storage == 0 || fault) {
		return err.set(DATACFGERR_REPORT, element->GetLineNum(), "invalid storage");
	}

	// Загрузка периодических отчетов
	if (REPORT_PERIODIC == Type) {
		string strPeriod = XmlUtils::getTextString(element->FirstChildElement(XmlName::PERIOD), "", fault);

		Period = m_flagsPeriod.getBit(strPeriod, fault);

		if (fault) {
			return err.set(DATACFGERR_REPORT, element->GetLineNum(), "undefined report period");
		}
	}

	//--------------------------------------------
	// Загружаем DataSet
	auto dataset = GetDataSetElement(element, dsname->GetText());

	if (!dataset) {
		return err.set(DATACFGERR_REPORT, element->GetLineNum(), "can't found dataset");
	}

	// Перебираем станции и линии в dataset
	// Заполняем только объект Present
	XML_FOR(total_xml, dataset, XmlName::TOTALS) {
		auto tot = new rReportTotal();

		Present.AverageItems.push_back(tot);

		tot->Source = nullptr;
		tot->Name   = XmlUtils::getAttributeString(total_xml, XmlName::NAME , "");
		tot->Alias  = XmlUtils::getAttributeString(total_xml, XmlName::ALIAS, "");

		if (tot->Name.empty() || tot->Alias.empty()) {
			return err.set(DATACFGERR_REPORT, total_xml->GetLineNum(), "undefined name");
		}

		XML_FOR(item_xml, total_xml, XmlName::ITEM) {
			rReportItem* item = new rReportItem();

			tot->Items.push_back(item);

			item->Name = XmlUtils::getAttributeString(item_xml, XmlName::NAME, "");

			if (item->Name.empty()) {
				return err.set(DATACFGERR_REPORT, item_xml->GetLineNum(), "undefined name");
			}

			if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(item_xml->FirstChildElement(XmlName::LINK), item->Source)) {
				return err.getError();
			}
		}
	}

	// Загружаем мгновенные данные из dataset
	// Заполняем только объект Present
	auto snapshots_xml = dataset->FirstChildElement(XmlName::SNAPSHOTS);
	if(snapshots_xml != nullptr)
	{
		XML_FOR(item_xml, snapshots_xml, XmlName::ITEM) {
			rReportItem* item = new rReportItem();

			Present.SnapshotItems.push_back(item);

			item->Name = XmlUtils::getAttributeString(item_xml, XmlName::NAME, "");

			if (item->Name.empty()) {
				return err.set(DATACFGERR_REPORT, item_xml->GetLineNum(), "undefined name");
			}

			if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(item_xml->FirstChildElement(XmlName::LINK), item->Source)) {
				return err.getError();
			}
		}
	}

	Completed.CreateFrom(Present);
	Archive.CreateFrom(Present);

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
std::string rReport::saveKernel(UDINT isio, const string &objname, const string &comment, UDINT isglobal)
{
	Present.AverageItems.push_back(new rReportTotal());
	Present.AverageItems.back()->Name = "#totalsource_1";
	Present.AverageItems.back()->Items.push_back(new rReportItem());
	Present.AverageItems.back()->Items.back()->Name = "#item_1";
	Present.AverageItems.back()->Items.push_back(new rReportItem());
	Present.AverageItems.back()->Items.back()->Name = "#item_2";

	Present.SnapshotItems.push_back(new rReportItem());
	Present.SnapshotItems.back()->Name = "#item_1";
	Present.SnapshotItems.push_back(new rReportItem());
	Present.SnapshotItems.back()->Name = "#item_2";

	Completed.CreateFrom(Present);
	Archive.CreateFrom(Present);

	return rSource::saveKernel(isio, objname, comment, isglobal);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
UDINT rReport::Store()
{
	Present.FinalTime.SetCurTime();

	Completed.Mark      = Present.Mark;
	Completed.StartTime = Present.StartTime;
	Completed.FinalTime = Present.FinalTime;
//	TimeStartUNIX       = 0;
//	TimeFinishUNIX      = 0;
	Status              = REPORT_STATUS_COMPLETED;

	for(UDINT ii = 0; ii < Present.AverageItems.size(); ++ii)
	{
		rReportTotal *p_total = Present.AverageItems[ii];
		rReportTotal *c_total = Completed.AverageItems[ii];

		c_total->StartTotal = p_total->StartTotal;
		c_total->FinalTotal = p_total->FinalTotal;

		rTotal::Clear(p_total->StartTotal);
		rTotal::Clear(p_total->FinalTotal);

		for(UDINT jj = 0; jj < p_total->Items.size(); ++jj)
		{
			rReportItem *p_itm = p_total->Items[ii];
			rReportItem *c_itm = c_total->Items[ii];

			c_itm->Value = p_itm->Value;
			p_itm->Value = 0.0;
		}
	}

	for(UDINT ii = 0; ii < Present.SnapshotItems.size(); ++ii)
	{
		rReportItem *p_itm = Present.SnapshotItems[ii];
		rReportItem *c_itm = Completed.SnapshotItems[ii];

		c_itm->Value = p_itm->Value;
		p_itm->Value = 0.0;
	}

	rEventManager::instance().Add(reinitEvent(EID_REPORT_GENERATED));

	// Формируем XML дерево отчета
	SaveToXML();

	return 0;
}


UDINT rReport::Start()
{
	Present.StartTime.SetCurTime();
	Present.Mark  = REPORT_MARK_INPROGRESS;
	Status        = REPORT_STATUS_RUNNING;

	for(UDINT ii = 0; ii < Present.AverageItems.size(); ++ii)
	{
		rReportTotal *total = Present.AverageItems[ii];

		total->StartTotal = total->Source->Present;
		total->FinalTotal = total->Source->Present;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Сохранение отчета в файл
//
///////////////////////////////////////////////////////////////////////////////////////////////////


void PrintElement(tinyxml2::XMLPrinter &printer, const string &name, LREAL value, STRID unit)
{
	printer.OpenElement(name.c_str());

	printer.OpenElement("value");
	printer.PushText(value);
	printer.CloseElement();

	printer.OpenElement("unit");
	printer.PushText(unit);
	printer.CloseElement();

	printer.CloseElement();
}


//-------------------------------------------------------------------------------------------------
void rReportTime::Print(tinyxml2::XMLPrinter &printer, const char *name)
{
	printer.OpenElement(name);

	printer.OpenElement("sec");   printer.PushText(_TM.tm_sec);  printer.CloseElement();
	printer.OpenElement("min");   printer.PushText(_TM.tm_min);  printer.CloseElement();
	printer.OpenElement("hour");  printer.PushText(_TM.tm_hour); printer.CloseElement();
	printer.OpenElement("day");   printer.PushText(_TM.tm_mday); printer.CloseElement();
	printer.OpenElement("month"); printer.PushText(_TM.tm_mon);  printer.CloseElement();
	printer.OpenElement("year");  printer.PushText(_TM.tm_year); printer.CloseElement();

	printer.CloseElement();
}


void rReportItem::Print(tinyxml2::XMLPrinter &printer)
{
	printer.OpenElement("variable");
	printer.PushAttribute("name", Name.c_str());

	printer.OpenElement("value");
	printer.PushText(Value);
	printer.CloseElement();

	printer.OpenElement("unit");
	printer.PushText(Source.getSourceUnit());
	printer.CloseElement();

	printer.CloseElement();
}




void rReportTotal::PrintTotals(tinyxml2::XMLPrinter &printer, const char *name, rBaseTotal &total)
{
	printer.OpenElement(name);

	PrintElement(printer, "mass"    , total.Mass    , UnitMass);
	PrintElement(printer, "volume"  , total.Volume  , UnitVolume);
	PrintElement(printer, "volume15", total.Volume15, UnitVolume);
	PrintElement(printer, "volume20", total.Volume20, UnitVolume);

	printer.CloseElement();
}


void rReportTotal::Print(tinyxml2::XMLPrinter &printer)
{
	printer.OpenElement("group"); printer.PushAttribute("name", Name.c_str());

	printer.OpenElement("totals");
	PrintTotals(printer, "start", StartTotal);
	PrintTotals(printer, "final", FinalTotal);
	printer.CloseElement(); // totals

	printer.OpenElement("variables");
	for(UDINT jj = 0; jj < Items.size(); ++jj)
	{
		Items[jj]->Print(printer);
	}
	printer.CloseElement(); // vars

	printer.CloseElement(); // group
}


void rReportDataset::Print(tinyxml2::XMLPrinter &printer)
{
	printer.OpenElement("mark");
	printer.PushText(Mark);
	printer.CloseElement();

	printer.OpenElement("time");
	StartTime.Print(printer, "start");
	FinalTime.Print(printer, "final");
	printer.CloseElement();

	printer.OpenElement("data");

	// Формируем переменые для Dataset
	for(UDINT ii = 0; ii < AverageItems.size(); ++ii)
	{
		AverageItems[ii]->Print(printer);
	}

	printer.OpenElement("snapshot");
	for(UDINT ii = 0; ii < SnapshotItems.size(); ++ii)
	{
		SnapshotItems[ii]->Print(printer);
	}
	printer.CloseElement(); // snapshot
	printer.CloseElement(); // data
}


UDINT rReport::SaveToXML(UDINT present)
{
	tinyxml2::XMLPrinter printer;
	rReportTime          reptime;

	printer.OpenElement("tritonn");
	printer.PushAttribute("signature", "0123456789ABCDEFGHIJ");

	printer.OpenElement("report");

	printer.OpenElement("type");
	printer.PushText(Type);
	printer.CloseElement();

	if(REPORT_PERIODIC == Type)
	{
		printer.OpenElement("period");
		printer.PushText(Period);
		printer.CloseElement(); //TODO Нужно сюда писать описание а не число
	}

	if(present)
	{
		Present.Print(printer);
		reptime = Present.StartTime;
	}
	else
	{
		Completed.Print(printer);
		reptime = Completed.StartTime;
	}

	printer.CloseElement(); // report
	printer.CloseElement(); // tritonn


	string filename = String_format("%s%s/%lu.xml", DIR_REPORT.c_str(), m_alias.c_str(), reptime._UNIX);
	UDINT   result  = SimpleFileSave(filename, printer.CStr());

	if(result != TRITONN_RESULT_OK)
	{
		rEventManager::instance().Add(reinitEvent(EID_REPORT_GENERATED) << result);
		return 0;
	}

	return 1;
}


UDINT rReport::GetUNIXPeriod()
{
	switch(Period)
	{
		case REPORT_PERIOD_HOUR     : return 3600;
		case REPORT_PERIOD_2HOUR    : return 3600  * 2;
		case REPORT_PERIOD_3HOUR    : return 3600  * 3;
		case REPORT_PERIOD_4HOUR    : return 3600  * 4;
		case REPORT_PERIOD_6HOUR    : return 3600  * 6;
		case REPORT_PERIOD_8HOUR    : return 3600  * 8;
		case REPORT_PERIOD_12HOUR   : return 3600  * 12;
		case REPORT_PERIOD_DAYLY    : return 3600  * 24;
		case REPORT_PERIOD_WEEKLY   : return 86400 * 7;
		case REPORT_PERIOD_BIWEEKLY : return 86400 * 14;
		case REPORT_PERIOD_MONTHLY  : return 86400 * DayInMonthShift(Present.StartTime._TM.tm_year, Present.StartTime._TM.tm_mon);
		case REPORT_PERIOD_QUARTERLY: return 86400 * DayInMonthShift(Present.StartTime._TM.tm_year, Present.StartTime._TM.tm_mon) + 86400 * DayInMonthShift(Present.StartTime._TM.tm_year, Present.StartTime._TM.tm_mon + 1) + 86400 * DayInMonthShift(Present.StartTime._TM.tm_year, Present.StartTime._TM.tm_mon + 2);
		case REPORT_PERIOD_ANNUAL   : return 86400 * (365 + IsLeapYear(Present.StartTime._TM.tm_year));
		case REPORT_PERIOD_5MIN     : return 60 * 5;
		case REPORT_PERIOD_15MIN    : return 60 * 15;
		default: return 0xFFFFFFFF;
	}
}


// Проверка на завершение переодического отчета
UDINT rReport::CheckFinishPeriodic()
{
	Time64_T  curtime = timegm64(NULL);
	struct tm curtm;
	UDINT     result = 0;

	//TODO проверяем не чаще чем раз в минуту
	//if(curtime & 0x04 != 0) continue;

	gmtime64_r(&curtime, &curtm);

	switch(Period)
	{
		case REPORT_PERIOD_HOUR     : result = (PastTM.tm_hour != curtm.tm_hour); break;
		case REPORT_PERIOD_2HOUR    : result = (PastTM.tm_hour != curtm.tm_hour) && (curtm.tm_hour %  2 == 0); break;
		case REPORT_PERIOD_3HOUR    : result = (PastTM.tm_hour != curtm.tm_hour) && (curtm.tm_hour %  3 == 0); break;
		case REPORT_PERIOD_4HOUR    : result = (PastTM.tm_hour != curtm.tm_hour) && (curtm.tm_hour %  4 == 0); break;
		case REPORT_PERIOD_8HOUR    : result = (PastTM.tm_hour != curtm.tm_hour) && (curtm.tm_hour %  8 == 0); break;
		case REPORT_PERIOD_12HOUR   : result = (PastTM.tm_hour != curtm.tm_hour) && (curtm.tm_hour % 12 == 0); break;
		case REPORT_PERIOD_DAYLY    : result = (PastTM.tm_mday != curtm.tm_mday); break;
		case REPORT_PERIOD_WEEKLY   : result = (PastTM.tm_wday != curtm.tm_wday) && (curtm.tm_wday      == 1); break;
		case REPORT_PERIOD_BIWEEKLY : result = (PastTM.tm_wday != curtm.tm_wday) && (curtm.tm_wday      == 1) && ((WeekNumber(curtm) - 1) % 2 == 0); break;
		case REPORT_PERIOD_MONTHLY  : result = (PastTM.tm_mon  != curtm.tm_mon ); break;
		case REPORT_PERIOD_QUARTERLY: result = (PastTM.tm_mon  != curtm.tm_mon ) && ((curtm.tm_mon - 1) % 3 == 0); break;
		case REPORT_PERIOD_ANNUAL   : result = (PastTM.tm_year != curtm.tm_year); break;
		case REPORT_PERIOD_5MIN     : result = (PastTM.tm_min  != curtm.tm_min ) && (curtm.tm_min  %  5 == 0); break;
		case REPORT_PERIOD_15MIN    : result = (PastTM.tm_min  != curtm.tm_min ) && (curtm.tm_min  % 15 == 0); break;
	}

	PastTM = curtm;

	return result;
}



