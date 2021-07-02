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

#include <string.h>
#include "locker.h"
#include "units.h"
#include "tritonn_version.h"
#include "xml_util.h"
#include "error.h"
#include "datetime.h"
#include "log_manager.h"
#include "system_variable.h"
#include "variable_item.h"
#include "variable_list.h"
#include "system_manager.h"
#include "comment_defines.h"
#include "generator_md.h"


rSystemVariable::rSystemVariable()
{
	if (!m_flagsLive.empty()) {
		return;
	}

	m_flagsLive
			.add("UNDEF"     , static_cast<USINT>(Live::UNDEF)      , "Не определенный статус")
			.add("STARTING"  , static_cast<USINT>(Live::STARTING)   , "Система загружается")
			.add("COLDSTART" , static_cast<USINT>(Live::REBOOT_COLD), "Система в режиме COLD-START")
			.add("DUMPTOTALS", static_cast<USINT>(Live::DUMP_TOTALS), "Система в режиме загрузки нарастающих")
			.add("DUMPVARS"  , static_cast<USINT>(Live::DUMP_VARS)  , "Система в режиме загрузки уставок")
			.add("RUNNING"   , static_cast<USINT>(Live::RUNNING)    , "Система работает")
			.add("HALT"      , static_cast<USINT>(Live::HALT)       , "Система в режиме HALT");

	pthread_rwlock_init(&m_rwlock, nullptr);

	m_version.m_major = TRITONN_VERSION_MAJOR;
	m_version.m_minor = TRITONN_VERSION_MINOR;
	m_version.m_build = TRITONN_VERSION_BUILD;
	m_version.m_hash  = TRITONN_VERSION_HASH;

	m_metrologyVer.m_major = 1;
	m_metrologyVer.m_minor = 0;
	m_metrologyVer.m_crc   = 0x11223344;

	m_configInfo.Developer[0] = 0;
	m_configInfo.File[0]      = 0;
	m_configInfo.Hash[0]      = 0;
	m_configInfo.Name[0]      = 0;
	m_configInfo.Version[0]   = 0;
}

rSystemVariable::~rSystemVariable()
{
	pthread_rwlock_destroy(&m_rwlock);
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

	list.add("system.state.alarm"        ,              rVariable::Flags::RS__, &m_state.m_eventAlarm  , U_DIMLESS, ACCESS_SA, "Количество не квитированных аварий");
	list.add("system.state.live"         , TYPE::USINT, rVariable::Flags::RS__, &m_state.m_live        , U_DIMLESS, ACCESS_SA, COMMENT::STATUS + m_flagsLive.getInfo(true));
	list.add("system.state.rebootreason" , TYPE::USINT, rVariable::Flags::RS__, &m_state.StartReason   , U_DIMLESS, ACCESS_SA, "Причина перезагрузки");
	list.add("system.state.simulate"     ,              rVariable::Flags::R___, &m_state.m_isSimulate  , U_DIMLESS, 0        , "Флаг симуляции системы");

	list.add("system.datetime.sec"       , TYPE::USINT, rVariable::Flags::R___, &m_dateTime.tm_sec     , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::SECONDS);
	list.add("system.datetime.min"       , TYPE::USINT, rVariable::Flags::R___, &m_dateTime.tm_min     , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::MINUTES);
	list.add("system.datetime.hour"      , TYPE::USINT, rVariable::Flags::R___, &m_dateTime.tm_hour    , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::HOURS);
	list.add("system.datetime.day"       , TYPE::USINT, rVariable::Flags::R___, &m_dateTime.tm_mday    , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::DAY);
	list.add("system.datetime.month"     , TYPE::USINT, rVariable::Flags::R___, &m_dateTime.tm_mon     , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::MONTH);
	list.add("system.datetime.year"      , TYPE::UINT , rVariable::Flags::R___, &m_dateTime.tm_year    , U_DIMLESS, 0        , COMMENT::TIME_CURRENT + COMMENT::YEAR);

	list.add("system.datetime.set.sec"   , TYPE::USINT, rVariable::Flags::____, &m_setDateTime.tm_sec  , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::SECONDS);
	list.add("system.datetime.set.min"   , TYPE::USINT, rVariable::Flags::____, &m_setDateTime.tm_min  , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::MINUTES);
	list.add("system.datetime.set.hour"  , TYPE::USINT, rVariable::Flags::____, &m_setDateTime.tm_hour , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::HOURS);
	list.add("system.datetime.set.day"   , TYPE::USINT, rVariable::Flags::____, &m_setDateTime.tm_mday , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::DAY);
	list.add("system.datetime.set.month" , TYPE::USINT, rVariable::Flags::____, &m_setDateTime.tm_mon  , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::MONTH);
	list.add("system.datetime.set.year"  , TYPE::UINT , rVariable::Flags::____, &m_setDateTime.tm_year , U_DIMLESS, ACCESS_SYSTEM, COMMENT::TIME_SET + COMMENT::YEAR);
	list.add("system.datetime.set.accept",              rVariable::Flags::____, &m_setDateTimeAccept   , U_DIMLESS, ACCESS_SYSTEM, "Команда установить время:<br/>0 - нет действия<br/>1 - применить установленное время");

	return TRITONN_RESULT_OK;
}

void rSystemVariable::processing()
{
	rLocker locker(m_rwlock, rLocker::TYPELOCK::WRITE); locker.Nop();

	getCurrentTime(m_unixTime, &m_dateTime);

	// set current time and date
	if (m_setDateTimeAccept) {
		rDateTime curdt(m_dateTime);
		rDateTime newdt(m_setDateTime);

		TRACEW(LOG::SYSVAR, "Set new time %s (present time %s)", newdt.toString().c_str(), curdt.toString().c_str());

#ifdef TRITONN_YOCTO
		setCurrentTime(m_setDateTime);
#else
		TRACEW(LOG::SYSVAR, "Set time disable. Not YOCTO build");
#endif
		//TODO Кидать мессагу о изменении времени

		m_setDateTimeAccept   = 0;
		m_setDateTime.tm_sec  = 0;
		m_setDateTime.tm_min  = 0;
		m_setDateTime.tm_hour = 0;
		m_setDateTime.tm_mday = 0;
		m_setDateTime.tm_mon  = 0;
		m_setDateTime.tm_year = 0;
	}
}

void rSystemVariable::getConfigInfo(rConfigInfo &conf)
{
	rLocker locker(m_rwlock); locker.Nop();

	conf = m_configInfo;
}

void rSystemVariable::getState(rState &state)
{
	rLocker locker(m_rwlock); locker.Nop();

	state = m_state;
}

void rSystemVariable::getTime(struct tm &sdt)
{
	rLocker locker(m_rwlock); locker.Nop();

	sdt = m_dateTime;
}

void rSystemVariable::setEventAlarm(UDINT count)
{
	rLocker locker(m_rwlock); locker.Nop();

	m_state.m_eventAlarm = count;
}

void rSystemVariable::setLive(Live live)
{
	rLocker locker(m_rwlock); locker.Nop();

	m_state.m_live = live;
}

void rSystemVariable::setSimulate(USINT sim)
{
	rLocker locker(m_rwlock); locker.Nop();

	m_state.m_isSimulate = sim;
}

void rSystemVariable::applyEthernet()
{
	for (auto& eth : m_ethernet) {
		rSystemManager::add("")
	}
}


UDINT rSystemVariable::loadFromXml(const std::string& filename, tinyxml2::XMLElement* root)
{
	rLocker locker(m_rwlock, rLocker::TYPELOCK::WRITE); locker.Nop();

	strncpy(m_configInfo.File, filename.c_str(), MAX_CONFIG_NAME);

	return TRITONN_RESULT_OK;
}

UDINT rSystemVariable::loadEthernet(tinyxml2::XMLElement* root, rError& err)
{
	if (!root) {
		return TRITONN_RESULT_OK;
	}

	rEthernet eth;
	UDINT     fault = 0;

	eth.m_dev     = XmlUtils::getAttributeString(root, XmlName::DEVICE, "", XmlUtils::Flags::TOLOWER);
	eth.m_ip      = XmlUtils::getTextString(root->FirstChildElement(XmlName::IP)     , "", fault);
	eth.m_mask    = XmlUtils::getTextString(root->FirstChildElement(XmlName::MASK)   , "", fault);
	eth.m_gateway = XmlUtils::getTextString(root->FirstChildElement(XmlName::GATEWAY), "", fault);

	if (eth.m_dev.empty()) {
		return err.set(DATACFGERR_EHTERNET_LOAD_FAULT, root->GetLineNum(), "device is null");
	}

	if (fault) {
		return err.set(DATACFGERR_EHTERNET_LOAD_FAULT, root->GetLineNum(), "");
	}

	m_ethernet.push_back(eth);

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
