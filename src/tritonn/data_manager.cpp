//=================================================================================================
//===
//=== data_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
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
#include "data_station.h"
#include "data_stream.h"
#include "data_selector.h"
#include "data_denssol.h"
#include "data_reduceddensity.h"
#include "data_ai.h"
#include "data_di.h"
#include "data_do.h"
#include "data_counter.h"
#include "data_report.h"
#include "data_rvar.h"
#include "data_sampler.h"
#include "text_manager.h"
#include "event_manager.h"
#include "io/manager.h"
#include "listconf.h"
#include "def_arguments.h"

//TODO Эти модули нужны только для SaveKernel, может их потом вынести в отдельный файл?
#include "generator_md.h"
#include "modbustcpslave_manager.h"
#include "opcua_manager.h"


extern rSafityValue<DINT> gReboot;


rDataManager::rDataManager() : rVariableClass(Mutex), Live(LIVE_UNDEF)
{
	RTTI                 = "rDataManager";
	m_sysVar.Ver.m_major = TRITONN_VERSION_MAJOR;
	m_sysVar.Ver.m_minor = TRITONN_VERSION_MINOR;
	m_sysVar.Ver.m_build = TRITONN_VERSION_BUILD;
	m_sysVar.Ver.m_hash  = TRITONN_VERSION_HASH;


	Live.Set(LIVE_RUNNING);
	Halt.Set(false);
}

rDataManager::~rDataManager()
{
}


//-------------------------------------------------------------------------------------------------
//


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//
void rDataManager::SetLiveStatus(USINT status)
{
	if(Live.Get() != LIVE_HALT)
	{
		Live.Set(status);
	}
}


USINT rDataManager::GetLiveStatus()
{
	return Live.Get();
}


void rDataManager::DoHalt(UDINT reason)
{
	if(Halt.Get()) return;

	Halt.Set(true);
	Live.Set(LIVE_HALT);

	rEventManager::instance().AddEventUDINT(EID_SYSTEM_HALT, reason);

	SimpleFileSave(FILE_RESTART, "cold");
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDataManager::Restart(USINT restart, const string &filename)
{
	UDINT live = GetLiveStatus();

	switch(restart)
	{
		case RESTART_WARM:
			TRACEI(LM_SYSTEM, "Command Warm-restart");
			rEventManager::instance().AddEvent(EID_SYSTEM_RESTART_WARM);
			break;

		case RESTART_COLD:
			TRACEI(LM_SYSTEM, "Command Cold-restart");
			SimpleFileSave(FILE_RESTART, "cold");
			rEventManager::instance().AddEvent(EID_SYSTEM_RESTART_COLD);
			break;

		case RESTART_DEBUG:
			TRACEI(LM_SYSTEM, "Command Debug-restart");
			SimpleFileSave(FILE_RESTART, "debug");
			break;

		default:
			TRACEI(LM_SYSTEM, "Unkonow command restart");
			rEventManager::instance().AddEventUDINT(EID_SYSTEM_RESTART_UNKNOW, restart);
			return 1;
	}

	//
	switch(live)
	{
		case LIVE_STARTING:
		case LIVE_RUNNING:
		case LIVE_HALT:
			rThreadMaster::instance().Finish();
			return 0;

		case LIVE_REBOOT_COLD:
			if(filename.size())
			{
				TRACEW(LM_SYSTEM, "Set new conf file: '%s'", filename.c_str());
				SimpleFileSave(FILE_CONF, filename);
			}
			rThreadMaster::instance().Finish();
			return 0;

		default:
			TRACEA(LM_SYSTEM, "Unknow live status");
			rThreadMaster::instance().Finish();
			return 2;
	}
}



// Защиты по мьютексу нет, так как это статические данные и не меняются
void rDataManager::GetVersion(rVersion &ver) const
{
	ver = m_sysVar.Ver;
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
	SetLiveStatus(LIVE_STARTING);

	//TODO Нужно очистить директорию ftp

	result = getConfFile(conf);
	if(result != TRITONN_RESULT_OK) {
		return result;
	}

	// Если это не cold-start, то загружаем конфигурацию
	if(LIVE_STARTING == GetLiveStatus()) {

		//TODO проверить на валидность hash
		TRACEI(LM_SYSTEM | LM_I, "Load config file '%s'", conf.c_str());

		if(rDataConfig::instance().LoadFile(conf, m_sysVar, ListSource, ListInterface, ListReport) != TRITONN_RESULT_OK) {
			return CreateHaltEvent(rDataConfig::instance().m_error);
		}

		//TODO После нужно загрузить данные из EEPROM и сравнить с конфигой
	}

	//--------------------------------------------
	// Создаем переменные
	m_sysVar.initVariables(m_varList);

	// Добавляем интерфейсы, создаем под них переменные
	for(auto interface : ListInterface) {
		rThreadMaster::instance().add(interface->GetThreadClass(), TMF_DELETE | TMF_NOTRUN, interface->Alias);
	}

	// Собираем переменные от объектов
	for (auto source : ListSource) {
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
	if(LIVE_STARTING == GetLiveStatus())
	{
		m_varList.saveToCSV(DIR_FTP + conf); // Сохраняем их на ftp-сервер
		SaveKernel();                         // Сохраняем описание ядра
		saveMarkDown();
	}

	//--------------------------------------------
	//TODO Нужно из rTextManager и rEventManager извлеч список языков, и сформировать единый список внутри rDataManager
	//TODO Сохранить массивы строк для WEB

	//
	SetLang(m_sysVar.Lang);

	if(LIVE_STARTING == GetLiveStatus())
	{
		SetLiveStatus(LIVE_RUNNING);
	}

	return result;
}


const rConfigInfo *rDataManager::GetConfName() const
{
	return &m_sysVar.ConfigInfo;
}




UDINT rDataManager::SetLang(const string &lang)
{
	rEventManager::instance().SetCurLang(lang);
	rTextManager::instance().SetCurLang(lang);

	strncpy(m_sysVar.Lang, lang.c_str(), 8);

	return 0;
}

UDINT rDataManager::saveMarkDown()
{
	rGeneratorMD md;
	rSampler smp;

	smp.generateMarkDown(md);

	md.save(DIR_MARKDOWN);

	return TRITONN_RESULT_OK;
}


//-------------------------------------------------------------------------------------------------
//TODO Переделать на SimpleFile
UDINT rDataManager::SaveKernel()
{
	std::string text = "";
	auto stn     = new rStation();
	auto str     = new rStream();
	auto ssel    = new rSelector();
	auto msel    = new rSelector();
	auto denssol = new rDensSol();
	auto rdcdens = new rReducedDens();
	rAI ai;
	rDI di;
	rDO do_;
	rCounter fi;
	rSampler smp;
	auto rep     = new rReport();
	auto rvar    = new rRVar();
	auto mbSlTCP = new rModbusTCPSlaveManager();
//	auto opcua   = new rOPCUAManager();

	stn->UnitVolume = U_m3;
	stn->UnitMass   = U_t;

	str->Station    = stn;

	ssel->GenerateIO();

	msel->Setup.Value |= SELECTOR_SETUP_MULTI;
	msel->GenerateIO();

	text += String_format("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
						  "<kernel ver=\"%i.%i\" xmlns=\"http://tritonn.ru\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://tritonn.ru ./kernel.xsd\">\n",
						  TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR);

	text += m_sysVar.saveKernel();

	text += rvar->saveKernel(false, "var", "Переменная", true);

	text += "\n<!-- \n\tIO objects list \n-->\n<io_list>\n";
	text += ai.saveKernel (true, "ai", "Аналоговый сигнал", true);
	text += fi.saveKernel (true, "counter", "Частотно-импульсный сигнал", true);
	text += di.saveKernel (true, "di", "Дискретный входной сигнал", true);
	text += do_.saveKernel(true, "do", "Дискретный выходной сигнал", true);
	text += "</io_list>\n";

	text += "\n<!-- \n\tStation/stream objects list \n-->\n<objects>\n";
	text += smp.saveKernel(false, "sampler", "Пробоотборник", false);
	text += denssol->saveKernel(false, "densitometer", "Плотномер (Солартрон)", false);
	text += rdcdens->saveKernel(false, "reduceddens", "Приведение плотности", true);
	text += ssel->saveKernel   (false, "selector", "Селектор", true);
	text += msel->saveKernel   (false, "multiselector", "Мультиселектор", true);
	text += stn->saveKernel    (false, "station", "Станция", true);
	text += str->saveKernel    (false, "stream", "Линия", false);
	rep->Type = REPORT_PERIODIC;
	text += rep->saveKernel    (false, "report", "Отчет (периодический)", true);
	rep->Type = REPORT_BATCH;
	text += rep->saveKernel    (false, "report", "Отчет (по партии)", true);
	text += "</objects>\n";

	text += "\n<!-- \nExternal interfaces \n-->\n<interfaces>\n";
	text += mbSlTCP->saveKernel("ModbusSlaveTCP", "Модбас слейв TCP");
//	opcua->SaveKernel(file, "OPCUA", "OPC UA server");
	text += "</interfaces>\n";

	text += rIOManager::instance().saveKernel();

	text += "\n</kernel>";

	UDINT result = SimpleFileSave(DIR_FTP + "kernel.xml", text);

//	delete opcua;
	delete mbSlTCP;
	delete rvar;
	delete rep;
	delete rdcdens;
	delete denssol;
	delete msel;
	delete ssel;
	delete str;
	delete stn;

	return result;
}



//-------------------------------------------------------------------------------------------------
//
rThreadStatus rDataManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;

	while(true)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

		Lock();

		m_sysVar.m_state.EventAlarm = rEventManager::instance().GetAlarm();
		m_sysVar.m_state.Live       = Live.Get();

		GetCurrentTime(m_sysVar.UnixTime, &m_sysVar.DateTime);

		if (m_sysVar.SetDateTimeAccept) {
			m_sysVar.SetDateTimeAccept = 0;
			//TODO Нужно менять время )))
			m_sysVar.SetDateTime.tm_sec  = 0;
			m_sysVar.SetDateTime.tm_min  = 0;
			m_sysVar.SetDateTime.tm_hour = 0;
			m_sysVar.SetDateTime.tm_mday = 0;
			m_sysVar.SetDateTime.tm_mon  = 0;
			m_sysVar.SetDateTime.tm_year = 0;
		}

		if(m_sysVar.m_state.Live == LIVE_RUNNING)
		{
			// Пердвычисления для всех объектов
			for (auto item : ListSource) {
				item->PreCalculate();
			}

			// Основной расчет всех объектов
			for (auto item : ListSource) {
				item->Calculate();
			}

			// Пердвычисления для отчетов
			for (auto item : ListReport) {
				item->PreCalculate();
			}

			// Основной расчет отчетов
			for (auto item : ListReport) {
				item->Calculate();
			}
		}

		rVariableClass::processing();
		rThreadClass::EndProccesing();
		Unlock();
	} // while
}



UDINT rDataManager::CreateHaltEvent(rError& err)
{
	rEvent event(EID_SYSTEM_CFGERROR);

	event << (HALT_REASON_CONFIGFILE | err.getError()) << err.getLineno();

	rEventManager::instance().Add(event);

	DoHalt(HALT_REASON_CONFIGFILE | err.getError());

	TRACEERROR("Can't load conf file '%s'. Error ID: %i. Line %i. Error string '%s'.",
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
		if (interface->CheckVars(err) != TRITONN_RESULT_OK) {
			return CreateHaltEvent(err);
		}
	}

	for (auto interface : ListInterface) {
		interface->StartServer();
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
				SetLiveStatus(LIVE_REBOOT_COLD);

				// Загружаем список конфигураций
				rListConfig::Load();

				TRACEI(LM_SYSTEM, "Cold restart!");
				//TODO Нужно выложить в web все языковые файлы
			}
			if ("debug" == text) {
				//TODO Доделать
				SetLiveStatus(LIVE_REBOOT_COLD);

				// Загружаем список конфигураций
				rListConfig::Load();
			}
		}
	} else {
		TRACEI(LM_SYSTEM, "Forced run!");
	}

	// удаляем файл
	SimpleFileDelete(FILE_RESTART);

	if (rSimpleArgs::instance().isSet(rArg::Config)) {
		conf = rSimpleArgs::instance().getOption(rArg::Config);
		return TRITONN_RESULT_OK;
	}

	result = SimpleFileLoad(FILE_CONF, conf);
	if(TRITONN_RESULT_OK != result) {
		rEventManager::instance().AddEventUDINT(EID_SYSTEM_CFGERROR, HALT_REASON_CONFIGFILE | result);

		DoHalt(HALT_REASON_CONFIGFILE | result);

		TRACEERROR("Can't load file '%s'. Error ID: %i", FILE_CONF.c_str(), result);

		return result;
	}

	return TRITONN_RESULT_OK;
}
