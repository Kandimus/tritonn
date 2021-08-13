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
#include "tritonn_manager.h"
#include "display_manager.h"
#include "../tritonn/data_snapshot_item.h"

#include "stringex.h"

namespace Args
{
const char* AUTO     = "autotest";
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

int main(int argc, const char **argv)
{
	std::vector<USINT> arr_bad;
	std::vector<USINT> arr_good;

	arr_bad.resize(15);
	arr_good.resize(15);
	String_ToBuffer("180120FFFFFFFFFFFFFFFFFF010000", arr_bad.data() , arr_bad.size() );
	String_ToBuffer("180020FFFFFFFFFFFFFFFFFF010000", arr_good.data(), arr_good.size());

	TT::LoginMsg msg_bad  = deserialize_LoginMsg(arr_bad);
	TT::LoginMsg msg_good = deserialize_LoginMsg(arr_good);

	bool  has_acc_bad = msg_bad.has_access();
	bool  has_res_bad = msg_bad.has_result();
	UDINT access_bad  = msg_bad.access();
	UDINT result_bad  = msg_bad.result();

	bool  has_acc_good = msg_good.has_access();
	bool  has_res_good = msg_good.has_result();
	UDINT access_good  = msg_good.access();
	UDINT result_good  = msg_good.result();

	printf("bad  = %i: %i, %i: 0x%08X\n", has_res_bad , result_bad , has_acc_bad , access_bad);
	printf("good = %i: %i, %i: 0x%08X\n", has_res_good, result_good, has_acc_good, access_good);


	rSimpleArgs::instance()
			.addOption(Args::AUTO    , 'a', "")
			.addOption(Args::USER    , 'U', "")
			.addOption(Args::PASSWORD, 'P', "")
			.addOption(Args::HOST    , 'h', "127.0.0.1")
			.addOption(Args::PORT    , 'p', String_format("%u", LanPort::PORT_TERM));

	rSimpleArgs::instance().parse(argc, argv);

	rLogManager::instance().setDir(DIR_LOG + "ttterm/");  // Запрещаем вывод в терминал
	rLogManager::instance().m_terminal.Set(false);  // Запрещаем вывод в терминал
	rLogManager::instance().m_enable.Set(true);     // Запрещаем вещание по TCP
	rLogManager::instance().setAddCalback(LogCallback);
	rLogManager::instance().Run(16);
	gInfo_Log = rLogManager::instance().GetThread();

	TRACEI(LOG::TERMINAL, "------------------------------------------");

	//
	gTritonnManager.Run(16);
	gInfo_Tritonn = gTritonnManager.GetThread();

	if(rSimpleArgs::instance().getOption(Args::USER).size() && rSimpleArgs::instance().getOption(Args::PASSWORD).size()) {
		gDisplayManager.setAutoLogin(rSimpleArgs::instance().getOption(Args::USER), rSimpleArgs::instance().getOption(Args::PASSWORD));
	}

	if (rSimpleArgs::instance().isSet(Args::AUTO)) {
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

//-------------------------------------------------------------------------------------------------
// Получение логов от LogManager'а
void LogCallback(const std::string &text)
{
	gDisplayManager.AddLog(text);
}


//-------------------------------------------------------------------------------------------------
//
std::string GetStatusError(rSnapshotItem::Status err, UDINT shortname)
{
	switch(err)
	{
		case rSnapshotItem::Status::ACCESSDENIED: return shortname ? "AD" : "Access denied";
		case rSnapshotItem::Status::ASSIGNED    : return shortname ? "OK" : "Assign";
		case rSnapshotItem::Status::TOASSIGN    : return shortname ? "Asg" : "To assign";
		case rSnapshotItem::Status::ERROR       : return shortname ? "ER" : "Error";
		case rSnapshotItem::Status::NOTFOUND    : return shortname ? "NF" : "Variable not found";
		case rSnapshotItem::Status::READONLY    : return shortname ? "RO" : "Variable is readonly";
		case rSnapshotItem::Status::UNDEF       : return shortname ? "UE" : "Undefined error";
		case rSnapshotItem::Status::WRITED      : return shortname ? "OK" : "writed";
		case rSnapshotItem::Status::TOWRITE     : return shortname ? "Wrt" : "To write";
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


