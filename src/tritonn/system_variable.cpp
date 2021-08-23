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
#include "simplefile.h"
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

	m_configInfo.m_developer = "";
	m_configInfo.m_filename  = "";
	m_configInfo.m_hash      = "";
	m_configInfo.m_name      = "";
	m_configInfo.m_version   = "";
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
	list.add("system.state.rebootreason" , TYPE::USINT, rVariable::Flags::RS__, &m_state.m_startReason , U_DIMLESS, ACCESS_SA, "Причина перезагрузки");
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
	std::string text = "# tritonn config ip\nauto lo\niface lo inet loopback\n\n";

#ifdef TRITONN_YOCTO
	bool needreset = true;
#endif

	for (auto& eth : m_ethernet) {
		TRACEI(LOG::SYSVAR, "Set IP addresses on %s as %s", eth.m_dev.c_str(), eth.m_ip.c_str());

		text += "allow-hotplug " + eth.m_dev + "\n";
		text += "auto " + eth.m_dev + "\n";
		text += "iface " + eth.m_dev + " inet static\n";
		text += "\t  address " + eth.m_ip + "\n";
		text += "\t  netmask " + eth.m_mask + "\n";

		if (eth.m_gateway.size()) {
			text += "\t  gateway " + eth.m_gateway + "\n";
		}

		text += "\n";

#ifdef TRITONN_YOCTO
		std::string text_ip = "ip addr add " + eth.m_ip + "/" + eth.m_mask + " dev " + eth.m_dev;

		if (eth.m_gateway.size()) {
			text_ip += "; ip route add " + eth.m_ip + "/" + eth.m_mask + " via " + eth.m_gateway;
		}

		if (needreset) {
			rSystemManager::instance().force("ip a flush dev eth0 > /dev/null");
			rSystemManager::instance().force("ip a flush dev eth1 > /dev/null");
			needreset = false;
		}

		rSystemManager::instance().add(text_ip);
#endif
	}

#ifndef TRITONN_YOCTO
	TRACEW(LOG::SYSVAR, "Not YOCTO build. IP addresses do not apply");
	simpleFileSave("interfaces.txt", text);
#else
	TRACEW(LOG::SYSVAR, "IP addresses apply. Restart network");
	simpleFileSave("/etc/network/interfaces", text);

	//rSystemManager::instance().add("ifup -a");

	mSleep(300);
#endif
}


UDINT rSystemVariable::loadFromXml(const std::string& filename, tinyxml2::XMLElement* root, tinyxml2::XMLElement* xml_setting, rError& err)
{
	rLocker locker(m_rwlock, rLocker::TYPELOCK::WRITE); locker.Nop();

	m_configInfo.m_filename  = filename.c_str();
	m_configInfo.m_developer = XmlUtils::getAttributeString(root, XmlName::DEVELOPMENT, "");
	m_configInfo.m_name      = XmlUtils::getAttributeString(root, XmlName::NAME       , "");
	m_configInfo.m_version   = XmlUtils::getAttributeString(root, XmlName::VERSION    , "");
	m_configInfo.m_hash      = XmlUtils::getAttributeString(root, XmlName::HASH       , "");

	if (xml_setting) {
		XML_FOR(xml_ethernet, xml_setting, XmlName::ETHERNET) {
			rEthernet eth;
			UDINT     fault = 0;

			eth.m_dev = XmlUtils::getAttributeString(xml_ethernet, XmlName::DEVICE, "", XmlUtils::Flags::TOLOWER);
			if (eth.m_dev.empty()) {
				return err.set(DATACFGERR_EHTERNET_LOAD_FAULT, root->GetLineNum(), "device is null");
			}

			eth.m_ip = XmlUtils::getTextString(xml_ethernet->FirstChildElement(XmlName::IP), "", fault);
			if (fault || eth.m_ip.empty()) {
				return err.set(DATACFGERR_EHTERNET_LOAD_FAULT, root->GetLineNum(), "ip is null");
			}

			eth.m_mask = XmlUtils::getTextString(xml_ethernet->FirstChildElement(XmlName::MASK), "", fault);
			if (fault || eth.m_mask.empty()) {
				return err.set(DATACFGERR_EHTERNET_LOAD_FAULT, root->GetLineNum(), "mask is null");
			}

			eth.m_gateway = XmlUtils::getTextString(xml_ethernet->FirstChildElement(XmlName::GATEWAY), "", fault);

			m_ethernet.push_back(eth);
		}
	}

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
