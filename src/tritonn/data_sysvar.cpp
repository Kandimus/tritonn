﻿//=================================================================================================
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
#include "bits_array.h"
#include "comment_defines.h"
#include "generator_md.h"

//-------------------------------------------------------------------------------------------------
//
UDINT rSystemVariable::initVariables(rVariableList& list)
{
	rBitsArray flagsLive;

	flagsLive
			.add("", static_cast<USINT>(Live::UNDEF)      , "Не определенный статус")
			.add("", static_cast<USINT>(Live::STARTING)   , "Система загружается")
			.add("", static_cast<USINT>(Live::REBOOT_COLD), "Система в режиме COLD-START")
			.add("", static_cast<USINT>(Live::RUNNING)    , "Система работает")
			.add("", static_cast<USINT>(Live::HALT)       , "Система в режиме HALT");

	list.add("system.version.major"      , TYPE_USINT, rVariable::Flags::RS_, &m_version.m_major     , U_DIMLESS, ACCESS_SA, "Версия ПО");
	list.add("system.version.minor"      , TYPE_USINT, rVariable::Flags::RS_, &m_version.m_minor     , U_DIMLESS, ACCESS_SA, "Подверсия ПО");
	list.add("system.version.build"      , TYPE_UINT , rVariable::Flags::RS_, &m_version.m_build     , U_DIMLESS, ACCESS_SA, "Номер сборки");
	list.add("system.version.hash"       , TYPE_UDINT, rVariable::Flags::RS_, &m_version.m_hash      , U_DIMLESS, ACCESS_SA, "Контрольная сумма сборки");

	list.add("system.metrology.major"    , TYPE_USINT, rVariable::Flags::RS_, &m_metrologyVer.m_major, U_DIMLESS, ACCESS_SA, "Версия метрологически значимой части ПО");
	list.add("system.metrology.minor"    , TYPE_USINT, rVariable::Flags::RS_, &m_metrologyVer.m_minor, U_DIMLESS, ACCESS_SA, "Подверсия метрологически значимой части ПО");
	list.add("system.metrology.crc"      , TYPE_UDINT, rVariable::Flags::RS_, &m_metrologyVer.m_crc  , U_DIMLESS, ACCESS_SA, "Контрольная сумма метрологически значимой части ПО");

	list.add("system.state.alarm"        , TYPE_UDINT, rVariable::Flags::RS_, &m_state.EventAlarm    , U_DIMLESS, ACCESS_SA, "Количество не квитированных аварий");
	list.add("system.state.live"         , TYPE_USINT, rVariable::Flags::RS_, &m_state.Live          , U_DIMLESS, ACCESS_SA, COMMENT::STATUS + flagsLive.getInfo(true));
	list.add("system.state.rebootreason" , TYPE_USINT, rVariable::Flags::RS_, &m_state.StartReason   , U_DIMLESS, ACCESS_SA, "Причина перезагрузки");
	list.add("system.state.simulate"     , TYPE_USINT, rVariable::Flags::R__, &m_state.m_isSimulate  , U_DIMLESS, 0        , "Флаг симуляции системы");

	list.add("system.datetime.sec"       , TYPE_INT  , rVariable::Flags::R__, &DateTime.tm_sec       , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::SECONDS);
	list.add("system.datetime.min"       , TYPE_INT  , rVariable::Flags::R__, &DateTime.tm_min       , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::MINUTES);
	list.add("system.datetime.hour"      , TYPE_INT  , rVariable::Flags::R__, &DateTime.tm_hour      , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::HOURS);
	list.add("system.datetime.day"       , TYPE_INT  , rVariable::Flags::R__, &DateTime.tm_mday      , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::DAY);
	list.add("system.datetime.month"     , TYPE_INT  , rVariable::Flags::R__, &DateTime.tm_mon       , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::MONTH);
	list.add("system.datetime.year"      , TYPE_INT  , rVariable::Flags::R__, &DateTime.tm_year      , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::YEAR);

	list.add("system.datetime.set.sec"   , TYPE_INT  , rVariable::Flags::___, &SetDateTime.tm_sec    , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::SECONDS);
	list.add("system.datetime.set.min"   , TYPE_INT  , rVariable::Flags::___, &SetDateTime.tm_min    , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::MINUTES);
	list.add("system.datetime.set.hour"  , TYPE_INT  , rVariable::Flags::___, &SetDateTime.tm_hour   , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::HOURS);
	list.add("system.datetime.set.day"   , TYPE_INT  , rVariable::Flags::___, &SetDateTime.tm_mday   , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::DAY);
	list.add("system.datetime.set.month" , TYPE_INT  , rVariable::Flags::___, &SetDateTime.tm_mon    , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::MONTH);
	list.add("system.datetime.set.year"  , TYPE_INT  , rVariable::Flags::___, &SetDateTime.tm_year   , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::YEAR);
	list.add("system.datetime.set.accept", TYPE_INT  , rVariable::Flags::___, &SetDateTimeAccept     , U_DIMLESS, ACCESS_SYSTEM, "Команда установить время:<br/>0 - нет действия<br/>1 - применить установленное время");

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
