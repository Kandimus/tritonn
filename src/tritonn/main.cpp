#include <iostream>

#include "tritonn_version.h"
#include "log_manager.h"
#include "event_manager.h"
#include "threadmaster.h"
#include "data_manager.h"
#include "io/manager.h"
#include "term_manager.h"
#include "json_manager.h"
//#include "data_variable.h"
#include "text_manager.h"
#include "simplefile.h"
#include "simpleargs.h"
#include "stringex.h"
#include "listconf.h" //NOTE TEST
#include "users.h"
#include "units.h"
#include "def_arguments.h"

#ifdef TRITONN_TEST
#include "test_thread.h"
#endif

#include "hash.h"

int main(int argc, char* argv[])
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

	rTestThread::instance().setArgs(argc, argv);

	rSimpleArgs::instance()
			.setSwitch(rArg::ForceRun , true)
			.setSwitch(rArg::Terminal , false)
			.setSwitch(rArg::Simulate , true)
			.setOption(rArg::Log      , "FFFFFFFF")
			.setOption(rArg::ForceConf, "test.xml");
#else
	rSimpleArgs::instance()
			.addSwitch(rArg::ForceRun , 'f')
			.addSwitch(rArg::Terminal , 't')
			.addSwitch(rArg::Simulate , 's')
			.addOption(rArg::Log      , 'l', "FFFFFFFF")
			.addOption(rArg::ForceConf, 'c', "test_sikn.xml");

	rSimpleArgs::instance().parse(argc, (const char**)argv);
#endif

	rThreadMaster::instance().Run(1000);


	// Таблицы конвертации единиц измерения
	rUnits::Init();

	// Пользователи
	rUser::Add("1"       , "1", 0xFFFFFFFF       , 0xFFFFFFFF       , 0, nullptr);
	rUser::Add("oznadmin", "1", ACCESS_MASK_ADMIN, ACCESS_MASK_ADMIN, 0, nullptr);


	//TODO Зачем тут загружаем все конфиги?
	//rListConfig::Load();

	//----------------------------------------------------------------------------------------------
	// Менеджер логирования
	UDINT logmask = 0;
	String_IsValidHex(rSimpleArgs::instance().getOption(rArg::Log).c_str(), logmask);

	rLogManager::Instance().Enable.Set(true);
	rLogManager::Instance().SetLogMask(logmask);
	rLogManager::Instance().Terminal.Set(rSimpleArgs::instance().isSet(rArg::Terminal));

	TRACEERROR("------------------------------------------");
	TRACEERROR("Tritonn %i.%i.%i.%x (C) VeduN, 2019-2020 RSoft, OZNA", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_BUILD, TRITONN_VERSION_HASH);
	rLogManager::Instance().StartServer();
	rLogManager::Instance().Run(16);

	rThreadMaster::instance().add(&rLogManager::Instance(), TMF_NONE, "logs");


	//----------------------------------------------------------------------------------------------
	// Системные строки
	if(tinyxml2::XML_SUCCESS != rTextManager::Instance().LoadSystem(FILE_SYSTEMTEXT))
	{
		TRACEERROR("Can't load system string. Error %i, line %i", rTextManager::Instance().ErrorID, rTextManager::Instance().ErrorLine);
		exit(0);
	}


	//----------------------------------------------------------------------------------------------
	// Менеджер сообщений
	rEventManager::instance().LoadText(FILE_SYSTEMEVENT); // Системные события
	rEventManager::instance().SetCurLang(LANG_RU); //NOTE Пока по умолчанию выставляем русский язык
	rEventManager::instance().Run(16);

	rThreadMaster::instance().add(&rEventManager::instance(), TMF_NONE, "events");


	//----------------------------------------------------------------------------------------------
	// Загружаем конфигурацию или переходим в cold-start
	rDataManager::instance().LoadConfig();
	rDataManager::instance().Run(100);

	rThreadMaster::instance().add(&rDataManager::instance(), TMF_NONE, "metrology");


	//----------------------------------------------------------------------------------------------
	// Стартуем обмен с модулями IO
	rIOManager::instance().Run(100);

	rThreadMaster::instance().add(&rIOManager::instance(), TMF_NONE, "io");


	//----------------------------------------------------------------------------------------------
	// Терминал
	rTermManager::Instance().Run(500);
	rTermManager::Instance().StartServer("0.0.0.0", TCP_PORT_TERM);

	rThreadMaster::instance().add(&rTermManager::Instance(), TMF_NONE, "config");


	//----------------------------------------------------------------------------------------------
	// JSON
	rJSONManager::Instance().Run(500);
	rJSONManager::Instance().StartServer("0.0.0.0", TCP_PORT_JSON);

	rThreadMaster::instance().add(&rJSONManager::Instance(), TMF_NONE, "web");

	//
	rDataManager::instance().StartInterfaces();

	//
	// Событие о запуске
	event.Reinit(EID_SYSTEM_RUNNING);
	rEventManager::instance().Add(event);

	event.Reinit(EID_TEST_SUCCESS) << STRID(16) << 12.34 << 45.6 << STRID(33) << 9.87654;
	rEventManager::instance().Add(event);
	

#ifdef TRITONN_TEST
	rThreadStatus oldteststatus = rThreadStatus::UNDEF;
	rTestThread::instance().Run(0);
#endif

	while(1)
	{
#ifdef TRITONN_TEST
		rThreadStatus teststatus = rTestThread::instance().GetStatus();
		if (oldteststatus != rThreadStatus::FINISHED && teststatus == rThreadStatus::FINISHED) {
			rThreadMaster::instance().Finish();
			mSleep(1000);
		}
		oldteststatus = teststatus;
#endif

		if (rThreadMaster::instance().GetStatus() == rThreadStatus::CLOSED) {
			TRACEW(LM_SYSTEM, "Closing...");
			break;
		}

		mSleep(100);
	}

	mSleep(500);
	
	TRACEERROR("Все потоки закрыты!");
	
	return 0;
}
