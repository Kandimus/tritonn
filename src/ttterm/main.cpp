#include <iostream>
#include <cstring>
#include <sstream>
#include <list>
#include <string.h>
#include <sys/ioctl.h>
#include "safity.h"
#include "log_manager.h"
#include "packet_client.h"
#include "packet_set.h"
#include "packet_get.h"
#include "tritonn_manager.h"
#include "display_manager.h"

using std::list;


rPacketClient       gTritonnClient;
rTritonnManager     gTritonnManager(gTritonnClient);
rDisplayManager     gDisplayManager;
rSafityValue<UDINT> gExit;
rThreadInfo         gInfo_Tritonn;
rThreadInfo         gInfo_Display;
rThreadInfo         gInfo_Log;
//rPacketSetData      gPeriodicSetData;
//rPacketGetData      gPeriodicGetData;


extern void  LogCallback(const string &text);
extern UDINT PeriodicSetAdd(rPacketSetData &pset, const string &name, const string &val);




int main(int argc, char **argv)
{
	string user = "";
	string pwd  = "";
	string ip   = "";
	UINT   port = TCP_PORT_TERM;

	// Обработка параметров запуска
	for(int ii = 1; ii < argc; ++ii)
	{
		if(!strcmp(argv[ii], "-a"))
		{
			if(ii >= argc - 1)
			{
				fprintf(stderr, "Invalid argument '-a'\n");
				exit(1);
			}

			gDisplayManager.LoadAutoCommand(argv[++ii]);
		}
		else if(!strcmp(argv[ii], "-U"))
		{
			if(ii >= argc - 1)
			{
				fprintf(stderr, "Invalid argument '-U'\n");
				exit(2);
			}

			user = argv[++ii];
		}
		else if(!strcmp(argv[ii], "-P"))
		{
			if(ii >= argc - 1)
			{
				fprintf(stderr, "Invalid argument '-P'\n");
				exit(3);
			}

			pwd = argv[++ii];
		}
		else if(!strcmp(argv[ii], "-i"))
		{
			if(ii >= argc - 1)
			{
				fprintf(stderr, "Invalid argument '-i'\n");
				exit(4);
			}

			ip = argv[++ii];
		}

		if(!strcmp(argv[ii], "-p"))
		{
			if(ii >= argc)
			{
				fprintf(stderr, "Invalid argument '-p'\n");
				exit(5);
			}

			port = atoi(argv[++ii]);
		}
	}


	rLogManager::Instance().Terminal.Set(false);  // Запрещаем вывод в терминал
	rLogManager::Instance().Enable.Set(true);     // Запрещаем вещание по TCP
	rLogManager::Instance().SetAddCalback(LogCallback);
	rLogManager::Instance().Run(0);
	gInfo_Log.Thread = rLogManager::Instance().GetThread();

	//
	gTritonnManager.Run(0);
	gInfo_Tritonn.Thread = gTritonnManager.GetThread();

	if(user.size())
	{
		gDisplayManager.SetAutoLogin(user, pwd);
	}

	if(ip.size() && port)
	{
		gTritonnManager.Connect(ip, port);
	}

	//
	gDisplayManager.Run(16);
	gInfo_Display.Thread = gDisplayManager.GetThread();


	///////////////////////////////////////////////////////////


	while(true)
	{
		if(gExit.Get())
		{
			break;
		}
	}

	gTritonnManager.Close();
	pthread_join(*gInfo_Tritonn.Thread, NULL);

	gDisplayManager.Close();
	pthread_join(*gInfo_Display.Thread, NULL);

   return 0;
}


UDINT PeriodicSetAdd(rPacketSetData &pset, const string &name, const string &val)
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
void LogCallback(const string &text)
{
	gDisplayManager.AddLog(text);
}


//-------------------------------------------------------------------------------------------------
//
string GetStatusError(USINT err, UDINT shortname)
{
	switch(err)
	{
		case SS_STATUS_ACCESSDENIED: return shortname ? "AD" : "Access denied";
		case SS_STATUS_ASSIGN      : return shortname ? "OK" : "Assign";
		case SS_STATUS_NOTASSIGN   : return shortname ? "NA" : "Not assign";
		case SS_STATUS_NOTFOUND    : return shortname ? "NF" : "Variable not found";
		case SS_STATUS_READONLY    : return shortname ? "RO" : "Variable is readonly";
		case SS_STATUS_UNDEF       : return shortname ? "UE" : "Undefined error";
		case SS_STATUS_WRITED      : return shortname ? "OK" : "writed";
	}

	return shortname? "??" : "<?>";
}



vector<string> &split(const string &s, char delim, vector<string> &elems)
{
	 std::stringstream ss(s);
	 std::string item;
	 while (std::getline(ss, item, delim))
	 {
		  elems.push_back(item);
	 }
	 return elems;
}

vector<string> split(const string &s, char delim)
{
	 vector<string> elems;
	 split(s, delim, elems);
	 return elems;
}


//================================================================================================
//
//
//


