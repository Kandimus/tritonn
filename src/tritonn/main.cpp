#include <iostream>

#include "tritonn_version.h"
#include "log_manager.h"
#include "event/manager.h"
#include "threadmaster.h"
#include "system_manager.h"
#include "data_manager.h"
#include "io/manager.h"
#include "term_manager.h"
#include "json_manager.h"
#include "error.h"
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

	rLogManager::instance().setDir(DIR_LOG);

	// Разбираем командную строку
#ifdef TRITONN_TEST

	rTestThread::instance().setArgs(argc, argv);

	rSimpleArgs::instance()
			.setSwitch(rArg::ForceRun , true)
			.setSwitch(rArg::Terminal , false)
			.setSwitch(rArg::Simulate , true)
			.setSwitch(rArg::NoDump   , true)
			.setOption(rArg::Log      , "FFFFFFFF")
			.setOption(rArg::Config   , "test.xml");
#else
	rSimpleArgs::instance()
			.addSwitch(rArg::ForceRun , 'f')
			.addSwitch(rArg::Terminal , 't')
			.addSwitch(rArg::Simulate , 's')
			.addSwitch(rArg::NoDump   , 'd')
			.addOption(rArg::Log      , 'l', "FFFFFFFF")
			.addOption(rArg::Config   , 'c', "test_sikn.xml");

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

	rLogManager::instance().m_enable.Set(true);
	rLogManager::instance().setLogMask(logmask);
	rLogManager::instance().m_terminal.Set(true);
	TRACEI(LOG::MAIN, " ");
	TRACEI(LOG::MAIN, "----------------------------------------------------------------------------------------------");
	TRACEI(LOG::MAIN, "Tritonn %i.%i.%i.%x (C) VeduN, 2019-2020 RSoft, OZNA", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_BUILD, TRITONN_VERSION_HASH);
	rLogManager::instance().m_terminal.Set(rSimpleArgs::instance().isSet(rArg::Terminal));
	rLogManager::instance().Run(16);

	rThreadMaster::instance().add(&rLogManager::instance(), TMF_NONE, "logs");


	// Менеджер системных команд
	rSystemManager::instance().Run(250);
	rThreadMaster::instance().add(&rSystemManager::instance(), TMF_NONE, "system");


	// Системные строки
	rError err;
	if(TRITONN_RESULT_OK != rTextManager::instance().loadSystem(FILE_SYSTEMTEXT, err))
	{
		TRACEP(LOG::MAIN, "Can't load system string. Error %i, line %i '%s'", err.getError(), err.getLineno(), err.getText().c_str());
		exit(0);
	}
	rTextManager::instance().setCurLang(LANG_RU);


	// Менеджер сообщений
	rEventManager::instance().loadText(FILE_SYSTEMEVENT); // Системные события
	rEventManager::instance().setCurLang(LANG_RU); //NOTE Пока по умолчанию выставляем русский язык
	rEventManager::instance().Run(16);
	rEventManager::instance().startServer();

	rThreadMaster::instance().add(&rEventManager::instance(), TMF_NONE, "events");


	// Загружаем конфигурацию или переходим в cold-start
	rDataManager::instance().LoadConfig();
	rDataManager::instance().Run(100);

	rThreadMaster::instance().add(&rDataManager::instance(), TMF_NONE, "metrology");


	TRACEI(LOG::MAIN, "Log storage: %u / %u", rLogManager::instance().COMPRESS_DAYS, rLogManager::instance().DELETE_DAYS);
	TRACEI(LOG::MAIN, "Event storage: %u / %u", rEventManager::instance().getStorage(), rEventManager::instance().DELETE_DAYS);


	// Стартуем обмен с модулями IO
	rIOManager::instance().Run(100);

	rThreadMaster::instance().add(&rIOManager::instance(), TMF_NONE, "io");


	// Терминал
	rTermManager::Instance().Run(500);
	rTermManager::Instance().StartServer("0.0.0.0", LanPort::PORT_TERM);

	rThreadMaster::instance().add(&rTermManager::Instance(), TMF_NONE, "config");


	//----------------------------------------------------------------------------------------------
	// JSON
	rJSONManager::Instance().Run(100);
	rJSONManager::Instance().StartServer("0.0.0.0", LanPort::PORT_JSON);

	rThreadMaster::instance().add(&rJSONManager::Instance(), TMF_NONE, "web");

	//
	rDataManager::instance().StartInterfaces();

	//
	// Событие о запуске
	event.reinit(EID_SYSTEM_RUNNING);
	rEventManager::instance().add(event);

	event.reinit(EID_TEST_SUCCESS) << STRID(16) << 12.34 << 45.6 << STRID(33) << 9.87654;
	rEventManager::instance().add(event);
	

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
			TRACEW(LOG::MAIN, "Closing...");
			break;
		}

		mSleep(100);
	}

	mSleep(500);
	
	TRACEI(LOG::MAIN, "Все потоки закрыты!");
	
	return 0;
}
