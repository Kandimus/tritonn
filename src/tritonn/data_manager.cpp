﻿//=================================================================================================
//===
//=== data_manager.cpp
//===
//=== Copyright (c) 2019-2021 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Основной класс-нить для обработки данных
//===
//=================================================================================================


#include "data_manager.h"
#include <string.h>
#include "tritonn_version.h"
#include "tickcount.h"
#include "locker.h"
#include "log_manager.h"
#include "simplefile.h"
#include "simpleargs.h"
#include "variable_item.h"
#include "threadmaster.h"
#include "error.h"
#include "data_config.h"
#include "data_snapshot_item.h"
#include "data_snapshot.h"
#include "data_report.h"
#include "interface/interface.h"
#include "text_manager.h"
#include "event/manager.h"
#include "io/manager.h"
#include "listconf.h"
#include "def_arguments.h"

extern rSafityValue<DINT> gReboot;


rDataManager::rDataManager() : rVariableClass(Mutex), m_live(Live::UNDEF)
{
	RTTI = "rDataManager";

	m_sysVar.m_version.m_major = TRITONN_VERSION_MAJOR;
	m_sysVar.m_version.m_minor = TRITONN_VERSION_MINOR;
	m_sysVar.m_version.m_build = TRITONN_VERSION_BUILD;
	m_sysVar.m_version.m_hash  = TRITONN_VERSION_HASH;

	m_sysVar.m_metrologyVer.m_major = 1;
	m_sysVar.m_metrologyVer.m_minor = 0;
	m_sysVar.m_metrologyVer.m_crc   = 0x11223344;

	m_live.Set(Live::RUNNING);
	Halt.Set(false);
}

rDataManager::~rDataManager()
{
}

//-------------------------------------------------------------------------------------------------
//
void rDataManager::setLiveStatus(Live status)
{
	if(m_live.Get() != Live::HALT) {
		TRACEW(LOG::DATAMGR, "Set Live Status is %u (%s)",
			   status, m_sysVar.m_flagsLive.getNameByValue(static_cast<UDINT>(status)).c_str());
		m_live.Set(status);
	}
}


Live rDataManager::getLiveStatus()
{
	return m_live.Get();
}


void rDataManager::DoHalt(UDINT reason)
{
	if(Halt.Get()) return;

	Halt.Set(true);
	m_live.Set(Live::HALT);

	rEventManager::instance().addEventUDINT(EID_SYSTEM_HALT, reason);

	SimpleFileSave(FILE_RESTART, "cold");
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataManager::Restart(USINT restart, const string &filename)
{
	Live status = getLiveStatus();

	switch(restart)
	{
		case RESTART_WARM:
			TRACEI(LOG::DATAMGR, "Command Warm-restart");
			rEventManager::instance().addEvent(EID_SYSTEM_RESTART_WARM);
			break;

		case RESTART_COLD:
			TRACEI(LOG::DATAMGR, "Command Cold-restart");
			SimpleFileSave(FILE_RESTART, "cold");
			rEventManager::instance().addEvent(EID_SYSTEM_RESTART_COLD);
			break;

		case RESTART_DEBUG:
			TRACEI(LOG::DATAMGR, "Command Debug-restart");
			SimpleFileSave(FILE_RESTART, "debug");
			break;

		default:
			TRACEI(LOG::DATAMGR, "Unkonow command restart");
			rEventManager::instance().addEventUDINT(EID_SYSTEM_RESTART_UNKNOW, restart);
			return 1;
	}

	//
	switch(status)
	{
		case Live::STARTING:
		case Live::RUNNING:
		case Live::HALT:
		case Live::DUMP_TOTALS:
		case Live::DUMP_VARS:
			rThreadMaster::instance().Finish();
			return 0;

		case Live::REBOOT_COLD:
			if(filename.size())
			{
				TRACEW(LOG::DATAMGR, "Set new conf file: '%s'", filename.c_str());
				SimpleFileSave(FILE_CONF, filename);
			}
			rThreadMaster::instance().Finish();
			return 0;

		default:
			TRACEA(LOG::DATAMGR, "Unknow live status");
			rThreadMaster::instance().Finish();
			return 2;
	}
}



// Защиты по мьютексу нет, так как это статические данные и не меняются
void rDataManager::GetVersion(rVersion &ver) const
{
	ver = m_sysVar.m_version;
}

void rDataManager::GetState(rState &state)
{
	rLocker locker(Mutex); locker.Nop();

	state = m_sysVar.m_state;
}

void rDataManager::GetTime(struct tm &sdt)
{
	rLocker locker(Mutex); locker.Nop();

	sdt = m_sysVar.DateTime;
}

void rDataManager::GetConfigInfo(rConfigInfo &conf) const
{
	conf = m_sysVar.ConfigInfo;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Работа с файлом конфигурации
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//
UDINT rDataManager::LoadConfig()
{
	UDINT  result = TRITONN_RESULT_OK;
	string text   = "";
	string conf   = "";

	strcpy(m_sysVar.Lang, LANG_RU.c_str());

	// Устанавливаем флаг, что загружаемся
	setLiveStatus(Live::STARTING);

	//TODO Нужно очистить директорию ftp

	result = getConfFile(conf);
	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	// Если это не cold-start, то загружаем конфигурацию
	if (Live::STARTING == getLiveStatus()) {

		//TODO проверить на валидность hash
		TRACEI(LOG::DATAMGR, "Load config file '%s'", conf.c_str());

		//TODO тут нужно заменить на вызов XMLFileCheck()
		if (rDataConfig::instance().LoadFile(conf, m_sysVar, m_listSource, ListInterface, ListReport) != TRITONN_RESULT_OK) {
			return CreateHaltEvent(rDataConfig::instance().m_error);
		}
	}

	//--------------------------------------------
	// Создаем переменные
	m_sysVar.initVariables(m_varList);

	// Добавляем интерфейсы, создаем под них переменные
	for (auto interface : ListInterface) {
		rThreadMaster::instance().add(interface->getThreadClass(), TMF_DELETE | TMF_NOTRUN, interface->m_alias);
	}

	// Собираем переменные от объектов
	for (auto source : m_listSource) {
		result = source->generateVars(m_varList);
		if (result != TRITONN_RESULT_OK) {
			//TODO Нужно ли выдавать ошибку?
			return result;
		}
	}

	// Собираем переменные от интерфейсов
	for (auto interface : ListInterface) {
		result = interface->generateVars(getVariableClass());
		if (result != TRITONN_RESULT_OK) {
			//TODO Нужно ли выдавать ошибку?
			return 1;
		}
	}

	rIOManager::instance().generateVars(getVariableClass());

	m_varList.sort();

	//--------------------------------------------
	//NOTE только в процессе разработки
	if (getLiveStatus() == Live::STARTING) {
		m_varList.saveToCSV(DIR_FTP + conf); // Сохраняем их на ftp-сервер
		saveMarkDown();
		m_hashCfg = "00112233445566778899aabbccddeeff00112233";
	}

	//--------------------------------------------
	//TODO Нужно из rTextManager и rEventManager извлеч список языков, и сформировать единый список внутри rDataManager
	//TODO Сохранить массивы строк для WEB

	//
	SetLang(m_sysVar.Lang);

	if (!rSimpleArgs::instance().isSet(rArg::NoDump)) {
		loadDumps();
	}

	return result;
}


const rConfigInfo *rDataManager::GetConfName() const
{
	return &m_sysVar.ConfigInfo;
}


UDINT rDataManager::SetLang(const string &lang)
{
	rEventManager::instance().setCurLang(lang);
	rTextManager::instance().setCurLang(lang);

	strncpy(m_sysVar.Lang, lang.c_str(), 8);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
rThreadStatus rDataManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	m_timerTotal.start(1000);

	while(true)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

		//Lock();
		rLocker lock(rVariableClass::m_mutex); lock.Nop();

		m_sysVar.m_state.EventAlarm = rEventManager::instance().getAlarm();
		m_sysVar.m_state.Live       = m_live.Get();

		getCurrentTime(m_sysVar.UnixTime, &m_sysVar.DateTime);

		// set current time and date
		if (m_sysVar.SetDateTimeAccept) {
			setCurrentTime(m_sysVar.SetDateTime);
			//TODO Кидать мессагу о изменении времени

			m_sysVar.SetDateTimeAccept   = 0;
			m_sysVar.SetDateTime.tm_sec  = 0;
			m_sysVar.SetDateTime.tm_min  = 0;
			m_sysVar.SetDateTime.tm_hour = 0;
			m_sysVar.SetDateTime.tm_mday = 0;
			m_sysVar.SetDateTime.tm_mon  = 0;
			m_sysVar.SetDateTime.tm_year = 0;
		}

		if (m_sysVar.m_state.Live == Live::RUNNING)
		{
			// Пердвычисления для всех объектов
			for (auto item : m_listSource) {
				item->preCalculate();
			}

			// Основной расчет всех объектов
			for (auto item : m_listSource) {
				item->calculate();
			}

			// Пердвычисления для отчетов
			for (auto item : ListReport) {
				item->preCalculate();
			}

			// Основной расчет отчетов
			for (auto item : ListReport) {
				item->calculate();
			}

			if (m_doSaveVars.Get()) {
				saveDataVariables();
			}

			if (m_timerTotal.isFinished()) {
				saveDataTotals();
				m_timerTotal.restart();
			}
		}

//		Unlock();

		rVariableClass::processing();
		rThreadClass::EndProccesing();

	} // while
}



UDINT rDataManager::CreateHaltEvent(rError& err)
{
	rEvent event(EID_SYSTEM_CFGERROR);

	rEventManager::instance().add(event << (HALT_REASON_CONFIGFILE | err.getError()) << err.getLineno());

	DoHalt(HALT_REASON_CONFIGFILE | err.getError());

	TRACEP(LOG::DATAMGR, "Can't load conf file '%s'. Error ID: %i. Line %i. Error string '%s'.",
			   rDataConfig::instance().FileName.c_str(), err.getError(), err.getLineno(), err.getText().c_str());

	return err.getError();
}


UDINT rDataManager::StartInterfaces()
{
	rError err;

	m_sysVar.m_state.m_isSimulate = rSimpleArgs::instance().isSet(rArg::Simulate);

	rThreadMaster::instance().generateVars(getVariableClass());

	// Проверяем переменные в интерфейсах
	for (auto interface : ListInterface) {
		if (interface->checkVars(err) != TRITONN_RESULT_OK) {
			return CreateHaltEvent(err);
		}
	}

	for (auto interface : ListInterface) {
		interface->startServer();
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataManager::getConfFile(std::string& conf)
{
	std::string text;
	UDINT result = TRITONN_RESULT_OK;

	if (!rSimpleArgs::instance().isSet(rArg::ForceRun)) {

		// Проверяем на cold/warm/debug старт
		result = SimpleFileLoad(FILE_RESTART, text);
		if (TRITONN_RESULT_OK == result) {
			if ("cold" == text) {
				setLiveStatus(Live::REBOOT_COLD);

				// Загружаем список конфигураций
				rListConfig::Load();

				TRACEI(LOG::DATAMGR, "Cold restart!");
				//TODO Нужно выложить в web все языковые файлы
			}
			if ("debug" == text) {
				//TODO Доделать
				setLiveStatus(Live::REBOOT_COLD);

				// Загружаем список конфигураций
				rListConfig::Load();
			}
		}
	} else {
		TRACEI(LOG::DATAMGR, "Forced run!");
	}

	// удаляем файл
	SimpleFileDelete(FILE_RESTART);

	if (rSimpleArgs::instance().isSet(rArg::Config)) {
		conf = rSimpleArgs::instance().getOption(rArg::Config);
		return TRITONN_RESULT_OK;
	}

	result = SimpleFileLoad(FILE_CONF, conf);
	if(TRITONN_RESULT_OK != result) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_CFGERROR, HALT_REASON_CONFIGFILE | result);

		DoHalt(HALT_REASON_CONFIGFILE | result);

		TRACEP(LOG::DATAMGR, "Can't load file '%s'. Error ID: %i", FILE_CONF.c_str(), result);

		return result;
	}

	return TRITONN_RESULT_OK;
}

void rDataManager::doSaveVars()
{
	m_doSaveVars.Set(1);
}
