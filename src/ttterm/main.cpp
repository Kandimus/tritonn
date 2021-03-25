#include <iostream>
#include <cstring>
#include <sstream>
#include <list>
#include <string.h>
#include <sys/ioctl.h>
#include "safity.h"
#include "simpleargs.h"
#include "log_manager.h"
#include "packet_client.h"
#include "packet_set.h"
#include "packet_get.h"
#include "tritonn_manager.h"
#include "display_manager.h"

namespace Args
{

const char* USER     = "user";
const char* PASSWORD = "password";
const char* HOST     = "host";
const char* PORT     = "port";

}


rPacketClient       gTritonnClient;
rTritonnManager     gTritonnManager(gTritonnClient);
rDisplayManager     gDisplayManager;
rSafityValue<UDINT> gExit;
pthread_t*          gInfo_Tritonn;
pthread_t*          gInfo_Display;
pthread_t*          gInfo_Log;
//rPacketSetData      gPeriodicSetData;
//rPacketGetData      gPeriodicGetData;


extern void  LogCallback(const string &text);
extern UDINT PeriodicSetAdd(rPacketSetData &pset, const string &name, const string &val);




int main(int argc, const char **argv)
{
	rSimpleArgs::instance()
			.addOption("autotest"    , 'a', "")
			.addOption(Args::USER    , 'U', "")
			.addOption(Args::PASSWORD, 'P', "")
			.addOption(Args::HOST    , 'h', "127.0.0.1")
			.addOption(Args::PORT    , 'p', String_format("%u", TCP_PORT_TERM));

	rSimpleArgs::instance().parse(argc, argv);

	rLogManager::instance().m_terminal.Set(false);  // Запрещаем вывод в терминал
	rLogManager::instance().m_enable.Set(true);     // Запрещаем вещание по TCP
	rLogManager::instance().setAddCalback(LogCallback);
	rLogManager::instance().Run(16);
	gInfo_Log = rLogManager::instance().GetThread();

	//
	gTritonnManager.Run(16);
	gInfo_Tritonn = gTritonnManager.GetThread();

	if(rSimpleArgs::instance().getOption(Args::USER).size())
	{
		gDisplayManager.SetAutoLogin(rSimpleArgs::instance().getOption(Args::USER), rSimpleArgs::instance().getOption(Args::PASSWORD));
	}

	if(rSimpleArgs::instance().getOption(Args::HOST).size() && rSimpleArgs::instance().getOption(Args::PORT).size())
	{
		UINT port = atol(rSimpleArgs::instance().getOption(Args::PORT).c_str());
		gTritonnManager.Connect(rSimpleArgs::instance().getOption(Args::HOST), port);
	}

	//
	gDisplayManager.Run(32);
	gInfo_Display = gDisplayManager.GetThread();


	///////////////////////////////////////////////////////////


	while(true)
	{
		if(gExit.Get())
		{
			break;
		}
		mSleep(100);
	}

	gTritonnManager.Finish();
	pthread_join(*gInfo_Tritonn, NULL);

	gDisplayManager.Finish();
	pthread_join(*gInfo_Display, NULL);

	rLogManager::instance().Finish();

   return 0;
}


UDINT PeriodicSetAdd(rPacketSetData &pset, const std::string& name, const std::string& val)
{
	if(pset.Count >= MAX_PACKET_SET_COUNT)
	{
		LogCallback("Error: maximum of set variables reached.");
		return 1;
	}

	strncpy(pset.Name [pset.Count], name.c_str(), MAX_VARIABLE_LENGTH);
	strncpy(pset.Value[pset.Count],  val.c_str(), MAX_VARVALUE_LENGTH);

	++pset.Count;

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Получение логов от LogManager'а
void LogCallback(const std::string &text)
{
	gDisplayManager.AddLog(text);
}


//-------------------------------------------------------------------------------------------------
//
std::string GetStatusError(USINT err, UDINT shortname)
{
	switch(err)
	{
		case 3: return shortname ? "AD" : "Access denied";
		case 5: return shortname ? "OK" : "Assign";
		case 8: return shortname ? "ER" : "Error";
		case 1: return shortname ? "NF" : "Variable not found";
		case 2: return shortname ? "RO" : "Variable is readonly";
		case 0: return shortname ? "UE" : "Undefined error";
		case 7: return shortname ? "OK" : "writed";
	}

	return shortname? "??" : "<?>";
}



std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	 std::stringstream ss(s);
	 std::string item;
	 while (std::getline(ss, item, delim))
	 {
		  elems.push_back(item);
	 }
	 return elems;
}

std::vector<std::string> split(const string &s, char delim)
{
	 std::vector<std::string> elems;
	 split(s, delim, elems);
	 return elems;
}


//================================================================================================
//
//
//


