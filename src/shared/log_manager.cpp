//=================================================================================================
//===
//=== log_manager.cpp
//===
//=== Copyright (c) 2019 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Класс-поток для выдачи log-сообщений по TCP
//===
//=================================================================================================

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>
#include "log_client.h"
#include "log_manager.h"
#include "simplefile.h"
#include "time64.h"

std::string rLogManager::m_logAppName = "logapp";

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// КОНСТРУКТОРЫ И ДЕСТРУКТОР
rLogManager::rLogManager()
	: rTCPClass("0.0.0.0", TCP_PORT_LOG, MAX_LOG_CLIENT), fnAddCalback(nullptr)
{
	RTTI     = "rLogManager";
	IncCount = 0;

	pthread_mutex_init(&MutexList, NULL);
	pthread_mutex_init(&MutexCallback, NULL);
	
	Level.Set(LM_ALL); //TODO Нужно изменить на нормальный уровень
	Terminal.Set(false);
	Enable.Set(false);     //TODO В штатном режиме можно после полной инициализации выключать

//    openlog(m_logAppName.c_str(), LOG_NDELAY/* | LOG_PERROR*/, LOG_LOCAL0);
}


rLogManager::~rLogManager()
{
	List.clear();
	
	closelog();

	pthread_mutex_destroy(&MutexList);
	pthread_mutex_destroy(&MutexCallback);
}



//-------------------------------------------------------------------------------------------------
//
//! В данной функции нельзя вызывать обычный Lock(), т.к. сообщение может добавлять класс TCPClass
//! от которого и унаследованн данный класс логов, и произойдет блокирование мьютексов
UDINT rLogManager::Add(UDINT mask, const char* filesource, UDINT lineno, const char *format, ...)
{
	UDINT terminal = false;
	UDINT level    = 0;
	
	if(!Enable.Get()) return 1;

	// Принудительно выставляем в маску логирования сообщения от менеджера сообщений
	level  = Level.Get();
	
	// Если маска сообщения не совпадает с глобальным уровнем, то пропускаем это сообщение
	if(!(level & mask)) return 0;
	
	// Если установлен флаг LM_LOG, то это логирование от менеджера логирования xD
	// в этом случае писать в порт безсмысленно, т.к. ошибка именно в порту, по этому выдаем на экран принудительно
	// Сообщения PANIC тоже печатаем всегда.
	terminal = Terminal.Get() || (mask & (LM_LOG | LM_P));
	
	// Создаем сообщение
	rPacketLog  packet(mask, lineno, filesource);

	// Формируем строку сообщения	сразу в поле packet.Text, без промежуточных буфферов
	va_list(args);
	va_start(args, format);
	vsnprintf(packet.Text, MAX_LOG_TEXT, format, args);
	va_end(args);

	auto fulltext = saveLogText(mask, packet.Date, filesource, lineno, (const char*)packet.Text);

	if(terminal)
	{
		fprintf(stderr, "%s", fulltext.c_str());
	}

	// Запускаем callback функцию, если она есть
	LockCallback();
	{
		if(fnAddCalback)
		{
			fnAddCalback(packet.Text);
		}
	}
	UnlockCallback();

	if(rTCPClass::IsStarted())
	{
		LockList();
		packet.MIC = IncCount++;
		List.push_back(packet);
		UnlockList();
	}

	return 0;
}



void rLogManager::OutErr(const char *filename, UDINT lineno, const char *format, ...)
{
	rPacketLog packet(LM_P, lineno, filename);
	
	va_list(args);
	va_start(args, format);
	vsnprintf(packet.Text, MAX_LOG_TEXT, format, args);
	va_end(args);

	auto fulltext = saveLogText(LM_P, packet.Date, filename, lineno, (const char*)packet.Text);

	fprintf(stderr, "%s", fulltext.c_str());
}


rLogManager &rLogManager::Instance()
{
	static rLogManager Singleton;

	return Singleton;
}


//-------------------------------------------------------------------------------------------------
//
rThreadStatus rLogManager::Proccesing()
{
	UDINT packetsize    = LENGTH_PACKET_LOG;
	rThreadStatus thread_status = rThreadStatus::UNDEF;
	UDINT clientCount   = 0;

	list<rPacketLog> sendlist;

	while(1)
	{
		// Обработка команд нити
		thread_status = rTCPClass::Proccesing();
		if(!THREAD_IS_WORK(thread_status))
		{
			return thread_status;
		}

		Lock();
		{
			clientCount = Client.size();
		}
		Unlock();

		// Если нет подключенных клиентов, то созданные сообщения удаляем
		if(!clientCount)
		{
			LockList();
			{
				List.clear();
			}
			UnlockList();
		}

		//TODO За скан одно сообщение без паузы, или все текущие?
		//TODO Нужно еще проверять на переполнение списка логов, установить предельное значение
		LockList();
		{
			if(List.size())
			{
				sendlist = List;
				List.clear();
			}
		}
		UnlockList();

		// Отсылаем сообщение всем клиентам
		for(list<rPacketLog>::iterator ipaket = sendlist.begin(); ipaket != sendlist.end(); ++ipaket)
		{
			Send(NULL, &(*ipaket), packetsize);
		}

		//TODO Удаление старых файлов. или архивация.
		
		rThreadClass::EndProccesing();
	}

	return rThreadStatus::UNDEF;
}


rClientTCP *rLogManager::NewClient(SOCKET socket, sockaddr_in *addr)
{
	return (rClientTCP *)new rLogClient(socket, addr);
}


UDINT rLogManager::ClientRecv(rClientTCP *client, USINT *buff, UDINT size)
{
	UNUSED(client);
	UNUSED(buff);
	UNUSED(size);
	//TODO доделать обработку принятия по каналу логов сообщения от удаленного клиента. Или же рубить его тут, бо нефиг слать нам мусор
	return 0;
}



UDINT rLogManager::StartServer()
{
	return rTCPClass::StartServer("0.0.0.0", TCP_PORT_LOG);
}


UDINT rLogManager::AddLogMask(UDINT lm)
{
	UDINT oldlvl = Level.Get();

	Level.Set(oldlvl | lm);

	return oldlvl;
}


UDINT rLogManager::RemoveLogMask(UDINT lm)
{
	UDINT oldlvl = Level.Get();

	Level.Set(oldlvl & (~lm));

	return oldlvl;
}


UDINT rLogManager::SetLogMask(UDINT lm)
{
	UDINT oldlvl = Level.Get();

	Level.Set(lm);

	return oldlvl;

}


DINT rLogManager::LockList()
{
	return pthread_mutex_lock(&MutexList);
}


DINT rLogManager::UnlockList()
{
	return pthread_mutex_unlock(&MutexList);
}


DINT rLogManager::LockCallback()
{
	return pthread_mutex_lock(&MutexCallback);
}


DINT rLogManager::UnlockCallback()
{
	return pthread_mutex_unlock(&MutexCallback);
}


UDINT rLogManager::SetAddCalback(Fn_LogAddCallback fn)
{
	LockCallback();
	fnAddCalback = fn;
	UnlockCallback();

	return 0;
}

std::string rLogManager::saveLogText(UDINT mask, const UDT& time, const std::string& source, UDINT lineno, const std::string& text)
{
	std::string filename = DIR_LOG + String_format("%u.txt", time.tv_sec / 86400);
	char logt[5] = "----";
	tm   dt;

	// Тип сообщения
	if(mask & LM_P) logt[0] = 'P';
	if(mask & LM_A) logt[1] = 'A';
	if(mask & LM_W) logt[2] = 'W';
	if(mask & LM_I) logt[3] = 'I';

	localtime_r(&time.tv_sec, &dt);

	std::string fulltext = String_format("%02i.%02i.%04i %02i:%02i:%02i.%03li [%s %s:%i] %08x %s\n",
										 dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900,
										 dt.tm_hour, dt.tm_min, dt.tm_sec, time.tv_usec / 1000,
										 logt, source.c_str(), lineno,
										 mask, text.c_str());

	SimpleFileAppend(filename, fulltext);

	return fulltext;
}



