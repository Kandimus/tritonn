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


#include <string.h>
#include "tritonn_version.h"
#include "log_manager.h"
#include "simplefile.h"
#include "simpleargs.h"
#include "threadmaster.h"
#include "data_station.h"
#include "data_stream.h"
#include "data_selector.h"
#include "data_denssol.h"
#include "data_reduceddensity.h"
#include "data_ai.h"
#include "data_counter.h"
#include "data_report.h"
#include "data_rvar.h"
#include "data_variable.h"
#include "text_manager.h"
#include "event_manager.h"
#include "data_manager.h"
#include "listconf.h"
#include "argsdef.h"

//TODO Эти модули нужны только для SaveKernel, может их потом вынести в отдельный файл?
#include "modbustcpslave_manager.h"
#include "opcua_manager.h"


extern rSafityValue<DINT> gReboot;
extern rArguments         gArg;


rDataManager::rDataManager() : Live(LIVE_UNDEF), Config()
{
	RTTI             = "rDataManager";
	SysVar.Ver.Major = TRITONN_VERSION_MAJOR;
	SysVar.Ver.Minor = TRITONN_VERSION_MINOR;
	SysVar.Ver.Patch = TRITONN_VERSION_PATCH;
	SysVar.Ver.Build = TRITONN_VERSION_BUILD;

	Live.Set(LIVE_RUNNING);
	Halt.Set(false);
}

rDataManager::~rDataManager()
{
}


//-------------------------------------------------------------------------------------------------
//
rDataManager &rDataManager::Instance()
{
	static rDataManager Singleton;

	return Singleton;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
// Получение данных от менеджера данных
UDINT rDataManager::Get(rSnapshot &snapshot)
{
	rLocker locker(Mutex); locker.Nop();

	for(UDINT ii = 0; ii < snapshot.Size(); ++ii)
	{
		rSnapshotItem *item = snapshot[ii];

		if(SS_STATUS_NOTASSIGN != item->Status) continue;

		// Переменная невидимая, а уровень доступа не админ и не СА
		if((item->Variable->Flags & VARF_HIDE) && (0 == (snapshot.GetAccess() & ACCESS_MASK_VIEWHIDE)))
		{
			item->Status = SS_STATUS_NOTFOUND;
			continue;
		}

		memcpy(item->Data, item->Variable->Pointer, EPT_SIZE[item->Variable->Type]);
		item->Status = SS_STATUS_ASSIGN;
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Запись данных в менеджер данных
UDINT rDataManager::Set(rSnapshot &snapshot)
{
	rLocker locker(Mutex); locker.Nop();

	for(UDINT ii = 0; ii < snapshot.Size(); ++ii)
	{
		rSnapshotItem *item = snapshot[ii];

		if(SS_STATUS_ASSIGN != item->Status) continue;

		// Переменная невидимая, а уровень доступа не админ и не СА
		if((item->Variable->Flags & VARF_HIDE) && (0 == (snapshot.GetAccess() & ACCESS_MASK_VIEWHIDE)))
		{
			item->Status = SS_STATUS_NOTFOUND;
			continue;
		}

		// Переменная только для чтения
		if(item->Variable->Flags & VARF_READONLY)
		{
			if(item->Variable->Flags & VARF_SUWRITE)
			{
				if(0 == (snapshot.GetAccess() & ACCESS_SA))
				{
					item->Status = SS_STATUS_READONLY;
					continue;
				}
			}
			else
			{
				item->Status = SS_STATUS_READONLY;
				continue;
			}
		}

		if((item->Variable->Access & snapshot.GetAccess()) != item->Variable->Access)
		{
			item->Status = SS_STATUS_ACCESSDENIED;

			//TODO Выдать сообщение
			continue;
		}

		memcpy(item->Variable->Pointer, item->Data, EPT_SIZE[item->Variable->Type]);
		item->Status = SS_STATUS_WRITED;
	}

	return 0;
}


UDINT rDataManager::GetAllVariables(rSnapshot &snapshot)
{
	rLocker locker(Mutex); UNUSED(locker);

	snapshot.Clear();

	for(UDINT ii = 0; ii < rVariable::ListVar.size(); ++ii)
	{
		const rVariable *var = rVariable::ListVar[ii];

		if(var->Flags & VARF_HIDE) continue;

		snapshot.Add(var->Name);
	}

	return TRITONN_RESULT_OK;
}


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

	rEventManager::Instance().AddEventUDINT(EID_SYSTEM_HALT, reason);

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
			rEventManager::Instance().AddEvent(EID_SYSTEM_RESTART_WARM);
			break;

		case RESTART_COLD:
			TRACEI(LM_SYSTEM, "Command Cold-restart");
			SimpleFileSave(FILE_RESTART, "cold");
			rEventManager::Instance().AddEvent(EID_SYSTEM_RESTART_COLD);
			break;

		case RESTART_DEBUG:
			TRACEI(LM_SYSTEM, "Command Debug-restart");
			SimpleFileSave(FILE_RESTART, "debug");
			break;

		default:
			TRACEI(LM_SYSTEM, "Unkonow command restart");
			rEventManager::Instance().AddEventUDINT(EID_SYSTEM_RESTART_UNKNOW, restart);
			return 1;
	}

	//
	switch(live)
	{
		case LIVE_STARTING:
		case LIVE_RUNNING:
		case LIVE_HALT:
			rThreadMaster::Instance().Finish();
			return 0;

		case LIVE_REBOOT_COLD:
			if(filename.size())
			{
				TRACEW(LM_SYSTEM, "Set new conf file: '%s'", filename.c_str());
				SimpleFileSave(FILE_CONF, filename);
			}
			rThreadMaster::Instance().Finish();
			return 0;

		default:
			TRACEA(LM_SYSTEM, "Unknow live status");
			rThreadMaster::Instance().Finish();
			return 2;
	}
}



// Защиты по мьютексу нет, так как это статические данные и не меняются
void rDataManager::GetVersion(rVersion &ver) const
{
	ver = SysVar.Ver;
}

void rDataManager::GetState(rState &state)
{
	rLocker locker(Mutex); locker.Nop();

	state = SysVar.State;
}

void rDataManager::GetTime(struct tm &sdt)
{
	rLocker locker(Mutex); locker.Nop();

	sdt = SysVar.DateTime;
}

void rDataManager::GetConfigInfo(rConfigInfo &conf) const
{
	conf = SysVar.ConfigInfo;
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

	rDataConfig::InitBitFlags();

	strcpy(SysVar.Lang, LANG_RU.c_str());

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
		result = Config.LoadFile(conf, SysVar, ListSource, ListInterface, ListReport);
		if(TRITONN_RESULT_OK != result)
		{
			return CreateConfigHaltEvent(Config);
		}

		//TODO После нужно загрузить данные из EEPROM и сравнить с конфигой
	}

	//--------------------------------------------
	// Создаем переменные
	SysVar.InitVariables(rVariable::ListVar);

	// Добавляем интерфейсы, создаем под них переменные
	for(auto interface : ListInterface)
	{
		rThreadMaster::Instance().Add(interface->GetThreadClass(), TMF_DELETE | TMF_NOTRUN, interface->Alias);
	}

	// Собираем переменные от объектов
	for(auto source : ListSource)
	{
		//TODO Нужно ли выдавать ошибку?
		if(source->GenerateVars(rVariable::ListVar)) return 1;
	}

	// Собираем переменные от интерфейсов
	for(auto interface : ListInterface)
	{
		//TODO Нужно ли выдавать ошибку?
		if(interface->GenerateVars(rVariable::ListVar)) return 1;
	}

	rVariable::Sort();

	//--------------------------------------------
	//NOTE только в процессе разработки
	if(LIVE_STARTING == GetLiveStatus())
	{
		rVariable::SaveToCSV(DIR_FTP + conf); // Сохраняем их на ftp-сервер
		SaveKernel();                         // Сохраняем описание ядра
	}

	//TODO  Тут нужно проверить переменные в интерфейсах
	for(UDINT ii = 0; ii < ListInterface.size(); ++ii)
	{
		result = ListInterface[ii]->CheckVars(Config);
		if(TRITONN_RESULT_OK != result)
		{
			return CreateConfigHaltEvent(Config);
		}
	}

	//--------------------------------------------
	//TODO Нужно из rTextManager и rEventManager извлеч список языков, и сформировать единый список внутри rDataManager
	//TODO Сохранить массивы строк для WEB

	//
	SetLang(SysVar.Lang);

	if(LIVE_STARTING == GetLiveStatus())
	{
		SetLiveStatus(LIVE_RUNNING);
	}

	return result;
}


const rConfigInfo *rDataManager::GetConfName() const
{
	return &SysVar.ConfigInfo;
}




UDINT rDataManager::SetLang(const string &lang)
{
	rEventManager::Instance().SetCurLang(lang);
	rTextManager::Instance().SetCurLang(lang);

	strncpy(SysVar.Lang, lang.c_str(), 8);

	return 0;
}


//-------------------------------------------------------------------------------------------------
//TODO Переделать на SimpleFile
UDINT rDataManager::SaveKernel()
{
	auto file    = fopen((DIR_FTP + "kernel.xml").c_str(), "wt");
	auto stn     = new rStation();
	auto str     = new rStream();
	auto ssel    = new rSelector();
	auto msel    = new rSelector();
	auto denssol = new rDensSol();
	auto rdcdens = new rReducedDens();
	auto ai      = new rAI();
	auto fi      = new rCounter();
	auto rep     = new rReport();
	auto rvar    = new rRVar();
	auto mbSlTCP = new rModbusTCPSlaveManager();
//	auto opcua   = new rOPCUAManager();

//	stn.Alias      = "#totalsource_1";
	stn->UnitVolume = U_m3;
	stn->UnitMass   = U_t;

//	str.Alias      = "#totalsource_2";
	str->Station    = stn;

	ssel->GenerateIO();

	msel->Setup.Value |= SELECTOR_SETUP_MULTI;
	msel->GenerateIO();

	if(!file) return 1;
	fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(file, "<kernel ver=\"%i.%i\" xmlns=\"http://tritonn.ru\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://tritonn.ru ./kernel.xsd\">\n"
			  , TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR);

	SysVar.SaveKernel(file);

	rvar->SaveKernel(file, false, "var", "Переменная", true);

	fprintf(file, "\n<io_list>\n");
	ai->SaveKernel(file, true, "ai", "Аналоговый сигнал", true);
	fi->SaveKernel(file, true, "counter", "Частотно-импульсный сигнал", true);
	fprintf(file, "</io_list>\n");

	fprintf(file, "<objects>\n");
	denssol->SaveKernel(file, false, "densitometer", "Плотномер (Солартрон)", false);
	rdcdens->SaveKernel(file, false, "reduceddens", "Приведение плотности", true);
	ssel->SaveKernel   (file, false, "selector", "Селектор", true);
	msel->SaveKernel   (file, false, "multiselector", "Мультиселектор", true);
	stn->SaveKernel    (file, false, "station", "Станция", true);
	str->SaveKernel    (file, false, "stream", "Линия", false);
	rep->Type = REPORT_PERIODIC;
	rep->SaveKernel    (file, false, "report", "Отчет (периодический)", true);
	rep->Type = REPORT_BATCH;
	rep->SaveKernel    (file, false, "report", "Отчет (по партии)", true);
	fprintf(file, "</objects>\n");

	fprintf(file, "<interfaces>\n");
	mbSlTCP->SaveKernel(file, "ModbusSlaveTCP", "Модбас слейв TCP");
//	opcua->SaveKernel(file, "OPCUA", "OPC UA server");
	fprintf(file, "</interfaces>\n");

	fprintf(file, "</kernel>");

	fclose(file);

//	delete opcua;
	delete mbSlTCP;
	delete rvar;
	delete rep;
	delete fi;
	delete ai;
	delete rdcdens;
	delete denssol;
	delete msel;
	delete ssel;
	delete str;
	delete stn;

	return 0;
}



//-------------------------------------------------------------------------------------------------
//
rThreadStatus rDataManager::Proccesing()
{
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	UDINT ii = 0;

	while(true)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if (!THREAD_IS_WORK(thread_status)) {
			return thread_status;
		}

		Lock();

		SysVar.State.EventAlarm = rEventManager::Instance().GetAlarm();
		SysVar.State.Live       = Live.Get();

		GetCurrentTime(SysVar.UnixTime, &SysVar.DateTime);

		if(SysVar.SetDateTimeAccept)
		{
			SysVar.SetDateTimeAccept = 0;
			//TODO Нужно менять время )))
			SysVar.SetDateTime.tm_sec  = 0;
			SysVar.SetDateTime.tm_min  = 0;
			SysVar.SetDateTime.tm_hour = 0;
			SysVar.SetDateTime.tm_mday = 0;
			SysVar.SetDateTime.tm_mon  = 0;
			SysVar.SetDateTime.tm_year = 0;
		}

		if(SysVar.State.Live == LIVE_RUNNING)
		{

			//TODO Получаем данные пришедшие по CAN-шине

//			++Snapshot.IO.AI[0].Code; //TODO Для теста )))

			// Пердвычисления для всех объектов
			for(ii = 0; ii < ListSource.size(); ++ii)
			{
				ListSource[ii]->PreCalculate();
			}

			// Основной расчет всех объектов
			for(ii = 0; ii < ListSource.size(); ++ii)
			{
				ListSource[ii]->Calculate();
			}

			// Пердвычисления для всех объектов
			for(ii = 0; ii < ListReport.size(); ++ii)
			{
				ListReport[ii]->PreCalculate();
			}

			// Основной расчет отчетов
			for(ii = 0; ii < ListReport.size(); ++ii)
			{
				ListReport[ii]->Calculate();
			}
		}

      Unlock();
		
		rThreadClass::EndProccesing();
	} // while
}



UDINT rDataManager::CreateConfigHaltEvent(rDataConfig &cfg)
{
	rEvent event(EID_SYSTEM_CFGERROR);

	event << (HALT_REASON_CONFIGFILE | cfg.ErrorID) << cfg.ErrorLine;

	rEventManager::Instance().Add(event);

	DoHalt(HALT_REASON_CONFIGFILE | cfg.ErrorID);

	TRACEERROR("Can't load conf file '%s'. Error ID: %i. Line %i. Error string '%s'.", cfg.FileName.c_str(), cfg.ErrorID, cfg.ErrorLine, cfg.ErrorStr.c_str());

	return cfg.ErrorID;
}


UDINT rDataManager::StartInterfaces()
{
	for (auto interface : ListInterface) {
		interface->StartServer();
	}

	return TRITONN_RESULT_OK;
}


UDINT rDataManager::getConfFile(std::string& conf)
{
	std::string text;
	UDINT result = TRITONN_RESULT_OK;

	if (!rSimpleArgs::instance().isSet(rArgs::ForceRun)) {

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

	if (rSimpleArgs::instance().isSet(rArgs::ForceConf)) {
		conf = rSimpleArgs::instance().getOption(rArgs::ForceConf);
		return TRITONN_RESULT_OK;
	}

	result = SimpleFileLoad(FILE_CONF, conf);
	if(TRITONN_RESULT_OK != result) {
		rEventManager::Instance().AddEventUDINT(EID_SYSTEM_CFGERROR, HALT_REASON_CONFIGFILE | result);

		DoHalt(HALT_REASON_CONFIGFILE | result);

		TRACEERROR("Can't load file '%s'. Error ID: %i", FILE_CONF.c_str(), result);

		return result;
	}

	return TRITONN_RESULT_OK;
}
