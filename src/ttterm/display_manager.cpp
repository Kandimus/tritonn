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
#include "safity.h"
#include "locker.h"
#include "log_manager.h"
#include "tickcount.h"
#include "simplefile.h"
#include "tritonn_version.h"
#include "data_proto.h"
#include "login_proto.h"
#include "packet_login.h"
#include "packet_loginanswe.h"
#include "packet_set.h"
#include "packet_setanswe.h"
#include "packet_get.h"
#include "packet_getanswe.h"
#include "display_manager.h"




//-------------------------------------------------------------------------------------------------
//
rDisplayManager::rDisplayManager()
{
	Auto          = false;
	InputStr      = "";
	InputCurPos   = 0;
	RedrawInput   = true;
	RedrawLog     = false;
	RedrawInfo    = true;
	MaxVisibleLog = 10;
	LoginSrc      = LOGIN_NONE;
	LoginOK       = 0;
	LoginName     = "";
	LoginPwd      = "";
	DumpFileName  = "./ttterm.dump";
	Hide          = false;
	AutoID        = 0;

	m_msgGetData.set_userdata(1);

	//DEBUG
	InputHistory.push_back("c 127.0.0.1");
	InputHistory.push_back("l ./test/vars.test");
	InputHistoryPos = InputHistory.size();
}


rDisplayManager::~rDisplayManager()
{
	endwin();
}


//-------------------------------------------------------------------------------------------------
//
rThreadStatus rDisplayManager::Proccesing()
{
	std::string   command = "";
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	rTickCount    tperiod;
	rTickCount    tautocmd;

	if (!Auto) {
		tperiod.start(1000);
	}

	// Автоматическое выполнение (без вывода на экран)
	if(!Hide)
	{
		initscr();
		noecho();
		keypad(stdscr, true);
		timeout(0);
		curs_set(1);

		getmaxyx(stdscr, MaxRow, MaxCol);

		if(MaxRow >= 20)
		{
			MaxVisibleLog = MaxRow - 20;
		}
	}

	while(1)
	{
		// Обработка команд нити
		thread_status = rThreadClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		Draw();

		command = GetCommand();

		if(command.size())
		{
			if(!ProccesingLogin(command))
			{
				UDINT          cmderr = true;
				vector<string> args;

				AddToHistory(command);

				split(command, ' ', args);

				cmderr = RunCmd(args);

				if(cmderr)
				{
					AddLog("Unknow command '" + command + "'");
				}
			} // if login

			command = "";
		}

		if(tperiod.isFinished())
		{
			tperiod.restart();

			if (m_msgGetData.read_size()) {
				++PacketGetCount;
				gTritonnManager.sendDataMsg(m_msgGetData);
			}
		}
		
	}

	return rThreadStatus::CLOSED;
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::AddLog(const string &text)
{
	rLocker locker(Mutex);

	LogList.push_back(text);

	while(LogList.size() > MaxVisibleLog)
	{
		LogList.pop_front();
	}

	RedrawLog = true;
}



//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::Draw()
{
	if(Hide) return;

	if(RedrawInfo)
	{
		mvwprintw(stdscr, 0, 0/*х*/, "ttterm %i.%i.%i.%x", TRITONN_VERSION_MAJOR, TRITONN_VERSION_MINOR, TRITONN_VERSION_BUILD, TRITONN_VERSION_HASH);

		if (TritonnVer.m_build || TritonnVer.m_major || TritonnVer.m_minor || TritonnVer.m_hash) {
			mvwprintw(stdscr, 0, MaxCol / 2/*х*/, "tritonn %i.%i.%i.%x <%s>", TritonnVer.m_major, TritonnVer.m_minor, TritonnVer.m_build, TritonnVer.m_hash,
						 TritonnConf.m_filename.c_str());
		}
		else
		{
			mvwprintw(stdscr, 0, MaxCol / 2/*х*/, "tritonn - not connect");
		}
		clrtoeol();

		mvwprintw(stdscr, 1, 0/*х*/, "--- Variables ------------------------------------ [%i]---", PacketGetCount); clrtoeol();
		mvwprintw(stdscr, 2 + MAX_PACKET_GET_COUNT / 2, 0/*х*/, "----------------------------------------------------------"); clrtoeol();
	}


	if(RedrawGet)
	{
		mvwprintw(stdscr, 1, 0/*х*/, "--- Variables ------------------------------------ [%i]---", PacketGetCount);
		for(UDINT ii = 0; ii < MAX_PACKET_GET_COUNT; ++ii)
		{
			UDINT x = (ii % 2 == 0) ? 0 : MaxCol / 2;

			if(!x)
			{
				wmove(stdscr, 2 + (ii / 2), 0);
				clrtoeol();
			}

			if(ii < PacketGetAnsweData.Count)
			{
				mvwprintw(stdscr, 2 + ii / 2, x, "%s = %s (%s)",
							 PacketGetAnsweData.Name[ii],
							 PacketGetAnsweData.Result[ii] == 5 ?  PacketGetAnsweData.Value[ii] : "?",
							 GetStatusError(PacketGetAnsweData.Result[ii], true).c_str());
			}

		}
	}

	// Перерисовываем логи
	if(RedrawLog)
	{
		UDINT yy = 0;
		for(list<string>::iterator ii = LogList.begin(); ii != LogList.end(); ++ii, ++yy)
		{
			mvwprintw(stdscr, 19 + yy, 0/*х*/, ii->c_str());
			//mvwprintw(stdscr, MaxRow - 1 - MaxVisibleLog + yy, 0/*х*/, ii->c_str());
			clrtoeol();
		}
	}

	if(RedrawInput)
	{
		if(LOGIN_NAME == LoginSrc)
		{
			InputBeginCur = 11;
			mvwprintw(stdscr, MaxRow - 1, 0/*х*/, "input name:");
			clrtoeol();
			mvwprintw(stdscr, MaxRow - 1, InputBeginCur, "%s", InputStr.c_str());
		}
		else if(LOGIN_PWD == LoginSrc)
		{
			InputBeginCur = 15;
			mvwprintw(stdscr, MaxRow - 1, 0/*х*/, "input password:");
			clrtoeol();
		}
		else
		{
			InputBeginCur = 1;
			mvwprintw(stdscr, MaxRow - 1, 0/*х*/, ">");
			clrtoeol();
			mvwprintw(stdscr, MaxRow - 1, InputBeginCur, "%s", InputStr.c_str());
		}
	}

	if(Auto)
	{
		mvwprintw(stdscr, MaxRow - 1, 0, "executing...%i/%i", AutoID, AutoCommands.size());
		clrtoeol();
		refresh();
	}
	else if(RedrawInput || RedrawLog | RedrawGet)
	{
		move(MaxRow - 1, InputBeginCur + InputCurPos);
		refresh();
	}

	RedrawLog   = false;
	RedrawInput = false;
	RedrawInfo  = false;
	RedrawGet   = false;

}


//-------------------------------------------------------------------------------------------------
// Команда на ввод логина и пароля, и отправку сообщения PacketLogin в тритонн
void rDisplayManager::ShowLogin()
{
	rLocker locker(Mutex);

	if(LoginName.size() && LoginPwd.size())
	{
		SendPacketLogin();

		LoginSrc = LOGIN_NONE;
	}
	else
	{
		LoginSrc    = LoginName.size() ? LOGIN_PWD : LOGIN_NAME;
		RedrawInput = true;
	}
}


//
UDINT rDisplayManager::ProccesingLogin(const string &command)
{
	if(LOGIN_NAME == LoginSrc)
	{
		rLocker locker(Mutex);

		LoginName   = command;
		LoginSrc    = LOGIN_PWD;
		RedrawInput = true;

		return 1;
	}

	if(LOGIN_PWD == LoginSrc)
	{
		rLocker locker(Mutex);

		LoginPwd    = command;
		LoginSrc    = LOGIN_NONE;
		RedrawInput = true;

		// Отослать пакет
		SendPacketLogin();

		return 1;
	}

	return 0;
}


UDINT rDisplayManager::SetAutoLogin(const string &name, const string &pwd)
{
	LoginName = name;
	LoginPwd  = pwd;

	return 0;
}



//-------------------------------------------------------------------------------------------------
//


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::AddToHistory(const string &str)
{
	for(UDINT ii = 0; ii < InputHistory.size(); ++ii)
	{
		if(InputHistory[ii] == str)
		{
			InputHistory.erase(InputHistory.begin() + ii);
			break;
		}
	}

	InputHistory.push_back(str);
	InputHistoryPos = InputHistory.size();
}


//-------------------------------------------------------------------------------------------------
// Загрузка авто-команд
UDINT rDisplayManager::LoadAutoCommand(const string &filename)
{
	vector<string> args;

	Hide   = true;

	args.push_back("load");
	args.push_back(filename);

	cmdLoad(args);

	return 0;
}






///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Обработка сетевых протоколов
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
// Отправка сообщения PacketLogin в тритонн
UDINT rDisplayManager::SendPacketLogin()
{
	rPacketLogin packet;

	packet.SetUserName(LoginName);
	packet.SetUserPwd (LoginPwd);
	gTritonnManager.Send(&packet.Data, LENGTH_PACKET_LOGIN);

	LoginName = "";
	LoginPwd  = "";

	return 0;
}


//-------------------------------------------------------------------------------------------------
//
void rDisplayManager::CallbackLogin(TT::LoginMsg* msg)
{
	rLocker locker(Mutex); locker.Nop();

	if (msg) {
		LoginOK = msg.access();
	} else {
		LoginOK = 0;
		memset(&TritonnVer, 0, sizeof(TritonnVer));
	}

	RedrawInfo  = true;
}


//-------------------------------------------------------------------------------------------------
//
UDINT rDisplayManager::CallbackData(rPacketClient* client)
{
	for(UDINT ii = 0; ii < data->Count; ++ii)
	{
		if(data->Result[ii] == 7)
		{
			TRACEI(LogMask, String_format("Variable '%s' set as '%s'.", data->Name[ii], data->Value[ii]).c_str());
		}
		else
		{
			TRACEW(LogMask, "Variable '%s' setting failed. Error: %s", data->Name[ii], GetStatusError(data->Result[ii], false).c_str());
		}
	}

	WaitingAnswe.Set(WAITTING_NONE);

	return 0;
}


//
UDINT rDisplayManager::CallbackGetAnswe(rPacketGetAnsweData *data)
{
	switch(data->UserData)
	{
		case USER_PERIODIC:
		{
			rLocker lock(MutexPacket);

			PacketGetAnsweData = *data;
			RedrawGet          = 1;

			break;
		}

		case USER_DUMP:
		{
			FILE *file = fopen(DumpFileName.c_str(), "at");

			if(nullptr == file)
			{
				TRACEP(LOG::TERMINAL, "Can't open dump file!");
				break;
			}

			for(UDINT ii = 0; ii < data->Count; ++ii)
			{
				fprintf(file, "%s = %s (%s)\n", data->Name[ii], data->Value[ii], GetStatusError(data->Result[ii], true).c_str());
			}
			fclose(file);

			break;
		}

		default:
			for(UDINT ii = 0; ii < data->Count; ++ii)
			{
				if(data->Result[ii] == 5)
				{
					TRACEI(LogMask, String_format("Variable '%s' = '%s'.", data->Name[ii], data->Value[ii]).c_str());
				}
				else
				{
					TRACEW(LogMask, "Variable '%s' getting failed. Error: %s", data->Name[ii], GetStatusError(data->Result[ii], false).c_str());
				}
			}
			break;
	}

	WaitingAnswe.Set(WAITTING_NONE);

	return 0;
}


// Получение команд, или из ввода или из файла
string rDisplayManager::GetCommand()
{
	string command ="";

	if(!Auto)
	{
		rLocker locker(Mutex); locker.Nop();

		command = GetInput();

		return command;
	}

	if(!LoginOK)
	{
		return command;
	}

	if(WAITTING_NONE == WaitingAnswe.Get())
	{
		if(AutoID >= AutoCommands.size())
		{
			if(Hide)
			{
				gExit.Set(1);
				return "";
			}
			else
			{
				Auto   = false;
				AutoID = 0;
				RedrawInput = true;
				return "";
			}
		}

//printf("%s\n", AutoCommands[AutoID].c_str());

		command = AutoCommands[AutoID++];
	}

	return command;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Разбор ввода пользователя
//
////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//
string rDisplayManager::GetInput()
{
	string result = "";
	int    ch     = getch();

	// Обычные символы
	if(ch >= 32 && ch <= 126)
	{
		if(InputCurPos >= InputStr.size())
		{
			InputStr += (char)ch;
		}
		else
		{
			InputStr.insert(InputStr.begin() + InputCurPos, (char)ch);
		}
		++InputCurPos;
		RedrawInput = true;
	}
	else if('\n' == ch)
	{
		result      = InputStr;
		InputStr    = "";
		InputCurPos = 0;
		RedrawInput = true;
	}
	else if(KEY_BACKSPACE == ch)
	{
		if(InputCurPos)
		{
			InputStr.erase(InputStr.begin() + InputCurPos - 1);
			--InputCurPos;
			RedrawInput = true;
		}
	}
	else if(KEY_DC == ch)
	{
		if(InputCurPos < InputStr.size())
		{
			InputStr.erase(InputStr.begin() + InputCurPos);
			RedrawInput = true;
		}
	}
	else if(KEY_LEFT == ch)
	{
		if(InputCurPos > 0)
		{
			--InputCurPos;
			RedrawInput = true;
		}
	}
	else if(KEY_RIGHT == ch)
	{
		if(InputCurPos < InputStr.size())
		{
			++InputCurPos;
			RedrawInput = true;
		}
	}
	else if(KEY_UP == ch)
	{
		if(InputHistoryPos > 0)
		{
			--InputHistoryPos;
			InputStr    = InputHistory[InputHistoryPos];
			InputCurPos = InputStr.size();
			RedrawInput = true;
		}
	}
	else if(KEY_DOWN == ch)
	{
		if(InputHistoryPos < InputHistory.size() - 1)
		{
			++InputHistoryPos;
			InputStr    = InputHistory[InputHistoryPos];
			InputCurPos = InputStr.size();
			RedrawInput = true;
		}
	}
	else if(KEY_HOME == ch)
	{
		InputCurPos = 0;
		RedrawInput = true;
	}
	else if(KEY_END == ch)
	{
		InputCurPos = InputStr.size();
		RedrawInput = true;
	}

	return result;
}



