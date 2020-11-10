//=================================================================================================
//===
//=== display_manager.h
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для отрисовки на терминале линукса.
//===
//=================================================================================================

#pragma once

#include <vector>
#include <list>
#include <ncurses.h>
#include "def.h"
#include "thread_class.h"
#include "structures.h"
#include "packet_get.h"
#include "packet_getanswe.h"
#include "tritonn_manager.h"

using std::vector;
using std::list;


struct rPacketLoginAnsweData;
struct rPacketSetAnsweData;




// Этапы ввода логина и пароля
const UDINT LOGIN_NONE = 0;
const UDINT LOGIN_NAME = 1;
const UDINT LOGIN_PWD  = 2;


const UDINT USER_NONE     = 0;
const UDINT USER_PERIODIC = 1;
const UDINT USER_DUMP     = 2;


const UDINT WAITTING_NONE  = 0;
const UDINT WAITTING_ANSWE = 1;



//-------------------------------------------------------------------------------------------------
//
class rDisplayManager : public rThreadClass
{
public:
	rDisplayManager();
	virtual ~rDisplayManager();

	void AddLog(const string &text);

	UDINT AddPeriodicGet(vector<string> &varname);
	UDINT DelPeriodicGet(vector<string> &varname);

	void  ShowLogin();
	UDINT SetAutoLogin(const string &name, const string &pwd);

	UDINT LoadAutoCommand(const string &filename);

	UDINT CallbackLoginAnswe(rPacketLoginAnsweData *data);
	UDINT CallbackSetAnswe  (rPacketSetAnsweData   *data);
	UDINT CallbackGetAnswe  (rPacketGetAnsweData   *data);

protected:
	virtual rThreadStatus Proccesing();

	string GetCommand();
	string GetInput();
	void   Draw();
	void   AddToHistory(const string &str);

	UDINT SendPacketLogin();
	UDINT ProccesingLogin(const string &command);

	UDINT RunCmd       (vector<string> &args);
	void  cmdConnect   (vector<string> &args);
	void  cmdDisconnect(vector<string> &args);
	void  cmdExit      (vector<string> &args);
	void  cmdSet       (vector<string> &args);
	void  cmdGet       (vector<string> &args);
	void  cmdPGet      (vector<string> &args);
	void  cmdMGet      (vector<string> &args);
	void  cmdDump      (vector<string> &args);
	void  cmdLoad      (vector<string> &args);
	void  cmdWait      (vector<string> &args);


public:


private:
	UDINT           MaxRow;
	UDINT           MaxCol;

	string          LoginName;
	string          LoginPwd;
	UDINT           LoginSrc;
	UDINT           LoginOK;

	string          InputStr;
	UDINT           InputCurPos;
	UDINT           InputBeginCur;
	USINT           RedrawInput;
	USINT           RedrawLog;
	USINT           RedrawInfo;
	USINT           RedrawGet;
	vector<string>  InputHistory;
	UDINT           InputHistoryPos;

	UDINT               MaxVisibleLog;
	pthread_mutex_t     MutexList;
	list<string>        LogList;

	rSafityValue<UDINT> WaitingAnswe;

	rVersion            TritonnVer;
	rConfigInfo         TritonnConf;

	UDINT               Auto;
	UDINT               Hide;
	UDINT               AutoID;
	string              DumpFileName;
	vector<string>      AutoCommands;

	pthread_mutex_t     MutexPacket;
	UDINT               PacketGetCount;
	rPacketGetAnsweData PacketGetAnsweData;
	rPacketGetData      PacketGetData;

};

extern rTritonnManager     gTritonnManager;
extern rSafityValue<UDINT> gExit;


extern string GetStatusError(USINT err, UDINT shortname);
extern vector<string> &split(const string &s, char delim, vector<string> &elems);
extern vector<string> split(const string &s, char delim);
