//=================================================================================================
//===
//=== data_sysvar.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс для системных переменных rDataManager
//===
//=================================================================================================

#include "units.h"
#include "data_sysvar.h"
#include "variable_item.h"
#include "variable_list.h"
#include "comment_defines.h"
#include "generator_md.h"


void rSystemVariable::initFlags()
{
	m_flagsLive
			.add("UNDEF"     , static_cast<USINT>(Live::UNDEF)      , "Не определенный статус")
			.add("STARTING"  , static_cast<USINT>(Live::STARTING)   , "Система загружается")
			.add("COLDSTART" , static_cast<USINT>(Live::REBOOT_COLD), "Система в режиме COLD-START")
			.add("DUMPTOTALS", static_cast<USINT>(Live::DUMP_TOTALS), "Система в режиме загрузки нарастающих")
			.add("DUMPVARS"  , static_cast<USINT>(Live::DUMP_VARS)  , "Система в режиме загрузки уставок")
			.add("RUNNING"   , static_cast<USINT>(Live::RUNNING)    , "Система работает")
			.add("HALT"      , static_cast<USINT>(Live::HALT)       , "Система в режиме HALT");

//	m_flagsType
//			.add("undef" , static_cast<USINT>(TYPE::UNDEF), "Тип не определен")
//			.add("int8"  , static_cast<USINT>(TYPE::SINT) , "Целый знаковый 8 битный тип")
//			.add("uint8" , static_cast<USINT>(TYPE::USINT), "Целый беззнаковый 8 битный тип")
//			.add("int16" , static_cast<USINT>(TYPE::INT)  , "Целый знаковый 16 битный тип")
//			.add("uint16", static_cast<USINT>(TYPE::UINT) , "Целый беззнаковый 16 битный тип")
//			.add("int32" , static_cast<USINT>(TYPE::UDINT), "Целый знаковый 32 битный тип")
//			.add("uint32", static_cast<USINT>(TYPE::DINT) , "Целый беззнаковый 16 битный тип")
//			.add("float" , static_cast<USINT>(TYPE::REAL) , "Вещественный 32 битных тип")
//			.add("double", static_cast<USINT>(TYPE::LREAL), "Вещественный 64 битных тип")
//			.add("strid" , static_cast<USINT>(TYPE::STRID), "Строковый тип");
}

//-------------------------------------------------------------------------------------------------
//
UDINT rSystemVariable::initVariables(rVariableList& list)
{
	list.add("system.version.major"      , TYPE::USINT, rVariable::Flags::RS__, &m_version.m_major     , U_DIMLESS, ACCESS_SA, "Версия ПО");
	list.add("system.version.minor"      , TYPE::USINT, rVariable::Flags::RS__, &m_version.m_minor     , U_DIMLESS, ACCESS_SA, "Подверсия ПО");
	list.add("system.version.build"      , TYPE::UINT , rVariable::Flags::RS__, &m_version.m_build     , U_DIMLESS, ACCESS_SA, "Номер сборки");
	list.add("system.version.hash"       , TYPE::UDINT, rVariable::Flags::RS__, &m_version.m_hash      , U_DIMLESS, ACCESS_SA, "Контрольная сумма сборки");

	list.add("system.metrology.major"    ,              rVariable::Flags::RS__, &m_metrologyVer.m_major, U_DIMLESS, ACCESS_SA, "Версия метрологически значимой части ПО");
	list.add("system.metrology.minor"    ,              rVariable::Flags::RS__, &m_metrologyVer.m_minor, U_DIMLESS, ACCESS_SA, "Подверсия метрологически значимой части ПО");
	list.add("system.metrology.crc"      ,              rVariable::Flags::RS__, &m_metrologyVer.m_crc  , U_DIMLESS, ACCESS_SA, "Контрольная сумма метрологически значимой части ПО");

	list.add("system.state.alarm"        ,              rVariable::Flags::RS__, &m_state.EventAlarm    , U_DIMLESS, ACCESS_SA, "Количество не квитированных аварий");
	list.add("system.state.live"         ,              rVariable::Flags::RS__, &m_state.Live          , U_DIMLESS, ACCESS_SA, COMMENT::STATUS + m_flagsLive.getInfo(true));
	list.add("system.state.rebootreason" , TYPE::USINT, rVariable::Flags::RS__, &m_state.StartReason   , U_DIMLESS, ACCESS_SA, "Причина перезагрузки");
	list.add("system.state.simulate"     ,              rVariable::Flags::R___, &m_state.m_isSimulate  , U_DIMLESS, 0        , "Флаг симуляции системы");

	list.add("system.datetime.sec"       , TYPE::USINT, rVariable::Flags::R___, &DateTime.tm_sec       , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::SECONDS);
	list.add("system.datetime.min"       , TYPE::USINT, rVariable::Flags::R___, &DateTime.tm_min       , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::MINUTES);
	list.add("system.datetime.hour"      , TYPE::USINT, rVariable::Flags::R___, &DateTime.tm_hour      , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::HOURS);
	list.add("system.datetime.day"       , TYPE::USINT, rVariable::Flags::R___, &DateTime.tm_mday      , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::DAY);
	list.add("system.datetime.month"     , TYPE::USINT, rVariable::Flags::R___, &DateTime.tm_mon       , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::MONTH);
	list.add("system.datetime.year"      , TYPE::UINT , rVariable::Flags::R___, &DateTime.tm_year      , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::YEAR);

	list.add("system.datetime.set.sec"   , TYPE::USINT, rVariable::Flags::____, &SetDateTime.tm_sec    , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::SECONDS);
	list.add("system.datetime.set.min"   , TYPE::USINT, rVariable::Flags::____, &SetDateTime.tm_min    , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::MINUTES);
	list.add("system.datetime.set.hour"  , TYPE::USINT, rVariable::Flags::____, &SetDateTime.tm_hour   , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::HOURS);
	list.add("system.datetime.set.day"   , TYPE::USINT, rVariable::Flags::____, &SetDateTime.tm_mday   , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::DAY);
	list.add("system.datetime.set.month" , TYPE::USINT, rVariable::Flags::____, &SetDateTime.tm_mon    , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::MONTH);
	list.add("system.datetime.set.year"  , TYPE::UINT , rVariable::Flags::____, &SetDateTime.tm_year   , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::YEAR);
	list.add("system.datetime.set.accept",              rVariable::Flags::____, &SetDateTimeAccept     , U_DIMLESS, ACCESS_SYSTEM, "Команда установить время:<br/>0 - нет действия<br/>1 - применить установленное время");

	return TRITONN_RESULT_OK;
}


void rSystemVariable::generateMarkDown(rGeneratorMD& md)
{
	rVariableList list;
	std::string   result = "";

	initVariables(list);

	result += list.getMarkDown();

	md.add("sysvar").addRemark(result);
}
