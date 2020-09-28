#include <iostream>

#include "tritonn_version.h"
#include "log_manager.h"
#include "event_manager.h"
#include "threadmaster.h"
#include "data_manager.h"
#include "term_manager.h"
#include "json_manager.h"
#include "data_variable.h"
#include "text_manager.h"
#include "simplefile.h"
#include "listconf.h" //NOTE TEST
#include "users.h"
#include "units.h"

#ifdef TRITONN_TEST
	#include "test_thread.h"
	#include "simpletest.h"
#endif

//#include "modbustcpslave_manager.h"
#include "hash.h"
//#include "opcua_manager.h"

int main(int argc, const char **argv)
{
	rEvent event;

	if(0)
	{
		string text    = "";
		string crypt   = "";
		string uncrypt = "";

		SimpleFileLoad("./users.xml", text);

		EncryptEAS(text, AES_KEY, AES_IV, crypt);

		SimpleFileSave("./users.crypt.txt", crypt);
	
		DecryptEAS(crypt, AES_KEY, AES_IV, uncrypt);

		SimpleFileSave("./users.uncrypt.xml", uncrypt);
	}

	rLogManager::m_logAppName = "tritonn";

	// Разбираем командную строку
#ifdef TRITONN_TEST
	rSimpleTest::Instance().Args(argc, argv);

	rThreadMaster::Instance().GetArg()->ForceConf   = "test.xml";
	rThreadMaster::Instance().GetArg()->ForceRun    = true;
	rThreadMaster::Instance().GetArg()->TerminalOut = false;
	rThreadMaster::Instance().GetArg()->logMask     = 0;
#else
	rThreadMaster::Instance().ParseArgs(argc, argv);
#endif

	rThreadMaster::Instance().Run(1000);


	// Таблицы конвертации единиц измерения
	rUnits::Init();

	// Пользователи
	rUser::Add("1"       , "1", 0xFFFFFFFF       , 0xFFFFFFFF       , 0, nullptr);
	rUser::Add("oznadmin", "1", ACCESS_MASK_ADMIN, ACCESS_MASK_ADMIN, 0, nullptr);


	//TODO Зачем тут загружаем все конфиги?
	//rListConfig::Load();

	//----------------------------------------------------------------------------------------------
	// Менеджер логирования
	rLogManager::Instance().Enable.Set(true); //TODO Может эти флаги вынести в аргументы?
	rLogManager::Instance().SetLogMask(rThreadMaster::Instance().GetArg()->logMask);
	rLogManager::Instance().Terminal.Set(rThreadMaster::Instance().GetArg()->TerminalOut);

	TRACEERROR("------------------------------------------");
	TRACEERROR("Tritonn %i.%i.%i.%i (C) VeduN, RSoft, OZNA", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_PATCH, TRITONN_VERSION_BUILD);
	rLogManager::Instance().StartServer();
	rLogManager::Instance().Run(10);

	rThreadMaster::Instance().Add(&rLogManager::Instance(), TMF_NONE, "system.logs");


	//----------------------------------------------------------------------------------------------
	// Системные строки
	if(tinyxml2::XML_SUCCESS != rTextManager::Instance().LoadSystem(FILE_SYSTEMTEXT))
	{
		TRACEERROR("Can't load system string. Error %i, line %i", rTextManager::Instance().ErrorID, rTextManager::Instance().ErrorLine);
		exit(0);
	}


	//----------------------------------------------------------------------------------------------
	// Менеджер сообщений
	rEventManager::Instance().LoadText(FILE_SYSTEMEVENT); // Системные события
	rEventManager::Instance().SetCurLang(LANG_RU); //NOTE Пока по умолчанию выставляем русский язык
	rEventManager::Instance().Run(100);

	rThreadMaster::Instance().Add(&rEventManager::Instance(), TMF_NONE, "system.events");


	//----------------------------------------------------------------------------------------------
	// Загружаем конфигурацию или переходим в cold-start
	rDataManager::Instance().LoadConfig();
	rDataManager::Instance().Run(500);

	rThreadMaster::Instance().Add(&rDataManager::Instance(), TMF_NONE, "system.data");


	//----------------------------------------------------------------------------------------------
	// Терминал
	rTermManager::Instance().Run(500);
	rTermManager::Instance().StartServer("0.0.0.0", TCP_PORT_TERM);

	rThreadMaster::Instance().Add(&rTermManager::Instance(), TMF_NONE, "system.config");


	//----------------------------------------------------------------------------------------------
	// JSON
	rJSONManager::Instance().Run(500);
	rJSONManager::Instance().StartServer("0.0.0.0", TCP_PORT_JSON);

	rThreadMaster::Instance().Add(&rJSONManager::Instance(), TMF_NONE, "system.json");

	rDataManager::Instance().StartInterfaces();

#ifdef TRITONN_TEST
	rTestThread::Instance().Run(0);
#endif

	//
	// Событие о запуске
	event.Reinit(EID_SYSTEM_RUNNING);
	rEventManager::Instance().Add(event);

	event.Reinit(EID_TEST_SUCCESS) << STRID(16) << 12.34 << 45.6 << STRID(33) << 9.87654;
	rEventManager::Instance().Add(event);
	

	while(1)
	{
#ifdef TRITONN_TEST
		if (rTestThread::Instance().GetStatus() == rThreadStatus::FINISHED) {
			rThreadMaster::Instance().Close();
			break;
		}
#endif
		if(rThreadMaster::Instance().GetStatus() == rThreadStatus::CLOSED)
		{
			TRACEW(LM_SYSTEM, "Closing...");
			break;
		}
	}
	

	rVariable::DeleteVariables();
			
	mSleep(500);
	
	TRACEERROR("Все потоки закрыты!");
	
	return 0;
}
