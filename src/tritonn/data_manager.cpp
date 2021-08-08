//=================================================================================================
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
#include "tickcount.h"
#include "locker.h"
#include "log_manager.h"
#include "simplefile.h"
#include "simpleargs.h"
#include "system_variable.h"
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


rDataManager::rDataManager() : rThreadClass(), rVariableClass(Mutex), m_live(Live::UNDEF)
{
	RTTI = "rDataManager";

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
			   status, rSystemVariable::instance().getLiveName(status).c_str());

		m_live.Set(status);
	}
}


Live rDataManager::getLiveStatus()
{
	return m_live.Get();
}


void rDataManager::DoHalt(HaltReason hr, UDINT reason)
{
	if(Halt.Get()) return;

	Halt.Set(true);
	m_live.Set(Live::HALT);

	rEventManager::instance().addEventUDINT(EID_SYSTEM_HALT, static_cast<UDINT>(hr) | reason);

	simpleFileSave(FILE_RESTART, "cold");
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
			simpleFileSave(FILE_RESTART, "cold");
			rEventManager::instance().addEvent(EID_SYSTEM_RESTART_COLD);
			break;

		case RESTART_DEBUG:
			TRACEI(LOG::DATAMGR, "Command Debug-restart");
			simpleFileSave(FILE_RESTART, "debug");
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
				simpleFileSave(FILE_CONF, filename);
			}
			rThreadMaster::instance().Finish();
			return 0;

		default:
			TRACEA(LOG::DATAMGR, "Unknow live status");
			rThreadMaster::instance().Finish();
			return 2;
	}
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

	setLang(LANG_RU);

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
		if (rDataConfig::instance().LoadFile(conf, m_listSource, ListInterface, m_listReport) != TRITONN_RESULT_OK) {
			return CreateHaltEvent(rDataConfig::instance().m_error);
		}
	}

	//--------------------------------------------
	// Создаем переменные
	rSystemVariable::instance().initVariables(m_varList);

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

	if (getLiveStatus() != Live::STARTING) {
		return result;
	}

	// Все порядке, запускаемся дальше

	//NOTE Блок только во время разработки
	m_varList.saveToCSV(DIR_FTP + conf); // Сохраняем их на ftp-сервер
	saveMarkDown();
	m_hashCfg = "00112233445566778899aabbccddeeff00112233";

	//
	setLang(rDataConfig::instance().m_lang);

	if (!rSimpleArgs::instance().isSet(rArg::NoDump)) {
		loadDumps();
	} else {
		TRACEI(LOG::DATAMGR, "NoDump is set!");
		setLiveStatus(Live::RUNNING);
	}

	if (!rSimpleArgs::instance().isSet(rArg::NoSetIP)) {
		rSystemVariable::instance().applyEthernet();
	} else {
		TRACEI(LOG::DATAMGR, "Don't load IP addresses");
	}

	return result;
}


void rDataManager::setLang(const string &lang)
{
	rEventManager::instance().setCurLang(lang);
	rTextManager::instance().setCurLang(lang);
}


//-------------------------------------------------------------------------------------------------
//
rThreadStatus rDataManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	Live curLive;

	m_timerTotal.start(1000);

rTickCount ttt;
ttt.start(2000);

	while(true)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}


		{
		//Lock();
		rLocker lock(rVariableClass::m_mutex); lock.Nop();

		curLive = m_live.Get();

		rSystemVariable::instance().setEventAlarm(rEventManager::instance().getAlarm());
		rSystemVariable::instance().setLive(curLive);

		rSystemVariable::instance().processing();

		if (curLive == Live::RUNNING)
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
			for (auto item : m_listReport) {
				item->preCalculate();
			}

			// Основной расчет отчетов
			for (auto item : m_listReport) {
				item->calculate();
			}

			if (m_doSaveVars.Get()) {
				saveDataVariables();
			}

			if (m_timerTotal.isFinished()) {
				#ifndef TRITONN_TEST
				saveDataTotals();
				#endif
				m_timerTotal.restart();
			}
		}

		}
//		Unlock();

		if (ttt.isFinished()) {
			static int aaa = 1;
			rSnapshot ss(getVariableClass());

			ss.add("io.test_do.present.value", aaa);
			ss.set();

			if (ss("io.test_do.present.value")) {
				TRACEI(LOG::DATAMGR, "io.test_do.present.value = %i", aaa);
			}
			aaa = !aaa;
			ttt.restart();
		}

		rVariableClass::processing();
		rThreadClass::EndProccesing();

	} // while
}



UDINT rDataManager::CreateHaltEvent(rError& err)
{
	rEvent event(EID_SYSTEM_CFGERROR);

	rEventManager::instance().add(event << (static_cast<UDINT>(HaltReason::CONFIGFILE) | err.getError()) << err.getLineno());

	DoHalt(HaltReason::CONFIGFILE, err.getError());

	TRACEP(LOG::DATAMGR, "Can't load conf file '%s'. Error ID: %i. Line %i. Error string '%s'.",
			   rDataConfig::instance().m_fileName.c_str(), err.getError(), err.getLineno(), err.getText().c_str());

	return err.getError();
}


UDINT rDataManager::startInterfaces()
{
	rError err;

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


void rDataManager::startReports()
{
	for (auto item : m_listReport) {
		item->run();
	}
}


UDINT rDataManager::getConfFile(std::string& conf)
{
	std::string text;
	UDINT result = TRITONN_RESULT_OK;

	if (!rSimpleArgs::instance().isSet(rArg::ForceRun)) {

		// Проверяем на cold/warm/debug старт
		result = simpleFileLoad(FILE_RESTART, text);
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
	simpleFileDelete(FILE_RESTART);

	if (rSimpleArgs::instance().isSet(rArg::Config)) {
		conf = rSimpleArgs::instance().getOption(rArg::Config);
		return TRITONN_RESULT_OK;
	}

	result = simpleFileLoad(FILE_CONF, conf);
	if(TRITONN_RESULT_OK != result) {
		rEventManager::instance().addEventUDINT(EID_SYSTEM_CFGERROR, static_cast<UDINT>(HaltReason::CONFIGFILE) | result);

		DoHalt(HaltReason::CONFIGFILE, result);

		TRACEP(LOG::DATAMGR, "Can't load file '%s'. Error ID: %i", FILE_CONF.c_str(), result);

		return result;
	}

	return TRITONN_RESULT_OK;
}

void rDataManager::doSaveVars()
{
	#ifndef TRITONN_TEST
	m_doSaveVars.Set(1);
	#endif
}
