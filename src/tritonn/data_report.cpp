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

#include "data_report.h"
#include <vector>
#include <limits>
#include <cmath>
#include <string.h>
#include "event/eid.h"
#include "event/manager.h"
#include "text_id.h"
#include "data_manager.h"
#include "data_config.h"
#include "error.h"
#include "variable_list.h"
#include "simplefile.h"
#include "xmlfile.h"
#include "xml_util.h"
#include "comment_defines.h"
#include "generator_md.h"

rBitsArray rReport::m_flagsType;
rBitsArray rReport::m_flagsPeriod;
rBitsArray rReport::m_flagsStatus;
rBitsArray rReport::m_flagsMark;
rBitsArray rReport::m_flagsCommand;

void rReportTime::setCurTime()
{
	getCurrentTime(_UNIX, &_TM);
}


//
void rReportTime::generateVars(const string &prefix, rVariable::Flags flags, UDINT access, rVariableList& list)
{
	list.add(prefix + "sec"   , TYPE::USINT, flags, &_TM.tm_sec , U_DIMLESS, access, COMMENT::SECONDS);
	list.add(prefix + "min"   , TYPE::USINT, flags, &_TM.tm_min , U_DIMLESS, access, COMMENT::MINUTES);
	list.add(prefix + "hour"  , TYPE::USINT, flags, &_TM.tm_hour, U_DIMLESS, access, COMMENT::HOURS);
	list.add(prefix + "day"   , TYPE::USINT, flags, &_TM.tm_mday, U_DIMLESS, access, COMMENT::DAY);
	list.add(prefix + "month" , TYPE::USINT, flags, &_TM.tm_mon , U_DIMLESS, access, COMMENT::MONTH);
	list.add(prefix + "year"  , TYPE::UINT , flags, &_TM.tm_year, U_DIMLESS, access, COMMENT::YEAR);
}


rReport::rItem::rItem(rReport::rItem& src)
{
	m_name = src.m_name;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

rReport::rTotal::rTotal(rReport::rTotal& src)
{
	m_source     = nullptr;
	m_alias      = "";
	m_name       = src.m_name;
	m_unitMass   = src.m_unitMass;
	m_unitVolume = src.m_unitVolume;

	for (auto item : src.m_items) {
		m_items.push_back(new rReport::rItem(*item));
	}
}

rReport::rTotal::~rTotal()
{
	m_source = nullptr;
	for (auto item : m_items) {
		if(item) delete item;
	}
	m_items.clear();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
rReport::rDataset::rDataset()
{
	m_mark = rReport::Mark::UNDEF;
}


rReport::rDataset::~rDataset()
{
	clear();
}


void rReport::rDataset::clear()
{
	for (auto item : m_averageItems) {
		if(item) {
			delete item;
		}
	}

	for (auto item : m_snapshotItems) {
		if(item) {
			delete item;
		}
	}

	m_averageItems.clear();
	m_snapshotItems.clear();
}


void rReport::rDataset::createFrom(rReport::rDataset& ds)
{
	clear();

	for (auto item : ds.m_averageItems) {
		m_averageItems.push_back(new rReport::rTotal(*item));
	}

	for (auto item : ds.m_snapshotItems) {
		m_snapshotItems.push_back(new rReport::rItem(*item));
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
rReport::rReport() : rSource()
{
	if (m_flagsType.empty()) {
		m_flagsType
				.add("PERIODIC", static_cast<UINT>(Type::PERIODIC), "Переодический отчет")
				.add("BATCH"   , static_cast<UINT>(Type::BATCH)   , "Партионный отчет");
	}

	if (m_flagsPeriod.empty()) {
		m_flagsPeriod
				.add("HOUR"     , static_cast<UINT>(Period::HOUR)     , "Часовой отчет")
				.add("2HOUR"    , static_cast<UINT>(Period::HOUR_2)   , "Двухчасовой отчет")
				.add("3HOUR"    , static_cast<UINT>(Period::HOUR_3)   , "Трехчасовой отчет")
				.add("4HOUR"    , static_cast<UINT>(Period::HOUR_4)   , "Четырехчасовой отчет")
				.add("6HOUR"    , static_cast<UINT>(Period::HOUR_6)   , "Шестичасовой отчет")
				.add("8HOUR"    , static_cast<UINT>(Period::HOUR_8)   , "Восьмичасовой отчет")
				.add("12HOUR"   , static_cast<UINT>(Period::HOUR_12)  , "Двенадцатичасовой отчет")
				.add("DAYLY"    , static_cast<UINT>(Period::DAYLY)    , "Суточный отчет")
				.add("WEEKLY"   , static_cast<UINT>(Period::WEEKLY)   , "Недельный отчет")
				.add("BIWEEKLY" , static_cast<UINT>(Period::BIWEEKLY) , "Двухнедельный отчет")
				.add("MONTHLY"  , static_cast<UINT>(Period::MONTHLY)  , "Месячный отчет")
				.add("QUARTERLY", static_cast<UINT>(Period::QUARTERLY), "Квартальный отчет")
				.add("ANNUAL"   , static_cast<UINT>(Period::ANNUAL)   , "Годовой отчет")
				.add("5MIN"     , static_cast<UINT>(Period::MIN_5)    , "Пятиминутный отчет (только для тестирования)")
				.add("15MIN"    , static_cast<UINT>(Period::MIN_15)   , "Пятнадцатиминутный отчет (только для тестирования)");
	}

	if (m_flagsStatus.empty()) {
		m_flagsStatus
				.add("", static_cast<UINT>(Status::IDLE)     , "Не запущен")
				.add("", static_cast<UINT>(Status::RUNNING)  , "Запущен")
				.add("", static_cast<UINT>(Status::WAITING)  , "В паузе")
				.add("", static_cast<UINT>(Status::COMPLETED), "Заверщен");
	}

	if (m_flagsMark.empty()) {
		m_flagsMark
				.add("", static_cast<UINT>(Mark::UNDEF)     , COMMENT::STATUS_UNDEF)
				.add("", static_cast<UINT>(Mark::ILLEGAL)   , "Недействительный")
				.add("", static_cast<UINT>(Mark::INCOMPLETE), "Неполный")
				.add("", static_cast<UINT>(Mark::VALIDATE)  , "Действительный")
				.add("", static_cast<UINT>(Mark::INPROGRESS), "В работе");
	}

	if (m_flagsCommand.empty()) {
		m_flagsCommand
				.add("", static_cast<UINT>(Command::NONE)   , COMMENT::COMMAND_NONE)
				.add("", static_cast<UINT>(Command::START)  , COMMENT::COMMAND_START)
				.add("", static_cast<UINT>(Command::STOP)   , COMMENT::COMMAND_STOP)
				.add("", static_cast<UINT>(Command::RESTART), "Перезапустить");
	}

	m_status = Status::IDLE;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::initLimitEvent(rLink& /*link*/)
{
	return TRITONN_RESULT_OK;
}

void rReport::run()
{
	start(true);
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::calculate()
{
	if (rSource::calculate()) {
		return 0;
	}

	m_curTime.setCurTime();

	// Обработка периодических отчетов
	if (m_type == Type::PERIODIC) {
		if (m_status == Status::RUNNING) {
			// Проверка на завершение отчета
			if (CheckFinishPeriodic()) {
				store();
				start();
			}

		// Проверка на незавершенный отчет при WarmStart
		} else if (m_status == Status::IDLE) {
			if (m_present.m_timeStart._UNIX) { //NOTE Для чего эта проверка?????

				// Если время отчета уже вышло, то сохраняем его с меткой "недействительный"
				if (m_curTime._UNIX - m_present.m_timeStart._UNIX >= GetUNIXPeriod()) {
					m_present.m_mark = rReport::Mark::ILLEGAL;
					store();
					start();
					m_present.m_mark = rReport::Mark::INCOMPLETE;

				} else {
					// Если время еще не вышло, то сохраняем с меткой "неполный"
					m_status         = Status::RUNNING;
					m_present.m_mark = rReport::Mark::INCOMPLETE;
				}
			} else {
				start(true);
			}
		}
	}
	// Обработка партионных отчетов
	else if (m_type == Type::BATCH) {
		;
	} else {
		return 1;
	}

	for(auto avr : m_present.m_averageItems) {
		LREAL oldmass = 0.0;
		LREAL curmass = 0.0;
		LREAL inc     = 0.0;

		// Прирост нарастающих за этот цикл
		inc     = ::rTotal::sub(avr->m_source->m_present.Mass, avr->m_finalTotal.Mass);
		// Общая масса на прошлом цикле (сколько прокачали до этого цикла)
		oldmass = ::rTotal::sub(avr->m_finalTotal.Mass     , avr->m_startTotal.Mass);
		// Общая масса на текущем цикле (сколько прокачали на этом цикле)
		curmass = ::rTotal::sub(avr->m_source->m_present.Mass, avr->m_startTotal.Mass);

		// Сохраняем нарастающие
		avr->m_finalTotal = avr->m_source->m_present;

		// Устредняем
		for(auto item : avr->m_items) {
			item->m_link.calculate();

//printf("Calculate %s '%s:%s' [%u]\n", item->m_name.c_str(), item->m_link.m_alias.c_str(), item->m_link.m_param.c_str(), item->m_link.m_unit.toUDINT());

			if(curmass <= 0.0) continue;

			item->m_value = ((item->m_value * inc) + (item->m_link.m_value * oldmass)) / curmass;
		}
	}

	for(auto item : m_present.m_snapshotItems) {
		item->m_link.calculate();
		item->m_value = item->m_link.m_value;

//printf("Calculate %s '%s:%s' [%u]\n", item->m_name.c_str(), item->m_link.m_alias.c_str(), item->m_link.m_param.c_str(), item->m_link.m_unit.toUDINT());
	}

	//----------------------------------------------------------------------------------------------
	// Обрабатываем Limits для выходных значений
	postCalculate();
		
	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//
void rReport::rDataset::generateVars(const string &prefix, rVariableList& list)
{
	string name = "";

	list.add(prefix + "status", TYPE::UINT, rVariable::Flags::RS__, &m_mark, U_DIMLESS, ACCESS_SA, COMMENT::STATUS + rReport::m_flagsMark.getInfo(true));

	m_timeStart.generateVars(prefix + "datetime.begin.", rVariable::Flags::RS__, ACCESS_SA, list);
	FinalTime.generateVars  (prefix + "datetime.end."  , rVariable::Flags::RS__, ACCESS_SA, list);

	// Формируем переменые
	for (auto tot :  m_averageItems) {
		name = prefix + tot->m_name + ".total.";

		list.add(name + "begin.mass"    , rVariable::Flags::RS__, &tot->m_startTotal.Mass    , tot->m_unitMass  , ACCESS_SA, COMMENT::BEGIN + COMMENT::MASS);
		list.add(name + "begin.volume"  , rVariable::Flags::RS__, &tot->m_startTotal.Volume  , tot->m_unitVolume, ACCESS_SA, COMMENT::BEGIN + COMMENT::VOLUME);
		list.add(name + "begin.volume15", rVariable::Flags::RS__, &tot->m_startTotal.Volume15, tot->m_unitVolume, ACCESS_SA, COMMENT::BEGIN + COMMENT::VOLUME15);
		list.add(name + "begin.volume20", rVariable::Flags::RS__, &tot->m_startTotal.Volume20, tot->m_unitVolume, ACCESS_SA, COMMENT::BEGIN + COMMENT::VOLUME20);
		list.add(name + "end.mass"      , rVariable::Flags::RS__, &tot->m_finalTotal.Mass    , tot->m_unitMass  , ACCESS_SA, COMMENT::END   + COMMENT::MASS);
		list.add(name + "end.volume"    , rVariable::Flags::RS__, &tot->m_finalTotal.Volume  , tot->m_unitVolume, ACCESS_SA, COMMENT::END   + COMMENT::VOLUME);
		list.add(name + "end.volume15"  , rVariable::Flags::RS__, &tot->m_finalTotal.Volume15, tot->m_unitVolume, ACCESS_SA, COMMENT::END   + COMMENT::VOLUME15);
		list.add(name + "end.volume20"  , rVariable::Flags::RS__, &tot->m_finalTotal.Volume20, tot->m_unitVolume, ACCESS_SA, COMMENT::END   + COMMENT::VOLUME20);

		name = prefix + tot->m_name + ".";

		for (auto item : tot->m_items) {
			list.add(name + item->m_name, rVariable::Flags::RS__, &item->m_value, item->m_link.getSourceUnit(), ACCESS_SA, "Значение устредняемого параметра");
		}
	}

	name = prefix + "snapshot.";

	for(auto item : m_snapshotItems) {
		list.add(name + item->m_name, rVariable::Flags::RS__, &item->m_value, item->m_link.getSourceUnit(), ACCESS_SA, "Значение не устредняемого параметра");
	}
}



//-------------------------------------------------------------------------------------------------
//
UDINT rReport::generateVars(rVariableList& list)
{
	string name = m_alias + ".";

	rSource::generateVars(list);

	// Общие переменные для всех типов отчетов
	list.add(name + "type"               , TYPE::UINT, rVariable::Flags::R___, &m_type       , U_DIMLESS, 0, "Тип отчета:<br/>" + m_flagsType.getInfo(true));
	list.add(name + "archive.load.accept",             rVariable::Flags::____, &ArchiveAccept, U_DIMLESS, ACCESS_REPORT, "Команда загрузки архивного отчета:<br/>0 - нет действия<br/>1 - загрузить отчет");

	ArchiveTime.generateVars(name + "archive.load.", rVariable::Flags::____, ACCESS_REPORT, list);

	if (m_type == Type::PERIODIC) {
		list.add(name + "period", TYPE::UINT, rVariable::Flags::____, &m_period, U_DIMLESS, 0, "Период отчета:<br/>" + m_flagsPeriod.getInfo(true));
	} else {
		list.add(name + "command",             rVariable::Flags::____, &m_command.Value, U_DIMLESS, ACCESS_BATCH, COMMENT::COMMAND + m_flagsCommand.getInfo(true));
		list.add(name + "status" , TYPE::UINT, rVariable::Flags::R___, &m_status       , U_DIMLESS, 0           , COMMENT::STATUS  + m_flagsStatus.getInfo(true));
	}

	m_present.generateVars  (name + "present."  , list);
	m_completed.generateVars(name + "completed.", list);
	m_archive.generateVars  (name + "archive."  , list);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
// Поиск требуемого dataset в дереве конфигурации
tinyxml2::XMLElement *rReport::GetDataSetElement(tinyxml2::XMLElement *element, const char *dsname)
{
	auto reports = element->Parent();

	if (nullptr == reports) {
		return nullptr;
	}

	auto repsys = reports->Parent();

	if (nullptr == repsys) {
		return nullptr;
	}

	auto datasets = repsys->FirstChildElement(XmlName::DATASETS);

	if (nullptr == datasets) {
		return nullptr;
	}

	for (tinyxml2::XMLElement *ds = datasets->FirstChildElement(XmlName::DATASET); ds != nullptr; ds = ds->NextSiblingElement(XmlName::DATASET))
	{
		if(!strcmp(ds->Attribute(XmlName::NAME), dsname)) return ds;
	}

	return nullptr;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rReport::loadFromXML(tinyxml2::XMLElement* element, rError& err, const std::string& prefix)
{
	std::string strType = XmlUtils::getAttributeString(element, XmlName::TYPE, m_flagsType.getNameByBits(static_cast<UINT>(Type::PERIODIC)));
	UDINT fault = 0;

	if (TRITONN_RESULT_OK != rSource::loadFromXML(element, err, prefix)) {
		return err.getError();
	}

	tinyxml2::XMLElement* period  = element->FirstChildElement(XmlName::PERIOD);
	tinyxml2::XMLElement* storage = element->FirstChildElement(XmlName::STORAGE);
	tinyxml2::XMLElement* dsname  = element->FirstChildElement(XmlName::DATASET);

	// Тип отчета
	m_type = static_cast<Type>(m_flagsType.getValue(strType, fault));

	if ((m_type == Type::PERIODIC && !period) || !dsname || fault) {
		return err.set(DATACFGERR_REPORT, element->GetLineNum());
	}

	// Время хранения отчета
	m_storage = XmlUtils::getTextUINT(storage, REPORT_DEFAULT_STORAGE, fault);
	if (m_storage == 0 || fault) {
		return err.set(DATACFGERR_REPORT, element->GetLineNum(), "invalid storage");
	}

	// Загрузка периодических отчетов
	if (m_type == Type::PERIODIC) {
		string strPeriod = XmlUtils::getTextString(element->FirstChildElement(XmlName::PERIOD), "", fault);

		m_period = static_cast<Period>(m_flagsPeriod.getBit(strPeriod, fault));

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
	XML_FOR(total_xml, dataset, XmlName::FWA) {
		auto tot = new rReport::rTotal();

		m_present.m_averageItems.push_back(tot);

		tot->m_source  = nullptr;
		tot->m_name    = XmlUtils::getAttributeString(total_xml, XmlName::NAME , "", XmlUtils::Flags::TOLOWER);
		tot->m_alias   = XmlUtils::getAttributeString(total_xml, XmlName::ALIAS, "", XmlUtils::Flags::TOLOWER);
		tot->m_lineNum = total_xml->GetLineNum();

		if (tot->m_name.empty() || tot->m_alias.empty()) {
			return err.set(DATACFGERR_REPORT, total_xml->GetLineNum(), "undefined name");
		}

		XML_FOR(item_xml, total_xml, XmlName::ITEM) {
			auto item = new rReport::rItem();

			tot->m_items.push_back(item);

			item->m_name = XmlUtils::getAttributeString(item_xml, XmlName::NAME, "");

			if (item->m_name.empty()) {
				return err.set(DATACFGERR_REPORT, item_xml->GetLineNum(), "undefined name");
			}

			if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(item_xml->FirstChildElement(XmlName::LINK), item->m_link)) {
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
			auto item = new rReport::rItem();

			m_present.m_snapshotItems.push_back(item);

			item->m_name = XmlUtils::getAttributeString(item_xml, XmlName::NAME, "");

			if (item->m_name.empty()) {
				return err.set(DATACFGERR_REPORT, item_xml->GetLineNum(), "undefined name");
			}

			if (TRITONN_RESULT_OK != rDataConfig::instance().LoadLink(item_xml->FirstChildElement(XmlName::LINK), item->m_link)) {
				return err.getError();
			}
		}
	}

	m_completed.createFrom(m_present);
	m_archive.createFrom(m_present);

	reinitLimitEvents();

	return TRITONN_RESULT_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
void rReport::store()
{
	m_present.FinalTime.setCurTime();

	m_completed.m_mark      = m_present.m_mark;
	m_completed.m_timeStart = m_present.m_timeStart;
	m_completed.FinalTime   = m_present.FinalTime;
	m_status                = Status::COMPLETED;
//	TimeStartUNIX       = 0;
//	TimeFinishUNIX      = 0;

	for (UDINT ii = 0; ii < m_present.m_averageItems.size(); ++ii) {
		auto p_total = m_present.m_averageItems[ii];
		auto c_total = m_completed.m_averageItems[ii];

		c_total->m_startTotal = p_total->m_startTotal;
		c_total->m_finalTotal = p_total->m_finalTotal;
		c_total->m_unitMass   = p_total->m_unitMass;
		c_total->m_unitVolume = p_total->m_unitVolume;

		::rTotal::clear(p_total->m_startTotal);
		::rTotal::clear(p_total->m_finalTotal);

		for (UDINT jj = 0; jj < p_total->m_items.size(); ++jj) {
			auto p_itm = p_total->m_items[jj];
			auto c_itm = c_total->m_items[jj];

			c_itm->m_value       = p_itm->m_value;
			c_itm->m_link.m_unit = p_itm->m_link.getSourceUnit();
			p_itm->m_value       = 0.0;
		}
	}

	for (UDINT ii = 0; ii < m_present.m_snapshotItems.size(); ++ii) {
		auto p_itm = m_present.m_snapshotItems[ii];
		auto c_itm = m_completed.m_snapshotItems[ii];

		c_itm->m_value       = p_itm->m_value;
		c_itm->m_link.m_unit = p_itm->m_link.getSourceUnit();
		p_itm->m_value       = 0.0;
	}

	rEventManager::instance().add(reinitEvent(EID_REPORT_GENERATED));

	// Формируем XML дерево отчета
	SaveToXML();
}


void rReport::start(bool isFirstStart)
{
	m_present.m_timeStart.setCurTime();
	m_present.m_mark = isFirstStart ? Mark::INCOMPLETE :  Mark::VALIDATE;
	m_status         = Status::RUNNING;

	for (auto item : m_present.m_averageItems) {
		item->m_startTotal = item->m_source->m_present;
		item->m_finalTotal = item->m_source->m_present;
	}
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
void rReportTime::Print(tinyxml2::XMLPrinter& printer, const char* name)
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


void rReport::rItem::print(tinyxml2::XMLPrinter& printer)
{
	printer.OpenElement(XmlName::VARIABLE);
	printer.PushAttribute(XmlName::NAME, m_name.c_str());

	printer.OpenElement(XmlName::VALUE);
	printer.PushText(m_value);
	printer.CloseElement();

	printer.OpenElement(XmlName::UNIT);
	printer.PushText(m_link.m_unit.toUDINT());
	printer.CloseElement();

	printer.CloseElement();
}

void rReport::rTotal::printTotals(tinyxml2::XMLPrinter& printer, const char* name, const rBaseTotal& total)
{
	printer.OpenElement(name);

	PrintElement(printer, "mass"    , total.Mass    , m_unitMass.toUDINT());
	PrintElement(printer, "volume"  , total.Volume  , m_unitVolume.toUDINT());
	PrintElement(printer, "volume15", total.Volume15, m_unitVolume.toUDINT());
	PrintElement(printer, "volume20", total.Volume20, m_unitVolume.toUDINT());

	printer.CloseElement();
}


void rReport::rTotal::print(tinyxml2::XMLPrinter& printer)
{
	printer.OpenElement("group"); printer.PushAttribute(XmlName::NAME, m_name.c_str());

	printer.OpenElement("totals");
	printTotals(printer, "start", m_startTotal);
	printTotals(printer, "final", m_finalTotal);
	printer.CloseElement(); // totals

	printer.OpenElement(XmlName::VARIABLES);
	for (auto item : m_items) {
		item->print(printer);
	}
	printer.CloseElement(); // vars

	printer.CloseElement(); // group
}


void rReport::rDataset::print(tinyxml2::XMLPrinter& printer)
{
	printer.OpenElement("mark");
	printer.PushText(static_cast<UINT>(m_mark));
	printer.CloseElement();

	printer.OpenElement("time");
	m_timeStart.Print(printer, "start");
	FinalTime.Print(printer, "final");
	printer.CloseElement();

	printer.OpenElement("data");

	// Формируем переменые для Dataset
	for (auto item : m_averageItems) {
		item->print(printer);
	}

	printer.OpenElement("snapshot");
	for (auto item : m_snapshotItems) {
		item->print(printer);
	}
	printer.CloseElement(); // snapshot
	printer.CloseElement(); // data
}


UDINT rReport::SaveToXML(UDINT present)
{
	tinyxml2::XMLPrinter printer;
	rReportTime          reptime;

	printer.OpenElement("tritonn");
	printer.PushAttribute("signature", XMLREPORT_HASH_SALT.c_str());

	printer.OpenElement("report");

	printer.OpenElement("type");
	printer.PushText(static_cast<UINT>(m_type));
	printer.CloseElement();

	if (m_type == Type::PERIODIC) {
		printer.OpenElement("period");
		printer.PushText(static_cast<UINT>(m_period));
		printer.CloseElement(); //TODO Нужно сюда писать описание а не число
	}

	if(present) {
		m_present.print(printer);
		reptime = m_present.m_timeStart;
	}
	else
	{
		m_completed.print(printer);
		reptime = m_completed.m_timeStart;
	}

	printer.CloseElement(); // report
	printer.CloseElement(); // tritonn


	string filename = String_format("%s%s/%lu.xml", DIR_REPORT.c_str(), m_alias.c_str(), reptime._UNIX);
	UDINT result = xmlFileSave(filename, printer.CStr(), "signature");

	if (result != TRITONN_RESULT_OK) {
		rEventManager::instance().add(reinitEvent(EID_REPORT_CANTSAVE) << result);
		return 0;
	}

	return 1;
}


UDINT rReport::GetUNIXPeriod()
{
	auto ts = &m_present.m_timeStart;

	switch(m_period)
	{
		case Period::HOUR     : return 3600;
		case Period::HOUR_2   : return 3600  * 2;
		case Period::HOUR_3   : return 3600  * 3;
		case Period::HOUR_4   : return 3600  * 4;
		case Period::HOUR_6   : return 3600  * 6;
		case Period::HOUR_8   : return 3600  * 8;
		case Period::HOUR_12  : return 3600  * 12;
		case Period::DAYLY    : return 3600  * 24;
		case Period::WEEKLY   : return 86400 * 7;
		case Period::BIWEEKLY : return 86400 * 14;
		case Period::MONTHLY  : return 86400 * DayInMonthShift(ts->_TM.tm_year, ts->_TM.tm_mon);
		case Period::QUARTERLY: return 86400 * DayInMonthShift(ts->_TM.tm_year, ts->_TM.tm_mon) +
									   86400 * DayInMonthShift(ts->_TM.tm_year, ts->_TM.tm_mon + 1) +
									   86400 * DayInMonthShift(ts->_TM.tm_year, ts->_TM.tm_mon + 2);
		case Period::ANNUAL   : return 86400 * (IsLeapYear    (ts->_TM.tm_year) + 365);
		case Period::MIN_5    : return 60 * 5;
		case Period::MIN_15   : return 60 * 15;
		default: return 0xFFFFFFFF;
	}
}


// Проверка на завершение переодического отчета
UDINT rReport::CheckFinishPeriodic()
{
	auto curtm = &m_curTime._TM;
	UDINT     result = 0;

	//TODO проверяем не чаще чем раз в минуту
	//if(curtime & 0x04 != 0) continue;

	switch (m_period)
	{
		case Period::HOUR     : result = (PastTM.tm_hour != curtm->tm_hour); break;
		case Period::HOUR_2   : result = (PastTM.tm_hour != curtm->tm_hour) && (curtm->tm_hour %  2 == 0); break;
		case Period::HOUR_3   : result = (PastTM.tm_hour != curtm->tm_hour) && (curtm->tm_hour %  3 == 0); break;
		case Period::HOUR_4   : result = (PastTM.tm_hour != curtm->tm_hour) && (curtm->tm_hour %  4 == 0); break;
		case Period::HOUR_6   : result = (PastTM.tm_hour != curtm->tm_hour) && (curtm->tm_hour %  6 == 0); break;
		case Period::HOUR_8   : result = (PastTM.tm_hour != curtm->tm_hour) && (curtm->tm_hour %  8 == 0); break;
		case Period::HOUR_12  : result = (PastTM.tm_hour != curtm->tm_hour) && (curtm->tm_hour % 12 == 0); break;
		case Period::DAYLY    : result = (PastTM.tm_mday != curtm->tm_mday); break;
		case Period::WEEKLY   : result = (PastTM.tm_wday != curtm->tm_wday) && (curtm->tm_wday      == 1); break;
		case Period::BIWEEKLY : result = (PastTM.tm_wday != curtm->tm_wday) && (curtm->tm_wday      == 1) && ((WeekNumber(*curtm) - 1) % 2 == 0); break;
		case Period::MONTHLY  : result = (PastTM.tm_mon  != curtm->tm_mon ); break;
		case Period::QUARTERLY: result = (PastTM.tm_mon  != curtm->tm_mon ) && ((curtm->tm_mon - 1) % 3 == 0); break;
		case Period::ANNUAL   : result = (PastTM.tm_year != curtm->tm_year); break;
		case Period::MIN_5    : result = (PastTM.tm_min  != curtm->tm_min ) && (curtm->tm_min  %  5 == 0); break;
		case Period::MIN_15   : result = (PastTM.tm_min  != curtm->tm_min ) && (curtm->tm_min  % 15 == 0); break;
	}

	PastTM = *curtm;

	return result;
}

UDINT rReport::generateMarkDown(rGeneratorMD& md)
{
	std::string name   = isPeriodic() ? "periodic report" : "batch report";
	std::string remark = "";

	m_present.m_averageItems.push_back(new rReport::rTotal());
	m_present.m_averageItems.back()->m_name = "#report_total_1";
	m_present.m_averageItems.back()->m_items.push_back(new rReport::rItem());
	m_present.m_averageItems.back()->m_items.back()->m_name = "#report_avr_item_1";
	m_present.m_averageItems.back()->m_items.push_back(new rReport::rItem());
	m_present.m_averageItems.back()->m_items.back()->m_name = "#report_avr_item_2";

	m_present.m_snapshotItems.push_back(new rReport::rItem());
	m_present.m_snapshotItems.back()->m_name = "#report_item_1";
	m_present.m_snapshotItems.push_back(new rReport::rItem());
	m_present.m_snapshotItems.back()->m_name = "#report_item_2";

	m_archive.createFrom(m_present);
	m_completed.createFrom(m_present);

	if (isPeriodic()) {
		remark += m_flagsPeriod.getMarkDown("Period");
	}

	remark += "## XML Dataset\n````xml\n";
	remark += "<" + std::string(XmlName::DATASETS) + ">\n";
	remark += "\t<" + std::string(XmlName::DATASET) + " name=\"valid name of dataset\">\n";
	remark += "\t\t<" + std::string(XmlName::FWA) + " name=\"valid name\" alias=\"alias of calculate object contain totals\">\n";
	remark += "\t\t\t<" + std::string(XmlName::ITEM) + " name=\"valid name\">\n";
	remark += "\t\t\t\t" + rGeneratorMD::rItem::XML_LINK + "\n";
	remark += "\t\t\t</" + std::string(XmlName::ITEM) + ">\n";
	remark += "\t\t\t...\n";
	remark += "\t\t\t<" + std::string(XmlName::ITEM) + " name=\"valid name\">\n";
	remark += "\t\t\t\t" + rGeneratorMD::rItem::XML_LINK + "\n";
	remark += "\t\t\t</" + std::string(XmlName::ITEM) + ">\n";
	remark += "\t\t</" + std::string(XmlName::FWA) + ">\n";
	remark += "\t\t...\n";
	remark += "\t\t<" + std::string(XmlName::FWA) + "> " + rGeneratorMD::rItem::XML_OPTIONAL + "\n";
	remark += "\t\t\t...\n";
	remark += "\t\t</" + std::string(XmlName::FWA) + ">\n";

	remark += "\t\t<" + std::string(XmlName::SNAPSHOTS) + "> " + rGeneratorMD::rItem::XML_OPTIONAL + "\n";
	remark += "\t\t\t<" + std::string(XmlName::ITEM) + " name=\"valid name\">\n";
	remark += "\t\t\t\t" + rGeneratorMD::rItem::XML_LINK + "\n";
	remark += "\t\t\t</" + std::string(XmlName::ITEM) + ">\n";
	remark += "\t\t\t...\n";
	remark += "\t\t\t<" + std::string(XmlName::ITEM) + " name=\"valid name\">\n";
	remark += "\t\t\t\t" + rGeneratorMD::rItem::XML_LINK + "\n";
	remark += "\t\t\t</" + std::string(XmlName::ITEM) + ">\n";
	remark += "\t\t</" + std::string(XmlName::SNAPSHOTS) + ">\n";
	remark += "\t</" + std::string(XmlName::DATASET) + ">\n";

	remark += "\t...\n";
	remark += "\t<" + std::string(XmlName::DATASET) + " name=\"valid report name\"> " + rGeneratorMD::rItem::XML_OPTIONAL + "\n";;
	remark += "\t\t...\n";
	remark += "\t</" + std::string(XmlName::DATASET) + ">\n";
	remark += "</" + std::string(XmlName::DATASETS) + ">\n";
	remark += "````\n";

	rGeneratorMD::rItem& item = md.add(this, false,	rGeneratorMD::Type::REPORT)
			.setFilename(isPeriodic() ? "periodic report" : "batch report")
			.addProperty(XmlName::TYPE, &m_flagsType, true)
			.addXml(XmlName::STORAGE, static_cast<UDINT>(m_storage))
			.addXml(XmlName::DATASET, std::string("dataset name"))
			.addRemark(remark);

	if (isPeriodic()) {
		item.addXml("<" + std::string(XmlName::PERIOD) + ">period value</" + std::string(XmlName::PERIOD) + ">");
	}

	return TRITONN_RESULT_OK;
}

