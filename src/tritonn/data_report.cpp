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
#include "data_variable.h"
#include "simplefile.h"
#include "data_report.h"
#include "xml_util.h"



void rReportTime::SetCurTime()
{
	GetCurrentTime(_UNIX, &_TM);
}


//
void rReportTime::GenerateVars(const string &prefix, UINT flags, UDINT access, vector<rVariable *> &list)
{
	list.push_back(new rVariable(prefix + "sec"   , TYPE_USINT, flags, &_TM.tm_sec , U_DIMLESS, access));
	list.push_back(new rVariable(prefix + "min"   , TYPE_USINT, flags, &_TM.tm_min , U_DIMLESS, access));
	list.push_back(new rVariable(prefix + "hour"  , TYPE_USINT, flags, &_TM.tm_hour, U_DIMLESS, access));
	list.push_back(new rVariable(prefix + "day"   , TYPE_USINT, flags, &_TM.tm_mday, U_DIMLESS, access));
	list.push_back(new rVariable(prefix + "month" , TYPE_USINT, flags, &_TM.tm_mon , U_DIMLESS, access));
	list.push_back(new rVariable(prefix + "year"  , TYPE_UINT , flags, &_TM.tm_year, U_DIMLESS, access));
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
	Status = REPORT_STATUS_IDLE;
}


rReport::~rReport()
{
	;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::InitLimitEvent(rLink &/*link*/)
{
	return 0;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::Calculate()
{
	if(rSource::Calculate()) return 0;

	// Обработка периодических отчетов
	if(Type == REPORT_PERIODIC)
	{
		if(Status == REPORT_STATUS_RUNNING)
		{
			// Проверка на завершение отчета
			if(CheckFinishPeriodic())
			{
				Store();
				Start();
			}
		}
		// Проверка на незавершенный отчет при WarmStart
		else if(Status == REPORT_STATUS_IDLE)
		{
			if(Present.StartTime._UNIX)
			{
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
	else if(Type == REPORT_BATCH)
	{

	}
	else return 1;



	for(UDINT ii = 0; ii < Present.AverageItems.size(); ++ii)
	{
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

			itm->Source.Calculate();

			if(curmass <= 0.0) continue;

			itm->Value = ((itm->Value * inc) + (itm->Source.Value * oldmass)) / curmass;
		}
	}

	for(UDINT ii = 0; ii < Present.SnapshotItems.size(); ++ii)
	{
		rReportItem *itm = Present.SnapshotItems[ii];

		itm->Source.Calculate();
		itm->Value = itm->Source.Value;
	}

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	PostCalculate();
		
	return 0;
}







///////////////////////////////////////////////////////////////////////////////////////////////////
//





//-------------------------------------------------------------------------------------------------
//
void rReportDataset::GenerateVars(const string &prefix, vector<rVariable *> &list)
{
	string name = "";

	list.push_back(new rVariable(prefix + "status", TYPE_UINT, VARF_RS__, &Mark, U_DIMLESS, ACCESS_SA));

	StartTime.GenerateVars(prefix + "datetime.begin.", VARF_RS__, ACCESS_SA, list);
	FinalTime.GenerateVars(prefix + "datetime.end."  , VARF_RS__, ACCESS_SA, list);

	// Формируем переменые
	for(UDINT ii = 0; ii < AverageItems.size(); ++ii)
	{
		rReportTotal *tot = AverageItems[ii];

		name = prefix + tot->Name + ".total.";

		list.push_back(new rVariable(name + "begin.mass"    , TYPE_LREAL, VARF_RS__, &tot->StartTotal.Mass    , tot->UnitMass  , ACCESS_SA));
		list.push_back(new rVariable(name + "begin.volume"  , TYPE_LREAL, VARF_RS__, &tot->StartTotal.Volume  , tot->UnitVolume, ACCESS_SA));
		list.push_back(new rVariable(name + "begin.volume15", TYPE_LREAL, VARF_RS__, &tot->StartTotal.Volume15, tot->UnitVolume, ACCESS_SA));
		list.push_back(new rVariable(name + "begin.volume20", TYPE_LREAL, VARF_RS__, &tot->StartTotal.Volume20, tot->UnitVolume, ACCESS_SA));
		list.push_back(new rVariable(name + "end.mass"      , TYPE_LREAL, VARF_RS__, &tot->FinalTotal.Mass    , tot->UnitMass  , ACCESS_SA));
		list.push_back(new rVariable(name + "end.volume"    , TYPE_LREAL, VARF_RS__, &tot->FinalTotal.Volume  , tot->UnitVolume, ACCESS_SA));
		list.push_back(new rVariable(name + "end.volume15"  , TYPE_LREAL, VARF_RS__, &tot->FinalTotal.Volume15, tot->UnitVolume, ACCESS_SA));
		list.push_back(new rVariable(name + "end.volume20"  , TYPE_LREAL, VARF_RS__, &tot->FinalTotal.Volume20, tot->UnitVolume, ACCESS_SA));

		name = prefix + tot->Name + ".";

		for(UDINT jj = 0; jj < tot->Items.size(); ++jj)
		{
			rReportItem *itm = tot->Items[jj];

			list.push_back(new rVariable(name + itm->Name, TYPE_LREAL, VARF_RS__, &itm->Value, itm->Source.GetSourceUnit(), ACCESS_SA));
		}
	}

	name = prefix + "snapshot.";

	for(UDINT ii = 0; ii < SnapshotItems.size(); ++ii)
	{
		rReportItem *itm = SnapshotItems[ii];

		list.push_back(new rVariable(name + itm->Name, TYPE_LREAL, VARF_RS__, &itm->Value, itm->Source.GetSourceUnit(), ACCESS_SA));
	}
}



//-------------------------------------------------------------------------------------------------
//
UDINT rReport::GenerateVars(vector<rVariable *> &list)
{
	string name = Alias + ".";

	rSource::GenerateVars(list);

	// Общие переменные для всех типов отчетов
	list.push_back(new rVariable(name + "type"               , TYPE_UINT , VARF_R___, &Type         , U_DIMLESS, 0));
	list.push_back(new rVariable(name + "archive.load.accept", TYPE_UINT , VARF_____, &ArchiveAccept, U_DIMLESS, ACCESS_REPORT));

	ArchiveTime.GenerateVars(name + "archive.load.", VARF_____, ACCESS_REPORT, list);

	if(REPORT_PERIODIC == Type)
	{
		list.push_back(new rVariable(name + "period", TYPE_UINT, VARF_R___, &Period, U_DIMLESS, 0));
	}
	else
	{
		list.push_back(new rVariable(name + "command", TYPE_USINT, VARF_____, &Command, U_DIMLESS, ACCESS_BATCH));
		list.push_back(new rVariable(name + "status" , TYPE_UINT , VARF_R___, &Status , U_DIMLESS, 0));
	}

	Present.GenerateVars  (name + "present."  , list);
	Completed.GenerateVars(name + "completed.", list);
	Archive.GenerateVars  (name + "archive."  , list);

	return 0;
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
UDINT rReport::LoadFromXML(tinyxml2::XMLElement *element, rDataConfig &cfg)
{
	string defType  = rDataConfig::GetFlagNameByValue(rDataConfig::ReportTypeFlags , REPORT_PERIODIC);
	string strType  = XmlUtils::getAttributeString(element, XmlName::TYPE, defType);
	UDINT  err      = 0;

	if(tinyxml2::XML_SUCCESS != rSource::LoadFromXML(element, cfg)) return DATACFGERR_REPORT;

	tinyxml2::XMLElement *period  = element->FirstChildElement(XmlName::PERIOD);
	tinyxml2::XMLElement *storage = element->FirstChildElement(XmlName::STORAGE);
	tinyxml2::XMLElement *dsname  = element->FirstChildElement(XmlName::DATASET);

	// Тип отчета
	Type = rDataConfig::GetFlagFromStr(rDataConfig::ReportTypeFlags, strType, err);

	if((Type == REPORT_PERIODIC && nullptr == period) || nullptr == dsname || err)
	{
		return DATACFGERR_REPORT;
	}

	// Время хранения отчета
	Storage = rDataConfig::GetTextUINT(storage, REPORT_DEFAULT_STORAGE, err);
	if(Storage == 0 || err)
	{
		return DATACFGERR_REPORT;
	}

	// Загрузка периодических отчетов
	if(REPORT_PERIODIC == Type)
	{
		string strPeriod = rDataConfig::GetTextString(element->FirstChildElement(XmlName::PERIOD), "", err);

		Period = rDataConfig::GetFlagFromStr(rDataConfig::ReportPeriodFlags, strPeriod, err);

		if(err) return DATACFGERR_REPORT;
	}

	//--------------------------------------------
	// Загружаем DataSet
	tinyxml2::XMLElement *dataset = GetDataSetElement(element, dsname->GetText());

	if(nullptr == dataset) return DATACFGERR_REPORT;

	// Перебираем станции и линии в dataset
	// Заполняем только объект Present
	for(tinyxml2::XMLElement *xml_total = dataset->FirstChildElement(XmlName::TOTAL); xml_total != nullptr; xml_total = xml_total->NextSiblingElement(XmlName::TOTAL))
	{
		rReportTotal *tot = new rReportTotal();

		Present.AverageItems.push_back(tot);

		tot->Source = nullptr;
		tot->Name   = rDataConfig::GetAttributeString(xml_total, XmlName::NAME , "");
		tot->Alias  = rDataConfig::GetAttributeString(xml_total, XmlName::ALIAS, "");

		if(tot->Name.empty() || tot->Alias.empty()) return DATACFGERR_REPORT;

		for(tinyxml2::XMLElement *xml_item = xml_total->FirstChildElement(XmlName::ITEM); xml_item != nullptr; xml_item = xml_item->NextSiblingElement(XmlName::ITEM))
		{
			rReportItem *item = new rReportItem();

			tot->Items.push_back(item);

			item->Name = rDataConfig::GetAttributeString(xml_item, XmlName::NAME, "");

			if(item->Name.empty()) return DATACFGERR_REPORT;

			if(tinyxml2::XML_SUCCESS != cfg.LoadLink(xml_item->FirstChildElement(XmlName::LINK), item->Source)) return cfg.ErrorID;
		}
	}

	// Загружаем мгновенные данные из dataset
	// Заполняем только объект Present
	tinyxml2::XMLElement *xml_snapshots = dataset->FirstChildElement(XmlName::SNAPSHOTS);
	if(xml_snapshots != nullptr)
	{
		for(tinyxml2::XMLElement *xml_item = xml_snapshots->FirstChildElement(XmlName::ITEM); xml_item != nullptr; xml_item = xml_item->NextSiblingElement(XmlName::ITEM))
		{
			rReportItem *item = new rReportItem();

			Present.SnapshotItems.push_back(item);

			item->Name = rDataConfig::GetAttributeString(xml_item, XmlName::NAME, "");

			if(item->Name.empty()) return DATACFGERR_REPORT;

			if(tinyxml2::XML_SUCCESS != cfg.LoadLink(xml_item->FirstChildElement(XmlName::LINK), item->Source)) return cfg.ErrorID;
		}
	}

	Completed.CreateFrom(Present);
	Archive.CreateFrom(Present);

	ReinitLimitEvents();

	return tinyxml2::XML_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
UDINT rReport::SaveKernel(FILE *file, UDINT isio, const string &objname, const string &comment, UDINT isglobal)
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

	return rSource::SaveKernel(file, isio, objname, comment, isglobal);
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

	rEventManager::instance().Add(ReinitEvent(EID_REPORT_GENERATED));

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
	printer.PushText(Source.GetSourceUnit());
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


	string filename = String_format("%s%s/%lu.xml", DIR_REPORT.c_str(), Alias.c_str(), reptime._UNIX);
	UDINT   result  = SimpleFileSave(filename, printer.CStr());

	if(result != TRITONN_RESULT_OK)
	{
		rEventManager::instance().Add(ReinitEvent(EID_REPORT_GENERATED) << result);
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



