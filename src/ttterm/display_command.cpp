//=================================================================================================
//===
//=== display_manager.cpp
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

#include <string.h>
#include <sstream>
#include "safity.h"
#include "log_manager.h"
#include "tickcount.h"
#include "simplefile.h"
#include "ttterm_version.h"
#include "packet_login.h"
#include "packet_loginanswe.h"
#include "packet_set.h"
#include "packet_setanswe.h"
#include "packet_get.h"
#include "packet_getanswe.h"
#include "display_manager.h"


#define ARGS_TEXT_EQUAL(x, y)         ((x) == args[0] || (y) == args[0])


extern UDINT PeriodicSetAdd(rPacketSetData &pset, const string &name, const string &val);


UDINT rDisplayManager::RunCmd(vector<string> &args)
{
	if(args.empty()) return 1;

	if(ARGS_TEXT_EQUAL("c" , "connect")   ) { cmdConnect(args);    return 0; }
	if(ARGS_TEXT_EQUAL("dc", "disconnect")) { cmdDisconnect(args); return 0; }
	if(ARGS_TEXT_EQUAL("e" , "exit")      ) { cmdExit(args);       return 0; }
	if(ARGS_TEXT_EQUAL("s" , "set")       ) { cmdSet(args);        return 0; }
	if(ARGS_TEXT_EQUAL("g" , "get")       ) { cmdGet(args);        return 0; }
	if(ARGS_TEXT_EQUAL("+g", "+get")      ) { cmdPGet(args);       return 0; }
	if(ARGS_TEXT_EQUAL("-g", "-get")      ) { cmdMGet(args);       return 0; }
	if(ARGS_TEXT_EQUAL("d" , "dump")      ) { cmdDump(args);       return 0; }
	if(ARGS_TEXT_EQUAL("l" , "load")      ) { cmdLoad(args);       return 0; }
	if(ARGS_TEXT_EQUAL("w" , "wait")      ) { cmdWait(args);       return 0; }

//			PacketRcvd.Set(!gCmdList[ii].NeedAnswe);

	return 1;
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::cmdConnect(vector<string> &args)
{
	UINT port = TCP_PORT_TERM;

	if(args.size() > 3)
	{
		AddLog("using: connect <ip> [, <port>]");
		return;
	}

	if(args.size() > 2)
	{
		port = atoi(args[2].c_str());
	}

	if(gTritonnManager.IsConnected())
	{
		AddLog("Error. Allready connected.");
		return;
	}

	gTritonnManager.Connect(args[1], port);
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::cmdDisconnect(vector<string> &args)
{
	if(args.size() > 1)
	{
		AddLog("using: disconnect");
		return;
	}

	if(gTritonnManager.IsConnected())
	{
		gTritonnManager.Disconnect();

		CallbackLoginAnswe(nullptr);
	}
	else
	{
		AddLog("Error! Is not connected.");
	}
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::cmdExit(vector<string> &args)
{
	if(args.size() > 1)
	{
		AddLog("using: exit");
		return;
	}

	gExit.Set(true);
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::cmdSet(vector<string> &args)
{
	if(args.size() % 2 != 1 || args.size() == 1)
	{
		AddLog("using: set <variable> <new value> [<variable> <new value> [..]]");
		return;
	}

	if(!gTritonnManager.IsConnected())
	{
		AddLog("Error: Not connected to tritonn");
		return;
	}

	if((args.size() - 1) > (MAX_PACKET_GET_COUNT * 2))
	{
		AddLog("Error: maximum of set variables reached.");
		return;
	}

	rPacketSet pset;

	for(UDINT ii = 1; ii < args.size(); ii += 2)
	{
		if(PeriodicSetAdd(pset.Data, args[ii], args[ii + 1]))
		{
			AddLog("Error: Can't add variable");
			return;
		}
	}

	// Если мы в режиме автотеста, то нужно ждать ответа, иначе (мы в режиме терминала) не нужна
	WaitingAnswe.Set((Auto) ? WAITTING_ANSWE : WAITTING_NONE);

	gTritonnManager.SendPacketSet(pset.Data);
}


/*
//-------------------------------------------------------------------------------------------------
// Команда периодической установки значений
void CmdPSet(vector<string> &args)
{
	if(args.size() != 3)
	{
		LogCallback("using: set <variable> <new value>");
		return;
	}

	if(!gTritonnManager.IsConnected())
	{
		LogCallback("Error: Not connected to tritonn");
		return;
	}

	rPacketSet pset;

	if(!PeriodicSetAdd(pset.Data, args[1], args[2]))
	{
		gTritonnManager.SendPacketSet(pset.Data);
	}
}
*/


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::cmdGet(vector<string> &args)
{
	if(1 == args.size())
	{
		AddLog("using: get <variable> [<variable> [<variable> [..]]]");
		return;
	}

	if(!gTritonnManager.IsConnected())
	{
		AddLog("Error: Not connected to tritonn");
		return;
	}

	if((args.size() - 1) > MAX_PACKET_GET_COUNT)
	{
		AddLog("Error: maximum of get variables reached.");
		return;
	}

	rPacketGetData data;

	data.Count    = 0;
	data.UserData = 0;

	for(UDINT ii = 1; ii < args.size(); ++ii, ++data.Count)
	{
		strncpy(data.Name[ii - 1], args[ii].c_str(), MAX_VARIABLE_LENGTH);
	}

	// Если мы в режиме автотеста, то нужно ждать ответа, иначе (мы в режиме терминала) не нужна
	WaitingAnswe.Set((Auto) ? WAITTING_ANSWE : WAITTING_NONE);

	gTritonnManager.SendPacketGet(data);
}


//
void rDisplayManager::cmdPGet(vector<string> &args)
{
	if(1 == args.size())
	{
		AddLog("using: +get <variable> [<variable> [<variable> [..]]]");
		return;
	}

	if(!gTritonnManager.IsConnected())
	{
		AddLog("Error: Not connected to tritonn");
		return;
	}

	//
	rLocker lock(MutexPacket); lock.Nop();

	if(PacketGetData.Count + (args.size() - 1) > MAX_PACKET_GET_COUNT)
	{
		AddLog("Error: maximum of get variables reached.");
		return;
	}

	for(UDINT ii = 1; ii < args.size(); ++ii, ++PacketGetData.Count)
	{
		args[ii] = String_tolower(args[ii]);

		strncpy(PacketGetData.Name[PacketGetData.Count], args[ii].c_str(), MAX_VARIABLE_LENGTH);
	}

	WaitingAnswe.Set(WAITTING_NONE);

	// Отсылаем в TritonnManager посылку
	gTritonnManager.SendPacketGet(PacketGetData);
}


//
void rDisplayManager::cmdMGet(vector<string> &args)
{
	if(1 == args.size())
	{
		AddLog("using: -get <variable> [<variable> [<variable> [..]]]");
		return;
	}

	rLocker        lock(MutexPacket); lock.Nop();
	rPacketGetData tmp_packet;
	UINT           find = 0;

	tmp_packet.UserData = PacketGetData.UserData;
	tmp_packet.Count    = 0;
	for(UDINT jj = 1; jj < args.size(); ++jj)
	{
		args[jj] = String_tolower(args[jj]);
	}

	for(UDINT ii = 0; ii < PacketGetData.Count; ++ii)
	{
		find = 0;

		for(UDINT jj = 1; jj < args.size(); ++jj)
		{
			if(args[jj] == PacketGetData.Name[ii])
			{
				find = 1;
				break;
			}
		}

		if(!find)
		{
			strncpy(tmp_packet.Name[tmp_packet.Count++], PacketGetData.Name[ii], MAX_VARIABLE_LENGTH);
		}
	}

	memcpy(&PacketGetData, &tmp_packet, sizeof(rPacketGetData));

	WaitingAnswe.Set(WAITTING_NONE);

	// Отсылаем в TritonnManager посылку
	gTritonnManager.SendPacketGet(PacketGetData);
}



void rDisplayManager::cmdDump(vector<string> &args)
{
	if(1 == args.size())
	{
		AddLog("using: dump <variable> [<variable> [<variable> [..]]]");
		return;
	}

	if(!gTritonnManager.IsConnected())
	{
		AddLog("Error: Not connected to tritonn");
		return;
	}

	if(args.size() - 1 >= MAX_PACKET_GET_COUNT)
	{
		AddLog("Error: maximum of get variables reached.");
		return;
	}

	rPacketGetData data;

	data.Count    = 0;
	data.UserData = USER_DUMP;

	for(UDINT ii = 1; ii < args.size(); ++ii, ++data.Count)
	{
		strncpy(data.Name[ii - 1], args[ii].c_str(), MAX_VARIABLE_LENGTH);
	}

	WaitingAnswe.Set(WAITTING_ANSWE);

	gTritonnManager.SendPacketGet(data);
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::cmdLoad(vector<string> &args)
{
	string text   = "";
	UDINT  result = TRITONN_RESULT_OK;

	if(args.size() != 2)
	{
		AddLog("using: load <filename>");
		return;
	}

	//  Данную команду нельзя использовать в режиме авто-тестирования
	if(Auto) return;

	result = SimpleFileLoad(args[1], text);

	// Сбрасываем номер исполяемой авто-команды
	AutoID = 0;

	if(TRITONN_RESULT_OK != result)
	{
		AddLog(String_format("Can't load '%s', Error : %i", args[1].c_str(), result));
		return;
	}

	// Парсим на строки
	AutoCommands.clear();
	split(text, '\n', AutoCommands);
	DumpFileName = args[1] + ".dump";

	// Очищаем список команд от пустых символов перед командой и после
	for(UDINT ii = 0; ii < AutoCommands.size(); ++ii)
	{
		string *str  = &AutoCommands[ii];
		UDINT   done = 0;

		while(!done)
		{
			done = 1;

			if(str->empty())
			{
				AutoCommands.erase(AutoCommands.begin() + ii);
				--ii;
				continue;
			}

			if((*str)[0] == ';')
			{
				AutoCommands.erase(AutoCommands.begin() + ii);
				--ii;
				continue;
			}

			switch((*str)[0])
			{
				case ' ':
				case '\t':
				case '\r':
					str->erase(str->begin());
					done = 0;
					break;
			}

			switch((*str)[str->size() - 1])
			{
				case ' ':
				case '\t':
				case '\r':
					str->erase(str->end() - 1);
					done = 0;
					break;
			}
		}
	}

	Auto = 1;

	if(!Hide)
	{
		AddLog(String_format("load '%s', found %i command", args[1].c_str(), AutoCommands.size()));
	}

}



void rDisplayManager::cmdWait(vector<string> &args)
{
	if(2 != args.size())
	{
		AddLog("using: wait <msec>");
		return;
	}

	if(!Auto) return;

	mSleep(atoi(args[1].c_str()));
}
