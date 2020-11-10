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


struct rThreadInfo
{
	rThreadStatus  m_status;
	pthread_t*     m_thread;
	rThreadClass*  m_class;
};


rPacketClient       gTritonnClient;
rTritonnManager     gTritonnManager(gTritonnClient);
rDisplayManager     gDisplayManager;
rSafityValue<UDINT> gExit;
pthread_t*         gInfo_Tritonn;
pthread_t*         gInfo_Display;
pthread_t*         gInfo_Log;
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
	gInfo_Log = rLogManager::Instance().GetThread();

	//
	gTritonnManager.Run(0);
	gInfo_Tritonn = gTritonnManager.GetThread();

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
	gInfo_Display = gDisplayManager.GetThread();


	///////////////////////////////////////////////////////////


	while(true)
	{
		if(gExit.Get())
		{
			break;
		}
	}

	gTritonnManager.Finish();
	pthread_join(*gInfo_Tritonn, NULL);

	gDisplayManager.Finish();
	pthread_join(*gInfo_Display, NULL);

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


