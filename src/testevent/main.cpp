#include "def.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <list>
#include <string.h>
#include <sys/ioctl.h>
#include "simpleargs.h"
#include "log_manager.h"
#include "event_manager.h"

namespace Args
{
const char* HOST = "host";
const char* PORT = "port";
}

pthread_t* gInfo_Event;


int main(int argc, const char **argv)
{
	rSimpleArgs::instance()
			.addOption(Args::HOST, 'h', "127.0.0.1")
			.addOption(Args::PORT, 'p', String_format("%u", LanPort::PORT_EVENT));

	rSimpleArgs::instance().parse(argc, argv);

	rLogManager::instance().m_save.Set(false);
	rLogManager::instance().m_terminal.Set(true);
	rLogManager::instance().m_enable.Set(true);

	rEventManager::instance().Connect(rSimpleArgs::instance().getOption(Args::HOST), atoi(rSimpleArgs::instance().getOption(Args::PORT).c_str()));
	rEventManager::instance().Run(16);
	gInfo_Event = rEventManager::instance().GetThread();

	while(true)
	{
		mSleep(100);
	}

	rEventManager::instance().Finish();
	pthread_join(*gInfo_Event, NULL);

   return 0;
}
